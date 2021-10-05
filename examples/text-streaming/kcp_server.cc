#include <stdio.h>
#include <stdlib.h>

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

    //处理收消息
    n = recvfrom(send->sockfd, buf, BUF_SIZE, MSG_DONTWAIT,
                 (struct sockaddr *) &send->addr, &len);

    if(n < 0)  // 检测是否有UDP数据包: kcp头部+data
      continue;

    if(strlen(buf+24) > 1)    // 会有1个或两个kcp头部，每个24，原因暂不明
      printf("\nUDP recv: size = %d, buf = %s\n", n, buf+24);
    else
      printf("\nUDP recv: size = %d, buf = %s\n", n, buf+24+24);

    // 预接收数据: 调用ikcp_input将裸数据交给KCP
    // kcp接收到下层协议传进来的数据底层数据buffer转换成kcp的数据包格式
    // 这些数据有可能是KCP控制报文，并不是我们要的数据。
    ret = ikcp_input(send->kcp, buf, n);
    if(ret < 0)//检测ikcp_input对 buf 是否提取到真正的数据
    {
      continue;
    }

    while(1)
    {
      //kcp将接收到的kcp数据包还原成之前kcp发送的buffer数据
      ret = ikcp_recv(send->kcp, buf, n);
      if(ret < 0)  //检测ikcp_recv提取到的数据
        break;
    }

    printf("KCP recv: ip = %s, port = %d, buf size = %d\n",
           inet_ntoa(send->addr.sin_addr), ntohs(send->addr.sin_port), n);

    if(strcmp(buf, "Conn") == 0)
    {
      printf("[Conn]  Data from Client-> %s\n", buf);

      char msg[] = "Conn-OK";
      ret = ikcp_send(send->kcp, msg, (int)sizeof(msg));
      printf("Server reply -> 内容[%s], 字节[%d] ret = %d\n\n",
             msg, (int)sizeof(msg), ret);
      continue;
    }

    if ( n > 24 )
      number++;
      printf("[%d]: %s\n", number, buf);
  }
}

int main(int argc, char *argv[])
{
  printf("This is kcp server\n");
  if(argc < 2)
  {
    printf("Argument Error: No server port\n");
    return -1;
  }

  kcpObj send;
  send.is_server = true;
  send.port = atoi(argv[1]);

  //创建kcp对象把send传给kcp的user变量
  ikcpcb *kcp = ikcp_create(0x1, (void *)&send);
  kcp->output = udp_output;           // 设置kcp对象的回调函数
  ikcp_nodelay(kcp, 1, 10, 2, 1);
  ikcp_wndsize(kcp, 128, 128);
  send.kcp = kcp;

  init(&send);    // init server socket

  loop(&send);

  return 0;
}
