#include <stdio.h>
#include <stdlib.h>

#include "opencv2/opencv.hpp"

#include "ikcp.c"
#include "utils.h"

using namespace std;

#define FRAME_HEIGHT 720  // 9 // 72 // 0
#define FRAME_WIDTH  1280 // 16 //128 //0
#define FRAME_INTERVAL (1000/30)
#define ENCODE_QUALITY 80

#define UDP_BUF_SIZE 1600

static int number = 0;

void loop(kcpObj *send)
{
  int n, ret;
  unsigned int len = sizeof(struct sockaddr_in);
  int jpegqual =  ENCODE_QUALITY;   // Compression Parameter
  char buf[UDP_BUF_SIZE]={0};

  cv::Mat frame, frame_resize;
  vector < uchar > encoded;
  cv::VideoCapture cap(0);          // Grab the camera
  cv::namedWindow("send", cv::WINDOW_AUTOSIZE);
  if (!cap.isOpened()) {
      cerr << "OpenCV Failed to open camera";
      exit(1);
  }

  while(1)
  {
    // kcp process
    isleep(1);
    ikcp_update(send->kcp, iclock());

    while (1) {
      n = recvfrom(send->sockfd, buf, UDP_BUF_SIZE, MSG_DONTWAIT,
                   (struct sockaddr *) &send->addr, &len);
      if (n < 0)
        break;

      printf("udp recv: %d\n", n);
      ret = ikcp_input(send->kcp, buf, n);
    }

    while(1)
    {
      ret = ikcp_recv(send->kcp, buf, 32);
      if(ret < 0)
        break;
      printf("ikcp_recv: result [%d]\n", ret);
    }

    // read and send image
    isleep(10);
    cap >> frame;
    if(frame.size().width==0) continue;  // check integrity; skip erroneous data
    cv::resize(frame, frame_resize, cv::Size(FRAME_WIDTH, FRAME_HEIGHT),
               0, 0, cv::INTER_LINEAR);
    vector < int > compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(jpegqual);
    cv::imencode(".jpg", frame_resize, encoded, compression_params);
    printf("jpg size: %lu\n", encoded.size());

    cv::imshow("send", frame_resize);
    cv::waitKey(1);

    number++;
    printf("\n第[%d]次发\n", number);
    ret = ikcp_send(send->kcp, (const char *) & encoded[0], encoded.size());
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

  // 创建kcp对象把send传给kcp的user变量, TODO: user变量更新
  ikcpcb *kcp = ikcp_create(0x1, (void *)&send);
  kcp->output = udp_output;               // 设置kcp对象的回调函数
  // KCP max size = (IKCP_MTU_DEF - IKCP_OVERHEAD) * IKCP_WND_RCV = 176128
  // if image size is too large, increase IKCP_WND_RCV
  ikcp_nodelay(kcp, 1, 10, 2, 1);
  ikcp_wndsize(kcp, 1024, 1024);
  send.kcp = kcp;

  init(&send);                           // init client socket
  loop(&send);

  return 0;
}
