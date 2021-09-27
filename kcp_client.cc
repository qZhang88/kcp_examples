#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "ikcp.c"
#include "utils.h"


static int number = 0;

void loop(kcpObj *send)
{
  unsigned int len = sizeof(struct sockaddr_in);
  int n, ret;

  while(1)
  {
    isleep(1);

    // ikcp_update不是调用一次两次就起作用, 要loop调用
    // ikcp_update包含ikcp_flush, 将发送队列中的数据通过下层协议进行发送
    ikcp_update(send->kcp, iclock());

    char buf[BUF_SIZE]={0};

    // 处理收消息
    n = recvfrom(send->sockfd, buf, BUF_SIZE, MSG_DONTWAIT,
                 (struct sockaddr *) &send->addr, &len);

    if(n < 0)  // 检测是否有UDP数据包: kcp头部+data
      continue;

    printf("UDP recv data: size = %d, buf = %s\n", n, buf+24);

    // 预接收数据: 调用ikcp_input将裸数据交给KCP
    // kcp接收到下层协议传进来的数据底层数据buffer转换成kcp的数据包格式
    // 这些数据有可能是KCP控制报文，并不是我们要的数据。
    ret = ikcp_input(send->kcp, buf, n);

    if(ret < 0)//检测ikcp_input对 buf 是否提取到真正的数据
    {
      // printf("ikcp_input error ret = %d\n",ret);
      continue;
    }

    while(1)
    {
      //kcp将接收到的kcp数据包还原成之前kcp发送的buffer数据
      ret = ikcp_recv(send->kcp, buf, n);
      if(ret < 0)  //检测ikcp_recv提取到的数据
        break;
    }

    printf("data recv: ip = %s, port = %d\n",
           inet_ntoa(send->addr.sin_addr), ntohs(send->addr.sin_port));

    if(strcmp(buf, "Conn-OK") == 0)
    {
      printf("Data from Server-> %s\n", buf);

      ret = ikcp_send(send->kcp, send->buff, sizeof(send->buff) );
      printf("Client reply -> 内容[%s], 字节[%d]  ret = %d\n",
             send->buff, (int)sizeof(send->buff), ret);
      number++;
      printf("第[%d]次发\n", number);
    }

    //发消息
    if(strcmp(buf, "Server: Hello!") == 0)
    {
      printf("Data from Server-> %s\n", buf);

      ikcp_send(send->kcp, send->buff, sizeof(send->buff));
      number++;
      printf("第[%d]次发\n", number);
    }
    else
      printf("!!! Data from Server-> %s\n", buf);
  }
}

int main(int argc, char *argv[])
{
  printf("This is kcp client\n");
  if(argc != 3)
  {
    printf("Argument Error: No server ip address and port\n");
    return -1;
  }

  kcpObj send;
  send.is_server = false;
  send.ipstr = (unsigned char *)argv[1];
  send.port  = atoi(argv[2]);

  bzero(send.buff, sizeof(send.buff));
  char msg[] = "Client: Hello!";
  memcpy(send.buff, msg, sizeof(msg));

  // 创建kcp对象把send传给kcp的user变量, TODO: user变量更新
  ikcpcb *kcp = ikcp_create(0x1, (void *)&send);
  kcp->output = udp_output;              // 设置kcp对象的回调函数
  ikcp_nodelay(kcp,0, 10, 0, 0);         // 1, 10, 2, 1
  ikcp_wndsize(kcp, 128, 128);
  send.kcp = kcp;

  init(&send);          // init client socket

  char init_msg[] = "Conn";                //与服务器初次通信
  int  ret = ikcp_send(send.kcp, init_msg, (int)sizeof(init_msg));
  printf("ikcp_send: [%s] len=%d, ret = %d.\n",
         init_msg, (int)sizeof(init_msg), ret);   //发送成功的

  loop(&send);

  return 0;
}
