//============================================================================
// Name        : TestIntro.cpp
// Author      : Edgard Lima
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <cstdio>
#include <cstdint>

#include <random>
#include <iostream>
#include <cmath>
#include <chrono>

#include "Util.hh"

using namespace std;

template <typename T>
static T log2_floor(T a) {
  T res = 0;
  while (a >>= 1) { ++res; }
  return res;
}

template <typename T>
static T sqrt_ceil(T a) {
  T x = log2_floor(a);
  return (a >> (x >> 1)) + (x & 0x1);  // x/2 = x >> 1
}

template <typename T>
static T sqrt_floor(T a) {
  T x = log2_floor(a);
  return a >> (x >> 1);  // x/2 = x >> 1
}

int test_intro() {
  uint64_t k = 6;       // 4
  const uint64_t test_count = 65535;

  uint64_t u;           // 65536
  uint64_t sqrt_u;      // 256
  uint64_t log_u;       // 16
  uint64_t low_mask;    // 255 = 0x0F = 00001111

  if (k <= 5) {
    u = POW2(POW2(k));
    sqrt_u = sqrt_floor(u);
    log_u = log2_floor(u);
    low_mask = sqrt_u - 1;
  } else if (k == 6) {
    //                                               (pentabyte 10**15)-             (exabyte 10**18) = 8 765 432 109 876 543 210
    u = UINT64_MAX; // Can't be represented with 64 bits = UINT64_MAX + 1 = 18446744073709551615 + 1 = 18.446.744.073.709.551.616
    sqrt_u = 1 + (uint64_t)UINT32_MAX; // 1 +
    log_u = 64;
    low_mask = sqrt_u - 1;
  } else {
    std::cout << "Are you kidding me?!" << std::endl;
    exit (-1);
  }

  std::cout << "u= " << u << ", sqrt_u= " << sqrt_u << ", log_u= " << log_u  << ", low_mask= " << low_mask << std::endl;

  std::random_device rd;
  std::default_random_engine e1(rd());
  std::uniform_int_distribution<uint64_t> uniform_dist(0, u-1);  // u-1 is for k<=5, for k=6 could be 'u'

  auto start_ts = std::chrono::steady_clock::now();

  for (uint64_t i=0; i < test_count; i++) {
    uint64_t x = uniform_dist(e1);

    uint32_t high = x / sqrt_u;
    uint32_t low = x % sqrt_u;

    uint32_t bhigh = x >> (log_u >> 1);  // log_u / 2 =  log_u >> 1
    uint32_t blow = x & low_mask;

    if (high != bhigh || low != blow) {
      std::cerr << "differ! I'm a bad programmer!" << std::endl;
      printf("x=%llu(0x%02llx), high=0x%x, low=0x%x\n", (long long unsigned)x, (long long unsigned)x, high, low);
      break;
    }
  }

  auto diff = std::chrono::steady_clock::now() - start_ts;
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

  std::cout << "finished, elapsed= " << elapsed_ms.count() << " ms" << std::endl;

	return 0;
}
