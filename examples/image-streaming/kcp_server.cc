#include <stdio.h>
#include <stdlib.h>

#include "opencv2/opencv.hpp"

#include "ikcp.c"
#include "utils.h"

using namespace std;

#define UDP_BUF_SIZE 1600
#define KCP_BUF_SIZE 4000000    // max 255 * 1400, 255 is max ikcp_send package count

static int number = 0;

void loop(kcpObj *send)
{
  unsigned int len = sizeof(struct sockaddr_in);
  int n, ret;
  int udp_count = 0;

  cv::namedWindow("recv", cv::WINDOW_AUTOSIZE);

  while(1)
  {
    // kcp process
    isleep(1);
    ikcp_update(send->kcp, iclock());

    char buf[KCP_BUF_SIZE]={0};
    int udp_len = 0;

    while (1) {
      n = recvfrom(send->sockfd, buf, UDP_BUF_SIZE, MSG_DONTWAIT,
                   (struct sockaddr *) &send->addr, &len);
      if (n < 0)
        break;

      udp_count++;
      udp_len += n;
      printf("udp recv: [%d] %d bytes\n", udp_count, n);
      ret = ikcp_input(send->kcp, buf, n);
      printf("ikcp_input result: [%d]\n", ret);
    }

    if (udp_len == 0)
      continue;
    else
      printf("udp loop recv: %d bytes\n", udp_len);

    while(1)
    {
      ret = ikcp_recv(send->kcp, buf, KCP_BUF_SIZE);
      if(ret < 0)  //检测ikcp_recv提取到的数据
        break;

      printf("ikcp_recv: result [%d]\n", ret);
      cv::Mat rawData = cv::Mat(1, ret, CV_8UC1, buf);
      cv::Mat frame = cv::imdecode(rawData, cv::IMREAD_COLOR);
      if (frame.size().width == 0) {
          cerr << "decode failure!" << endl;
          continue;
      }
      cv::imshow("recv", frame);
      cv::waitKey(1);
    }
    printf("\n");
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
  ikcp_wndsize(kcp, 1024, 1024);
  send.kcp = kcp;

  init(&send);                       // init server socket
  loop(&send);

  return 0;
}
