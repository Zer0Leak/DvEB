/*
 * ConnectionServerBsd.cc
 *
 *  Created on: Aug 7, 2015
 *      Author: edgard
 */

#include "ConnectionServerBsd.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "ProtoVEB.hh"
#include "ServerHandler.hh"
#include "SocketBsd.hh"
#include "dveb/globals.hh"

#include <unistd.h>
#include <semaphore.h>

#define LISTEN_QUEUE_SIZE 2

ConnectionServerBsd::ConnectionServerBsd(uint16_t port, std::shared_ptr<ProtoVEB> pvEB) :
    port_(port), pvEB_(pvEB) {
}

ConnectionServerBsd::~ConnectionServerBsd() {
  // TODO Auto-generated destructor stub
}

static boost::condition_variable cond_;
static boost::mutex mut_;
static volatile int s_sock_ = -1;
static std::shared_ptr<ProtoVEB> s_pvEB_;
static sem_t semaphore_;

static void coon_handler() {
  int sock_opt_flag;

  while (1) {
    ::sem_wait(&semaphore_);
    int sock = s_sock_;

    ServerHandler sh;
    SocketBsd sb(sock, false);

    if (-1 == sock) {
      break;
    }

    sock_opt_flag = 1;
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *) &sock_opt_flag, sizeof(sock_opt_flag))) {
      std::cerr << "Failed to set TCP_NODELAY on communication socket: " << strerror(errno) << std::endl;
      goto error;
    }

    sock_opt_flag = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_opt_flag, sizeof(sock_opt_flag))) {
      std::cerr << "Failed to set SO_REUSEADDR on communication socket: " << strerror(errno) << std::endl;
      goto error;
    }

    if (DVEB_CACHE_SOCKET) {
      while (true == sh.handle(&sb, s_pvEB_));
    } else {
      sh.handle(&sb, s_pvEB_);
    }

    error: sb.close();
  }
}

void ConnectionServerBsd::start() {
  int serversock = -1;
  struct sockaddr_in sadr_server;
  int sock_opt_flag = 1;

  s_pvEB_ = pvEB_;

  try {

    if (0 != sem_init(&semaphore_, 0, 0)) {
      throw std::runtime_error("Can't initialize semaphore." + std::string(strerror(errno)));
    }

    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      throw std::runtime_error("Failed to create socket." + std::string(strerror(errno)));
    }
    if (setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_opt_flag, sizeof(sock_opt_flag))) {
      throw std::runtime_error("Failed to set server reuse." + std::string(strerror(errno)));
    }

    /* Construct the server sockaddr_in structure */
    memset(&sadr_server, 0, sizeof(sadr_server));
    sadr_server.sin_family = AF_INET;
    sadr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    sadr_server.sin_port = htons(port_);

    /* Bind the server socket */

    if (bind(serversock, (struct sockaddr *) &sadr_server, sizeof(sadr_server)) < 0) {
      throw std::runtime_error("Failed to bind." + std::string(strerror(errno)));
    }

    /* Listen on the server socket */
    if (listen(serversock, LISTEN_QUEUE_SIZE) < 0) {
      throw std::runtime_error("Failed to listen." + std::string(strerror(errno)));
    }

    boost::thread t(boost::bind(coon_handler));

    while (true) {
      unsigned int clientlen = sizeof(struct sockaddr_in);
      struct sockaddr_in sadr_client_sr;

      s_sock_ = ::accept(serversock, (struct sockaddr *) &sadr_client_sr, &clientlen);
      ::sem_post(&semaphore_);
      if (s_sock_ >= 0) {
      } else {
        throw std::runtime_error("Failed to accept." + std::string(strerror(errno)));
      }
    }

  } catch (std::exception & e) {
    // std::cout << e.what() << std::endl;

    if (serversock >= 0) {
      shutdown(serversock, SHUT_RDWR);
      close(serversock);
      serversock = -1;
    }

    throw e;
  }
}
