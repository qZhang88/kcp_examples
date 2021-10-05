#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "ikcp.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <windows.h>
#elif !defined(__unix)
#define __unix
#endif

#ifdef __unix
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#endif

/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
  #if defined(__unix)
  struct timeval time;
  gettimeofday(&time, NULL);
  if (sec) *sec = time.tv_sec;
  if (usec) *usec = time.tv_usec;
  #else
  static long mode = 0, addsec = 0;
  BOOL retval;
  static IINT64 freq = 1;
  IINT64 qpc;
  if (mode == 0) {
    retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    freq = (freq == 0)? 1 : freq;
    retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
    addsec = (long)time(NULL);
    addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
    mode = 1;
  }
  retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
  retval = retval * 2;
  if (sec) *sec = (long)(qpc / freq) + addsec;
  if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
  #endif
}

/* get clock in millisecond 64 */
static inline IINT64 iclock64(void)
{
  long s, u;
  IINT64 value;
  itimeofday(&s, &u);
  value = ((IINT64)s) * 1000 + (u / 1000);
  return value;
}

static inline IUINT32 iclock()
{
  return (IUINT32)(iclock64() & 0xfffffffful);
}

/* sleep in millisecond */
static inline void isleep(unsigned long millisecond)
{
  #ifdef __unix   /* usleep( time * 1000 ); */
  struct timespec ts;
  ts.tv_sec = (time_t)(millisecond / 1000);
  ts.tv_nsec = (long)((millisecond % 1000) * 1000000);
  /*nanosleep(&ts, NULL);*/
  usleep((millisecond << 10) - (millisecond << 4) - (millisecond << 3));
  #elif defined(_WIN32)
  Sleep(millisecond);
  #endif
}

typedef struct {
  bool is_server;
  unsigned char *ipstr;
  int port;
  ikcpcb *kcp;
  int sockfd;
  struct sockaddr_in addr;        // addr for sendto and recvfrom
} kcpObj;

int udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
  kcpObj *send = (kcpObj *)user;
  int n = sendto(send->sockfd, buf, len, 0,
      (struct sockaddr *) &send->addr, sizeof(struct sockaddr_in));
  if (n >= 0)
  {
    printf("udp_output: %d bytes send\n", n);  // 24字节的KCP头部
    return n;
  }
  else
  {
    printf("udp_output: %d bytes send, error\n", n);
    return -1;
  }
}

int init(kcpObj *send)
{
  send->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if(send->sockfd < 0)
  {
    perror("socket error!");
    exit(1);
  }

  bzero(&send->addr, sizeof(send->addr));

  if(send->is_server)
  {
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(send->port);
    printf("server socket: sockfd = %d  port:%d\n",
            send->sockfd, send->port);

    if(bind(send->sockfd, (struct sockaddr *)&(addr), sizeof(struct sockaddr_in)) < 0)
    {
      perror("bind error");
      exit(1);
    }
  }
  else
  {
    send->addr.sin_family      = AF_INET;
    send->addr.sin_addr.s_addr = inet_addr((char*)send->ipstr);
    send->addr.sin_port        = htons(send->port);
    printf("client socket: sockfd = %d, server ip = %s  port = %d\n",
           send->sockfd, send->ipstr, send->port);
  }
  return 0;
}

#endif
