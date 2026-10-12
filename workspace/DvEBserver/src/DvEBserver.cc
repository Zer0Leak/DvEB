//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iostream>
#include <system_error>
#include <fstream>
#include <sys/mman.h>

#include <tuple>

#include "dveb/Util.hh"
#include "dveb/ProtoVEBClient.hh"
#include "dveb/ProtoVEB.hh"
#include "dveb/DynamicProtoVEB.hh"

#include <string>

#include "dveb/ConnectionServerBsd.hh"

#include <exception>
#include <stdexcept>

const int max_length = 8192;

/*

void server(boost::asio::io_service& io_service, unsigned short port, std::shared_ptr<ProtoVEB> pVEB)
{
  tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));
  a.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  for (;;)
  {
    socket_ptr sock(new tcp::socket(io_service));
    a.accept(*sock);
    sock->set_option(boost::asio::ip::tcp::no_delay(true));
    sock->set_option(boost::asio::socket_base::reuse_address(true));
    boost::thread t(boost::bind(session, std::move(sock), pVEB));
    //session(sock, pVEB);
  }
}
*/

int main(int argc, char* argv[])
{
  try
  {
    int32_t port = 8083;
    char treeType = 'k';
    uint8_t treeK = 32;

    std::cout << "use: " << std::endl;
    std::cout << "8083 (port number) k,d (tree type - k,dynamic) 32 (tree k_size)" << std::endl;

    if (argc!= 4) {
      return 0;
    }

    if (argc >= 2) {
      port = std::stoi(argv[1]);
      if (argc >= 3) {
        treeType = argv[2][0];
        if (argc >= 4) {
          treeK = std::stoi(argv[3]);
        }
      }
    }

    std::cout << "using: port=" << port << ", treeType=" << treeType << ", treeK=" << (int)treeK << std::endl;

    std::shared_ptr<ProtoVEB> pvEB;

    if ('d' == treeType) {
      pvEB.reset(new DynamicProtoVEB(treeK));
    } else {
      pvEB = CreatePVEBFromK(treeK);
    }

    ConnectionServerBsd coon(port, pvEB);

    coon.start();

    /*
    boost::asio::io_service io_service;

    using namespace std; // For atoi.
    server(io_service, port, pvEB);
    */
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
