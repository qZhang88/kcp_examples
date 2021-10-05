#include <stdio.h>
#include <stdlib.h>

#include "opencv2/opencv.hpp"

#include "ikcp.c"
#include "utils.h"

using namespace std;

#define FRAME_HEIGHT 9 // 72 // 0
#define FRAME_WIDTH  16 //128 //0
#define FRAME_INTERVAL (1000/30)
#define ENCODE_QUALITY 80
#define BUF_SIZE 1400

static int number = 0;

void loop(kcpObj *send)
{
  int n, ret;
  unsigned int len = sizeof(struct sockaddr_in);
  int jpegqual =  ENCODE_QUALITY; // Compression Parameter
  char buf[BUF_SIZE]={0};
  IUINT32 current = iclock();

  cv::Mat frame, frame_resize;
  vector < uchar > encoded;
  cv::VideoCapture cap(0); // Grab the camera
  cv::namedWindow("send", cv::WINDOW_AUTOSIZE);
  if (!cap.isOpened()) {
      cerr << "OpenCV Failed to open camera";
      exit(1);
  }

  while(1)
  {
    // cv read and encode image
    cap >> frame;
    if(frame.size().width==0) continue;   //simple integrity check; skip erroneous data...
    cv::resize(frame, frame_resize, cv::Size(FRAME_WIDTH, FRAME_HEIGHT),
               0, 0, cv::INTER_LINEAR);
    vector < int > compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(jpegqual);
    cv::imencode(".jpg", frame_resize, encoded, compression_params);
    cv::imshow("send", frame_resize);
    cv::waitKey(1);

    // KCP max size = (IKCP_MTU_DEF - IKCP_OVERHEAD) * IKCP_WND_RCV = 176128
    printf("jpg size: %lu\n", encoded.size());
    ikcp_send(send->kcp, (const char *) & encoded[0], encoded.size());

    current = iclock();
    printf("CURRENT: %d *****  \n", current);
    while(iclock() - current < FRAME_INTERVAL)
    {
      isleep(2);

      bzero(buf, sizeof(buf));
      while (1) {
        n = recvfrom(send->sockfd, buf, BUF_SIZE, MSG_DONTWAIT,
                     (struct sockaddr *) &send->addr, &len);
        if (n < 0)
          break;
        else
          printf("udp recv: %d\n", n);
        ikcp_input(send->kcp, buf, n);
      }
      while(1)
      {
        ret = ikcp_recv(send->kcp, buf, n);
        if(ret < 0)  // 检测ikcp_recv提取到的数据
          break;
      }

      // printf("BEFORE UPDATE ****************************\n");
      ikcp_update(send->kcp, iclock());
    }
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
  kcp->output = udp_output;              // 设置kcp对象的回调函数
  ikcp_nodelay(kcp, 1, 10, 2, 1);         // 1, 10, 2, 1
  ikcp_wndsize(kcp, 128, 128);
  send.kcp = kcp;

  init(&send);          // init client socket
  loop(&send);

  return 0;
}
