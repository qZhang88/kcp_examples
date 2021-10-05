#include <stdio.h>
#include <stdlib.h>

#include "opencv2/opencv.hpp"

#include "ikcp.c"
#include "utils.h"

using namespace std;

#define BUF_SIZE 4000000    // max 255 * 1400, 255 is max ikcp_send package count

static int number = 0;

void loop(kcpObj *send)
{
  unsigned int len = sizeof(struct sockaddr_in);
  int n, ret;
  char buf[BUF_SIZE]={0};

  cv::namedWindow("recv", cv::WINDOW_AUTOSIZE);

  while(1)
  {
    isleep(1);
    ikcp_update(send->kcp, iclock());

    bzero(buf, sizeof(buf));

    while (1) {
      n = recvfrom(send->sockfd, buf, BUF_SIZE, MSG_DONTWAIT,
                 (struct sockaddr *) &send->addr, &len);
      if (n < 0) break;
      // 如果 p2收到udp，则作为下层协议输入到kcp2
      number++;
      printf("第[%d]个udp\n", number);
      ikcp_input(send->kcp, buf, n);
    }

    ret = ikcp_input(send->kcp, buf, n);
    if(ret < 0)    // 检测ikcp_input对 buf 是否提取到真正的数据
    {
      continue;
    }

    while(1)
    {
      ret = ikcp_recv(send->kcp, buf, n);
      if(ret < 0)  // 检测ikcp_recv提取到的数据
        break;
    }

    // printf("KCP recv: ip = %s, port = %d, buf size = %d\n",
    //        inet_ntoa(send->addr.sin_addr), ntohs(send->addr.sin_port), n);

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
