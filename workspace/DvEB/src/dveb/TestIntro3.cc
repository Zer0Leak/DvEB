//============================================================================
// Name        : TestIntro.cpp
// Author      : Edgard Lima
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <sys/sysinfo.h>

#include <iomanip>
#include <locale>

#include <random>
#include <iostream>
#include <cmath>
#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ProtoVEBk.hh"
#include "ProtoDataRaw.hh"
#include "Util.hh"

#include <queue>
#include <unordered_map>

#include <malloc.h>

using namespace std;

template<typename T>
static T log2_floor(T a) {
	T res = 0;
	while (a >>= 1) {
		++res;
	}
	return res;
}

template<typename T>
static T sqrt_ceil(T a) {
	T x = log2_floor(a);
	return (a >> (x >> 1)) + (x & 0x1);  // x/2 = x >> 1
}

template<typename T>
static T sqrt_floor(T a) {
	T x = log2_floor(a);
	return a >> (x >> 1);  // x/2 = x >> 1
}

volatile bool test_cancel_ = false;
volatile bool test_status_ = false;

uint64_t low_mask_from_log(uint64_t l) {
	uint64_t res = 0;
	l >>= 1;
	while (l--)
		res = (res << 1) | 1;
	return res;
}

uint64_t leme_count = 0;
uint64_t leme_count2 = 0;

int print_tree(ProtoVEB & vEB, uint64_t used_mem_start) {
	if (NULL != vEB.getMin().second) {
		std::cout << "min " << vEB.getMin().first << std::endl;
	}
	if (NULL != vEB.getMax().second) {
		std::cout << "max " << vEB.getMax().first << std::endl;
	}

	if (NULL != vEB.get(0)) {
		std::cout << "0" << std::endl;
	}

	uint64_t x = 0;
	while (true) {
		std::shared_ptr<ProtoData> pd;
		std::tie(x, pd) = vEB.getSuccessor(x);
		if (NULL != pd) {
			std::cout << ", " << x;
		} else {
			break;
		}
	}
	std::cout << std::endl;

	return 0;
}

int test_tree(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters, uint64_t loop_count) {

	std::unordered_map<uint64_t, uint8_t> my_values;

	//std::random_device rd;
	//std::default_random_engine e1(rd());
	std::default_random_engine e1(0);
	uint64_t k_cluster = vEB.K() / 2;
	uint64_t k_max = (__UINT64_C(1) << k_cluster) * clusters - 1;
	std::uniform_int_distribution<uint64_t> uniform_dist(0, k_max); // u-1 is for k<=5, for k=6 could be 'u'

	auto start_ts = std::chrono::steady_clock::now();

	uint64_t first_x = 0;
	uint64_t last_x = 0;

	uint64_t min = UINT64_MAX;
	uint64_t max = 0;
	for (uint64_t i = 0; i < (__UINT64_C(1) << loop_count); i++) {
		uint64_t x = uniform_dist(e1);

		if (0 == first_x) {
			first_x = x;
		}

		if (x < min) {
			min = x;
		}
		if (x > max) {
			max = x;
		}

		last_x = x;

		my_values.insert(std::make_pair(x, 1));

		std::shared_ptr<ProtoDataRaw> pdr(new ProtoDataRaw());
		//std::string pdr_str = std::string("Iam_") + std::to_string(x);
		//pdr->Move(pdr_str);
		vEB.insert(x, pdr);

		if (!vEB.isMember(first_x)) {
			std::cout << x << "first_x is no longer member" << std::endl;
		}
	}

	std::cout << "memory all inserted:" << FormatWithCommas(getUsedMemory() - used_mem_start) << std::endl;

	std::cout << "last x is " << last_x << std::endl;

	if (vEB.getMin().first != min) {
		std::cout << "wrong min" << std::endl;
	}
	if (vEB.getMax().first != max) {
		std::cout << "wrong max" << std::endl;
	} else {
		std::cout << "max is " << vEB.getMax().first << std::endl;
	}

	e1.seed(0);

	for (uint64_t i = 0; i < (__UINT64_C(1) << loop_count); i++) {
		uint64_t x = uniform_dist(e1);

		std::unordered_map<uint64_t, uint8_t>::iterator it = my_values.find(x);
		if (my_values.end() != it) {
			if (!vEB.isMember(x)) {
				std::cout << x << " is not member" << std::endl;
			}
			vEB.remove(x);
			if (vEB.isMember(x)) {
				std::cout << x << " is member" << std::endl;
			}
			my_values.erase(it);
		}
	}

	if (vEB.getMin().second != NULL) {
		std::cout << "non null min" << std::endl;
	}
	if (vEB.getMax().second != NULL) {
		std::cout << "non null max, " << vEB.getMax().first << std::endl;
	}

	auto diff = std::chrono::steady_clock::now() - start_ts;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

	std::cout << "finished, elapsed= " << elapsed_ms.count() << " ms" << std::endl;

	//std::cout << "letme count = " << leme_count << std::endl;
	//std::cout << "letme count2 = " << leme_count2 << std::endl;

	return 0;
}

static int test_tree_performance_getmax(ProtoVEB & vEB, const std::vector<uint64_t> & test_set,
		const uint64_t group_total) {
	std::cout << "get_max: ";
	auto start_ts = std::chrono::steady_clock::now();
	for (uint64_t i = 0; i < group_total; i++) {
		// uint64_t x = test_set[i];
		vEB.getMax();
	}
	auto diff = std::chrono::steady_clock::now() - start_ts;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
	std::cout << std::to_string(elapsed_ms.count()) << ", ";
	std::cout << std::endl;

	return elapsed_ms.count();
}

static int test_tree_performance_getsuccessor(ProtoVEB & vEB, const std::vector<uint64_t> & test_set,
		const uint64_t group_total) {
	std::cout << "get_successor: ";
	auto start_ts = std::chrono::steady_clock::now();
	for (uint64_t i = 0; i < group_total; i++) {
		uint64_t x = test_set[i];
		vEB.getSuccessor(x);
	}
	auto diff = std::chrono::steady_clock::now() - start_ts;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
	std::cout << std::to_string(elapsed_ms.count()) << ", ";
	std::cout << std::endl;

	return elapsed_ms.count();
}

static int test_tree_performance_ismember(ProtoVEB & vEB, const std::vector<uint64_t> & test_set,
		const uint64_t group_total) {
	std::cout << "is_member: ";
	auto start_ts = std::chrono::steady_clock::now();
	for (uint64_t i = 0; i < group_total; i++) {
		uint64_t x = test_set[i];
		vEB.isMember(x);
	}
	auto diff = std::chrono::steady_clock::now() - start_ts;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
	std::cout << std::to_string(elapsed_ms.count()) << ", ";
	std::cout << std::endl;

	return elapsed_ms.count();
}

static int test_tree_performance_insert(ProtoVEB & vEB, const std::vector<uint64_t> & test_set,
		const uint64_t group_total) {
	std::cout << "insert: ";
	auto start_ts = std::chrono::steady_clock::now();
	for (uint64_t i = 0; i < group_total; i++) {
		uint64_t x = test_set[i];
		std::shared_ptr<ProtoDataRaw> pdr(new ProtoDataRaw());
		//std::string pdr_str = std::string("Iam_") + std::to_string(x);
		//pdr->Move(pdr_str);
		vEB.insert(x, pdr);
	}
	auto diff = std::chrono::steady_clock::now() - start_ts;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
	std::cout << std::to_string(elapsed_ms.count()) << ", ";
	std::cout << std::endl;

	return elapsed_ms.count();
}

int test_tree_performance_N_by_N(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters, uint64_t loop_count) {
	//std::random_device rd;
	//std::default_random_engine e1(rd());
	std::default_random_engine e1(0);
	uint64_t k_cluster = vEB.K() / 2;
	uint64_t k_max = (__UINT64_C(1) << k_cluster) * clusters - 1;
	std::uniform_int_distribution<uint64_t> uniform_dist(0, k_max); // u-1 is for k<=5, for k=6 could be 'u'
	const uint64_t group_total = 1000;
	uint64_t group_count = 0;
	std::vector<uint64_t> test_set;
	test_set.reserve(group_total);

	ofstream file_insert;
	file_insert.open("/home/edgard/dveb_insert.txt", std::ios::out | std::ios::trunc);
	ofstream file_ismember;
	file_ismember.open("/home/edgard/dveb_ismember.txt", std::ios::out | std::ios::trunc);

	//////////////////////////////////////////////////////////////////////////////////////////////////

	e1.seed(0);

	for (uint64_t i = 0; i < (__UINT64_C(1) << loop_count); i++) {
		uint64_t x = uniform_dist(e1);
		test_set[group_count] = x;

		if (++group_count == group_total) {
			int elap;
			elap = test_tree_performance_insert(vEB, test_set, group_total);
			file_insert << std::to_string(elap) << ", ";
			elap = test_tree_performance_ismember(vEB, test_set, group_total);
			file_ismember << std::to_string(elap) << ", ";
			elap = test_tree_performance_getsuccessor(vEB, test_set, group_total);
			elap = test_tree_performance_getmax(vEB, test_set, group_total);
			group_count = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////////////////////////////////

	file_ismember.close();
	file_insert.close();

	return 0;
}

int test_tree_performance_n_to_N(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters, uint64_t loop_first,
		uint64_t loop_last, int64_t loop, bool insert_only, uint16_t verbose, const std::string & generator) {
	//std::random_device rd;
	//std::default_random_engine e1(rd());
	std::default_random_engine e1(0);
	uint64_t treeK = vEB.K();
	uint64_t cluster_num_bits = treeK / 2;
	uint64_t max_allowed_value = (__UINT64_C(1) << cluster_num_bits) * clusters - 1;
	std::uniform_int_distribution<uint64_t> uniform_dist(0, max_allowed_value); // u-1 is for k<=5, for k=6 could be 'u'
	uint64_t loop_count = 0;

	//::malloc_trim(0);

	if (loop_first < 1 || loop_first > loop_last) {
		throw std::runtime_error("Invalid loop_count_start");
	}

	time_t time_tt;

	std::cout << "Generating numbers in the closed interval [0, " << FormatWithCommas(max_allowed_value) << "]"
			<< std::endl;

	while (loop < 0 || loop-- > 0) {

		if (test_cancel_) {
			break;
		}

		if (verbose) {
			std::cout << "loop_count: " << FormatWithCommas(loop_count++) << std::endl;
		} else {
			loop_count++;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////

		for (uint64_t i = loop_first; i <= loop_last; i++) {

			if (test_cancel_) {
				break;
			}

			// sanity_check
			if (vEB.getMin().second != NULL) {
				throw std::runtime_error("Tree is not empty before start test");
			}

			ofstream file_insert;
			file_insert.open(std::string("/home/edgard/dveb_insert_") + std::to_string(i) + std::string(".txt"),
					std::ios::out | std::ios::app);
			e1.seed(0);

			auto start_ts = std::chrono::steady_clock::now();

			uint64_t really_inserted = 0;

			for (uint64_t j = 0; j < (__UINT64_C(1) << i); j++) {
				uint64_t x;
				if (generator[0] == 'c') {
					uint64_t cluster_to_insert = j % clusters;
					x = (j / clusters) % (ProtoVEB::k_properties[cluster_num_bits].max + 1);
					x = (cluster_to_insert << cluster_num_bits) | x;
					// x = j % (max_allowed_value + __UINT64_C(1)); to insert globally sequential
				} else if (generator[0] == 'u') {
					x = uniform_dist(e1);
				} else {
					std::runtime_error e(std::string("unknown generator: " + generator));
					throw e;
				}

				if (test_cancel_) {
					break;
				}

				std::shared_ptr<ProtoDataRaw> pdr(new ProtoDataRaw());
				std::shared_ptr<ProtoData> replaced = vEB.insert(x, pdr);

				if (replaced == NULL) {
					really_inserted++;
				}

				if (test_status_) {
					std::cout << "loop_count: " << FormatWithCommas(loop_count) << std::endl;
					std::cout << "Inserted i=" << i << ", j=" << FormatWithCommas(j + __UINT64_C(1)) << "/"
							<< FormatWithCommas(__UINT64_C(1) << i) << std::endl;
					test_status_ = false;
				}
			}

			if (test_cancel_) {
				break;
			}

			auto diff = std::chrono::steady_clock::now() - start_ts;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
			file_insert << std::to_string(elapsed_ms.count()) << ", ";
			file_insert.close();

			if (verbose) {
				std::cout << "Inserted " << "i=" << i << ", finished in " << FormatWithCommas(elapsed_ms.count())
						<< " ms" << std::endl;
				std::cout << "really_inserted: " << FormatWithCommas(really_inserted) << "/"
						<< FormatWithCommas(__UINT64_C(1) << i) << ", max_allowed_values: "
						<< FormatWithCommas(max_allowed_value) << std::endl;
				//::malloc_trim(0);
				std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
				struct sysinfo info;
				sysinfo(&info);
				std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
						<< FormatWithCommas(info.totalram) << std::endl;

				std::cout << vEB.status() << std::endl;

				time_tt = ::time(NULL);
				std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
			}

			if (false == insert_only) {
				///////////////////////////////////////////////////////////////////////////////

				ofstream file_ismember;
				file_ismember.open(std::string("/home/edgard/dveb_ismember_") + std::to_string(i) + std::string(".txt"),
						std::ios::out | std::ios::app);
				e1.seed(0);

				start_ts = std::chrono::steady_clock::now();

				for (uint64_t j = 0; j < (__UINT64_C(1) << i); j++) {
					uint64_t x;
					if (generator[0] == 'c') {
						uint64_t cluster_to_ismember = j % clusters;
						x = (j / clusters) % (ProtoVEB::k_properties[cluster_num_bits].max + 1);
						x = (cluster_to_ismember << cluster_num_bits) | x;
						// x = j % (max_allowed_value + __UINT64_C(1)); to insert globally sequential
					} else if (generator[0] == 'u') {
						x = uniform_dist(e1);
					} else {
						std::runtime_error e(std::string("unknown generator: " + generator));
						throw e;
					}

					if (test_cancel_) {
						break;
					}

					if (false == vEB.isMember(x)) {
						throw std::runtime_error(std::to_string(x) + " is not member.");
					}

					if (test_status_) {
						std::cout << "loop_count: " << FormatWithCommas(loop_count) << std::endl;
						std::cout << "IsMember i=" << i << ", j=" << FormatWithCommas(j + __UINT64_C(1)) << "/"
								<< FormatWithCommas(__UINT64_C(1) << i) << std::endl;
						test_status_ = false;
					}
				}

				if (test_cancel_) {
					break;
				}

				diff = std::chrono::steady_clock::now() - start_ts;
				elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
				file_ismember << std::to_string(elapsed_ms.count()) << ", ";
				file_ismember.close();

				if (verbose) {
					std::cout << "IsMember " << "i=" << i << ", finished in " << FormatWithCommas(elapsed_ms.count())
							<< " ms" << std::endl;
					//::malloc_trim(0);
					std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
					struct sysinfo info;
					sysinfo(&info);
					std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
							<< FormatWithCommas(info.totalram) << std::endl;

					time_tt = ::time(NULL);
					std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
				}

				///////////////////////////////////////////////////////////////////////////////

				ofstream file_remove;
				file_remove.open(std::string("/home/edgard/dveb_remove_") + std::to_string(i) + std::string(".txt"),
						std::ios::out | std::ios::app);
				e1.seed(0);

				start_ts = std::chrono::steady_clock::now();

				for (uint64_t j = 0; j < (__UINT64_C(1) << i); j++) {
					uint64_t x;
					if (generator[0] == 'c') {
						uint64_t cluster_to_remove = j % clusters;
						x = (j / clusters) % (ProtoVEB::k_properties[cluster_num_bits].max + 1);
						x = (cluster_to_remove << cluster_num_bits) | x;
						// x = j % (max_allowed_value + __UINT64_C(1)); to insert globally sequential
					} else if (generator[0] == 'u') {
						x = uniform_dist(e1);
					} else {
						std::runtime_error e(std::string("unknown generator: " + generator));
						throw e;
					}

					if (test_cancel_) {
						break;
					}

					vEB.remove(x);

					if (test_status_) {
						std::cout << "loop_count: " << FormatWithCommas(loop_count) << std::endl;
						std::cout << "Removed i=" << i << ", j=" << FormatWithCommas(j + __UINT64_C(1)) << "/"
								<< FormatWithCommas(__UINT64_C(1) << i) << std::endl;
						test_status_ = false;
					}
				}

				if (test_cancel_) {
					break;
				}

				diff = std::chrono::steady_clock::now() - start_ts;
				elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
				file_remove << std::to_string(elapsed_ms.count()) << ", ";
				file_remove.close();

				if (verbose) {
					std::cout << "Removed " << "i=" << i << ", finished in " << FormatWithCommas(elapsed_ms.count())
							<< " ms" << std::endl;
					//::malloc_trim(0);
					std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
					struct sysinfo info;
					sysinfo(&info);
					std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
							<< FormatWithCommas(info.totalram) << std::endl;

					time_tt = ::time(NULL);
					std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
				}

				// sanity_check
				if (vEB.getMin().second != NULL) {
					throw std::runtime_error("Tree is not empty after complete delete");
				}

			}

		}

	}

	std::cout << "loops done: " << FormatWithCommas(loop_count) << std::endl;

	if (test_cancel_) {
		std::cout << "test canceled by user" << std::endl;
	}

	return 0;
}

int test_tree_performance_n_to_N_delete_end(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters,
		uint64_t loop_first, uint64_t loop_last, int64_t loop, bool insert_only, uint16_t verbose,
		const std::string & generator) {
	//std::random_device rd;
	//std::default_random_engine e1(rd());
	std::default_random_engine e1(0);
	uint64_t treeK = vEB.K();
	uint64_t cluster_num_bits = treeK / 2;
	uint64_t max_allowed_value = (__UINT64_C(1) << cluster_num_bits) * clusters - 1;
	std::uniform_int_distribution<uint64_t> uniform_dist(0, max_allowed_value); // u-1 is for k<=5, for k=6 could be 'u'
	uint64_t loop_count = 0;

	//::malloc_trim(0);

	if (loop_first < 1 || loop_first > loop_last) {
		throw std::runtime_error("Invalid loop_count_start");
	}

	time_t time_tt;

	std::cout << "Generating numbers in the closed interval [0, " << FormatWithCommas(max_allowed_value) << "]"
			<< std::endl;

	while (loop < 0 || loop-- > 0) {

		if (test_cancel_) {
			break;
		}

		if (verbose) {
			std::cout << "loop_count: " << FormatWithCommas(loop_count++) << std::endl;
		} else {
			loop_count++;
		}

		e1.seed(0);

		if (verbose) {
			std::cout << "loop= " << loop << std::endl;
			//::malloc_trim(0);
			std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
			struct sysinfo info;
			sysinfo(&info);
			std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
					<< FormatWithCommas(info.totalram) << std::endl;

			while (true) {
				try {
					std::cout << vEB.status() << std::endl;
					break;
				} catch (const std::exception & e) {
					std::cerr << "Exception in vEB.status() retrying in 1s." << std::endl;
					std::cerr << e.what() << std::endl;
					sleep(1);
				}
			}

			time_tt = ::time(NULL);
			std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////////

		uint64_t really_inserted = 0;

		for (uint64_t i = loop_first; i <= loop_last; i++) {

			if (test_cancel_) {
				break;
			}

			ofstream file_insert;
			file_insert.open(std::string("/home/edgard/dveb_insert_") + std::to_string(i) + std::string(".txt"),
					std::ios::out | std::ios::app);


			uint64_t max_j = __UINT64_C(1) << i;
			uint64_t j = 0;

			if (i == loop_first) {
				// sanity_check
				std::shared_ptr<ProtoData> min;
				while (true) {
					try {
						min = vEB.getMin().second;
						break;
					} catch(const std::exception & e) {
						std::cerr << "Exception in vEB.getMin().second retrying in 1s..." << std::endl;
						std::cerr << e.what() << std::endl;
						sleep(1);
					}
				}

				if (min != NULL) {
					throw std::runtime_error("Tree is not empty before start test");
				}
				j = 0;
			} else {
				j = __UINT64_C(1) << (i - 1);
			}

			auto start_ts = std::chrono::steady_clock::now();
			bool got_exception = false;

			for (; j < max_j; j++) {
				uint64_t x;
				if (generator[0] == 'c') {
					uint64_t cluster_to_insert = j % clusters;
					x = (j / clusters) & ProtoVEB::k_properties[cluster_num_bits].max;
					x = (cluster_to_insert << cluster_num_bits) | x;
					// x = j % (max_allowed_value + __UINT64_C(1)); to insert globally sequential
				} else if (generator[0] == 'u') {
					x = uniform_dist(e1);
				} else {
					std::runtime_error e(std::string("unknown generator: " + generator));
					throw e;
				}

				if (test_cancel_) {
					break;
				}

				std::shared_ptr<ProtoDataRaw> pdr(new ProtoDataRaw());
				std::shared_ptr<ProtoData> replaced;

				while (true) {
					try {
						replaced = vEB.insert(x, pdr);
						break;
					} catch (const std::exception & e) {
						std::cerr << "Exception in vEB.insert(x, pdr) retrying in 1s..." << std::endl;
						std::cerr << e.what() << std::endl;
						sleep(1);
						got_exception = true;
					}
				}

				if (replaced == NULL) {
					really_inserted++;
				} else {
					if (generator[0] == 'c') {
						std::runtime_error e("Aren't supposed to insert a number twice in cluster_seq mode.");
						throw e;
					}
				}

				if (test_status_) {
					std::cout << "loop_count: " << FormatWithCommas(loop_count) << std::endl;
					std::cout << "Inserted i=" << i << ", j=" << FormatWithCommas(j + __UINT64_C(1)) << "/"
							<< FormatWithCommas(max_j) << std::endl;
					test_status_ = false;
				}
			}

			if (test_cancel_) {
				break;
			}

			auto diff = std::chrono::steady_clock::now() - start_ts;
			auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

			if (got_exception) {
				file_insert  << "-1, ";
			} else {
				file_insert << std::to_string(elapsed_ms.count()) << ", ";
			}

			file_insert.close();

			if (verbose) {
				std::cout << "Inserted " << "i=" << i << ", finished in " << FormatWithCommas(elapsed_ms.count())
						<< " ms" << std::endl;
				std::cout << "really_inserted: " << FormatWithCommas(really_inserted) << "/" << FormatWithCommas(max_j)
						<< ", max_allowed_values: " << FormatWithCommas(max_allowed_value) << std::endl;
				//::malloc_trim(0);
				std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
				struct sysinfo info;
				sysinfo(&info);
				std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
						<< FormatWithCommas(info.totalram) << std::endl;

				while (true) {
					try {
						std::cout << vEB.status() << std::endl;
						break;
					} catch (const std::exception & e) {
						std::cerr << "Exception in vEB.status() retrying in 1s..." << std::endl;
						std::cerr << e.what() << std::endl;
						sleep(1);
					}
				}

				time_tt = ::time(NULL);
				std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
			}

			if (false == insert_only) {
				///////////////////////////////////////////////////////////////////////////////

				for(int repeat_search_count=0; repeat_search_count < 1; repeat_search_count++) {

					ofstream file_ismember;
					file_ismember.open(std::string("/home/edgard/dveb_ismember_") + std::to_string(i) + std::string(".txt"),
							std::ios::out | std::ios::app);

					uint64_t numer_of_searchs = 0;

					e1.seed(0);
					j = 0;

					got_exception = false;

					uint64_t step;
					const uint64_t max_number_of_searches = 100000LL;

					//if (max_j > max_number_of_searches) {
					//	step = max_j / max_number_of_searches;
					//} else {
					//	step = 1;
					//}
					step = 1;

					start_ts = std::chrono::steady_clock::now();

					for (; j < max_j;) {
						uint64_t x;
						if (generator[0] == 'c') {
							uint64_t cluster_to_ismember = j % clusters;
							x = (j / clusters) & ProtoVEB::k_properties[cluster_num_bits].max;
							x = (cluster_to_ismember << cluster_num_bits) | x;
							// x = j % (max_allowed_value + __UINT64_C(1)); to insert globally sequential
						} else if (generator[0] == 'u') {
							x = uniform_dist(e1);
						} else {
							std::runtime_error e(std::string("unknown generator: " + generator));
							throw e;
						}

						if (test_cancel_) {
							break;
						}

						bool is_member = false;

						while (true) {
							try {
								is_member = vEB.isMember(x);
								break;
							} catch (const std::exception & e) {
								std::cerr << "Exception in vEB.isMember(x) retrying in 1s..." << std::endl;
								std::cerr << e.what() << std::endl;
								sleep(1);
								got_exception = true;
							}
						}

						if (false == is_member) {
							throw std::runtime_error(std::to_string(x) + " is not member.");
						}

						if (test_status_) {
							std::cout << "loop_count: " << FormatWithCommas(loop_count) << std::endl;
							std::cout << "IsMember i=" << i << ", j=" << FormatWithCommas(j + __UINT64_C(1)) << "/"
									<< FormatWithCommas(max_j) << std::endl;
							test_status_ = false;
						}

						numer_of_searchs++;

						j += step;

						if (max_j < max_number_of_searches) {
							if (numer_of_searchs == max_number_of_searches) {
								break;
							}
							if (j >= max_j) {
								e1.seed(0);
								j = 0;
							}
						}

					}
					j = max_j;

					if (test_cancel_) {
						break;
					}

					diff = std::chrono::steady_clock::now() - start_ts;
					elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
					if (got_exception) {
						file_ismember << "-1, ";
					} else {
						file_ismember << std::to_string(elapsed_ms.count()) << ", ";
					}
					file_ismember.close();

					if (verbose) {
						std::cout << "i= " << i << ", max_j=" << max_j << ", numer_of_searchs=" << numer_of_searchs << std::endl;
						std::cout << "IsMember " << "i=" << i << ", finished in " << FormatWithCommas(elapsed_ms.count())
								<< " ms" << std::endl;
						//::malloc_trim(0);
						std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
						struct sysinfo info;
						sysinfo(&info);
						std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
								<< FormatWithCommas(info.totalram) << std::endl;

						time_tt = ::time(NULL);
						std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
					}

				}

				///////////////////////////////////////////////////////////////////////////////

				if (i == loop_last) {

					ofstream file_remove;
					file_remove.open(std::string("/home/edgard/dveb_remove_") + std::to_string(i) + std::string(".txt"),
							std::ios::out | std::ios::app);
					e1.seed(0);

					j = 0;

					got_exception = false;
					start_ts = std::chrono::steady_clock::now();

					for (; j < max_j; j++) {
						uint64_t x;
						if (generator[0] == 'c') {
							uint64_t cluster_to_ismember = j % clusters;
							x = (j / clusters) & ProtoVEB::k_properties[cluster_num_bits].max;
							x = (cluster_to_ismember << cluster_num_bits) | x;
							// x = j % (max_allowed_value + __UINT64_C(1)); to insert globally sequential
						} else if (generator[0] == 'u') {
							x = uniform_dist(e1);
						} else {
							std::runtime_error e(std::string("unknown generator: " + generator));
							throw e;
						}

						if (test_cancel_) {
							break;
						}

						while (true) {
							try {
								vEB.remove(x);
								break;
							} catch (const std::exception & e) {
								std::cerr << "Exception in vEB.remove(x) retrying in 1s..." << std::endl;
								std::cerr << e.what() << std::endl;
								sleep(1);
								got_exception = true;
							}
						}

						if (test_status_) {
							std::cout << "loop_count: " << FormatWithCommas(loop_count) << std::endl;
							std::cout << "Removed i=" << i << ", j=" << FormatWithCommas(j + __UINT64_C(1)) << "/"
									<< FormatWithCommas(max_j) << std::endl;
							test_status_ = false;
						}
					}

					if (test_cancel_) {
						break;
					}

					diff = std::chrono::steady_clock::now() - start_ts;
					elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
					if (got_exception) {
						file_remove << "-1, ";
					} else {
						file_remove << std::to_string(elapsed_ms.count()) << ", ";
					}
					file_remove.close();

					if (verbose) {
						std::cout << "Removed " << "i=" << i << ", finished in " << FormatWithCommas(elapsed_ms.count())
								<< " ms" << std::endl;
						//::malloc_trim(0);
						std::cout << "Process memory in bytes: " << FormatWithCommas(getUsedMemory()) << std::endl;
						struct sysinfo info;
						sysinfo(&info);
						std::cout << "Free system memory in bytes: " << FormatWithCommas(info.freeram) << "/"
								<< FormatWithCommas(info.totalram) << std::endl;

						time_tt = ::time(NULL);
						std::cout << "The current date/time is: " << ::asctime(::localtime(&time_tt)) << std::endl;
					}

					std::shared_ptr<ProtoData> min;
					while (true) {
						try {
							min = vEB.getMin().second;
							break;
						} catch(const std::exception & e) {
							std::cerr << "Exception in vEB.getMin().second retrying in 1s..." << std::endl;
							std::cerr << e.what() << std::endl;
							sleep(1);
						}
					}

					// sanity_check
					if (min != NULL) {
						throw std::runtime_error("Tree is not empty after complete delete");
					}

				}

			}

		}

	}

	std::cout << "loops done: " << FormatWithCommas(loop_count) << std::endl;

	if (test_cancel_) {
		std::cout << "test canceled by user" << std::endl;
	}

	return 0;
}

int test_intro3(uint64_t k) {
	const uint64_t test_count = 65535;

	uint64_t u;           // 65536
	//uint64_t sqrt_floor_u;      // 256
	//uint64_t sqrt_ceil_u;      // 256
	uint64_t log_u;       // 16
	uint64_t low_mask;    // 255 = 0x0F = 00001111

	if (k <= 64) {
		u = POW2(k);
		//sqrt_floor_u = sqrt_floor(u);
		//sqrt_ceil_u = sqrt_ceil(u);
		log_u = log2_floor(u);
		low_mask = low_mask_from_log(log_u);
	} else if (k == 64) {
		//                                               (pentabyte 10**15)-             (exabyte 10**18) = 8 765 432 109 876 543 210
		u = UINT64_MAX; // Can't be represented with 64 bits = UINT64_MAX + 1 = 18446744073709551615 + 1 = 18.446.744.073.709.551.616
		//sqrt_floor_u = 1 + (uint64_t)UINT32_MAX; // 1 +
		log_u = 64;
		low_mask = low_mask_from_log(log_u);
	} else {
		std::cout << "Are you kidding me?!" << std::endl;
		exit(-1);
	}

	// uint64_t max;
	// uint64_t sqrt_high_u;
	// uint64_t sqrt_low_u;
	// uint32_t low_mask;
	// uint8_t log_u;

	uint64_t up_sqrt_u = POW2((log_u >> 1) + (log_u & 1));
	uint64_t low_sqrt_u = POW2(log_u >> 1);
	//std::cout << "up_sqrt_u= " <<  up_sqrt_u << std::endl;
	//std::cout << "low_sqrt_u= " << low_sqrt_u << std::endl;

	std::cout << "{" << u - 1 << ", " << up_sqrt_u << ", " << low_sqrt_u << ", 0x" << std::hex << low_mask << std::dec
			<< "}, //" << k << std::endl;

	//std::cout << "u= " << u << ", sqrt_floor_u= " << sqrt_floor_u << ", sqrt_ceil_u= " << sqrt_ceil_u << ", log_u= " << log_u  << ", low_mask= 0x" << std::hex << low_mask << std::dec << std::endl;

	//std::random_device rd;
	//std::default_random_engine e1(rd());
	std::default_random_engine e1(0);
	std::uniform_int_distribution<uint64_t> uniform_dist(0, u - 1); // u-1 is for k<=5, for k=6 could be 'u'

	auto start_ts = std::chrono::steady_clock::now();

	for (uint64_t i = 0; i < test_count; i++) {
		uint64_t x = uniform_dist(e1);

		uint32_t high = x / low_sqrt_u;
		uint32_t low = x % low_sqrt_u;

		uint32_t bhigh = x >> (log_u >> 1);  // log_u / 2 =  log_u >> 1
		uint32_t blow = x & low_mask;

		if (high != bhigh || low != blow) {
			std::cerr << "differ! I'm a bad programmer!" << std::endl;
			printf("x=%llu(0x%02llx), high=0x%x, low=0x%x\n", (long long unsigned) x, (long long unsigned) x, high,
					low);
			printf("x=%llu(0x%02llx), bhigh=0x%x, blow=0x%x\n", (long long unsigned) x, (long long unsigned) x, bhigh,
					blow);
			break;
		}
	}

	auto diff = std::chrono::steady_clock::now() - start_ts;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

	//std::cout << "finished, elapsed= " << elapsed_ms.count() << " ms" << std::endl;

	return 0;
}
