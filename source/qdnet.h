#ifndef QDNET_H
#define QDNET_H

#include <stdio.h>

#include "types.h"
#include "timer.h"
#include "trace.h"
#include "utils.h"
#include "packet.h"
#include "arp.h"
#include "tcp.h"
#include "tcpsockm.h"
#include "udp.h"
#include "dns.h"

#define CONNECT_TIMEOUT  (10000ul)

#ifdef __cplusplus
extern "C" {
#endif

// Server and file information
TcpSocket *sock;

volatile int8_t breakFlag = 0;
void __interrupt __far breakHandler() {
  breakFlag = 1;
}

int8_t userInterrupted() {
  if (breakFlag) {
    fprintf(stderr, "Ctrl-Break detected - aborting!");
    return 1;
  }

  if (biosIsKeyReady()) {
    char c = biosKeyRead();
    if ((c == 27) || (c == 3)) {
      fprintf(stderr, "Esc or Ctrl-C detected - aborting!");
      return 1;
    }
  }

  return 0;
}

static void shutdown(int rc) {
  Utils::endStack();
  exit(rc);
}

int8_t resolveHost(char *hostName, IpAddr_t &hostAddr) {
  int8_t rc = Dns::resolve(hostName, hostAddr, 1);
  if (rc < 0) 
    return -1;

  uint8_t done = 0;

  while (!done) {
    if (userInterrupted()) break;
    if (!Dns::isQueryPending()) break;

    PACKET_PROCESS_MULT(5);
    Arp::driveArp();
    Tcp::drivePackets();
    Dns::drivePendingQuery();
  }

  rc = Dns::resolve(hostName, hostAddr, 0);
  if (rc != 0) {
    return -1;
  }

  return 0;
}

int8_t connectSocket(IpAddr_t &hostAddr, char* serverPort, uint32_t tcpBufSize) {
  uint16_t localport = 2048 + rand();

  sock = TcpSocketMgr::getSocket();
  if (sock->setRecvBuffer(tcpBufSize)) {
    perror("Could not create socket");
    return -1;
  }

  uint16_t convertedPort = (uint16_t)atoi(serverPort);
  if(convertedPort == 0)
    convertedPort = 80;

  if (sock->connectNonBlocking(localport, hostAddr, convertedPort))
    return -1;

  int8_t rc = -1;

  clockTicks_t start;
  clockTicks_t lastCheck;
  start = lastCheck = TIMER_GET_CURRENT();

  while (1) {
    if (userInterrupted()) break;

    PACKET_PROCESS_MULT(5);
    Tcp::drivePackets( );
    Arp::driveArp( );

    if (sock->isConnectComplete()) {
      rc = 0;
      break;
    }

    if (sock->isClosed() || (Timer_diff(start, TIMER_GET_CURRENT()) > TIMER_MS_TO_TICKS(CONNECT_TIMEOUT))) {
      break;
    }

    // Sleep so that we are not spewing TRACE records.
    while (lastCheck == TIMER_GET_CURRENT()) {};
    lastCheck = TIMER_GET_CURRENT();
  }

  return rc;
}

void releaseSocket() {
  if(sock != NULL){
    sock->close();
    TcpSocketMgr::freeSocket(sock);
    sock = NULL;
  }
}

int16_t sendMessage(uint8_t* sendBuffer, int16_t msgLength) {
  int16_t bytesSent = 0;

  while (bytesSent < msgLength) {
    if (userInterrupted()) break;

    PACKET_PROCESS_MULT(5);
    Arp::driveArp();
    Tcp::drivePackets();

    int16_t rc = sock->send((uint8_t *)(sendBuffer+bytesSent), msgLength-bytesSent);
    if (rc > 0) {
      bytesSent += rc;
    } else if (rc < 0) {
      return -1;
    }
  }

  return 0;
}

int16_t recvMessage(uint8_t* recvBuffer, int16_t bufLength, int16_t timeout) {
  clockTicks_t start = TIMER_GET_CURRENT();
  int16_t bytesRead = 0;

  while (1) {
    if (userInterrupted()) break;

    PACKET_PROCESS_MULT(5);
    Tcp::drivePackets();
    Arp::driveArp();

    int16_t rc = sock->recv((uint8_t *)recvBuffer, bufLength);
    if (rc >= 0) 
      bytesRead += rc;
    else
      return bytesRead;

    if (sock->isRemoteClosed()) {
      break;
    } else if (Timer_diff(start, TIMER_GET_CURRENT()) > TIMER_MS_TO_TICKS(timeout)) {
      fprintf(stderr, "Response timeout\n");
      break;
    }
  }

  return bytesRead;
}

#ifdef __cplusplus
}
#endif

#endif
