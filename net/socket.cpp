// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "socket.h"
#include "socketsOps.h"
#include <cstring>

BEGIN_NS(net)

//bool Socket::getTcpInfo(struct tcp_info* tcpi) const
//{
//  socklen_t len = sizeof(*tcpi);
//  memZero(tcpi, len);
//  return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
//}

//bool Socket::getTcpInfoString(char* buf, int len) const
//{
//  struct tcp_info tcpi;
//  bool ok = getTcpInfo(&tcpi);
//  if (ok)
//  {
//    snprintf(buf, len, "unrecovered=%u "
//             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
//             "lost=%u retrans=%u rtt=%u rttvar=%u "
//             "sshthresh=%u cwnd=%u total_retrans=%u",
//             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
//             tcpi.tcpi_rto,          // Retransmit timeout in usec
//             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
//             tcpi.tcpi_snd_mss,
//             tcpi.tcpi_rcv_mss,
//             tcpi.tcpi_lost,         // Lost packets
//             tcpi.tcpi_retrans,      // Retransmitted packets out
//             tcpi.tcpi_rtt,          // Smoothed round trip time in usec
//             tcpi.tcpi_rttvar,       // Medium deviation
//             tcpi.tcpi_snd_ssthresh,
//             tcpi.tcpi_snd_cwnd,
//             tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
//  }
//  return ok;
//}

Socket::~Socket()
{
    SocketsOps::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr)
{
    SocketsOps::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen()
{
    SocketsOps::listenOrDie(sockfd_);
}

SOCKET Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    SOCKET connfd = SocketsOps::accept(sockfd_, &addr);
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    SocketsOps::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
#ifdef WIN32
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));
#else
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
#endif
  // FIXME CHECK
}

void Socket::setReuseAddr(bool on)
{
    SocketsOps::setReuseAddr(sockfd_, on);
}

void Socket::setReusePort(bool on)
{
    SocketsOps::setReusePort(sockfd_, on);
}

void Socket::setKeepAlive(bool on)
{
#ifdef WIN32
    //TODO: ²¹È«WindowsµÄÐ´·¨
#else
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
#endif
}

END_NS(net)