/*
 * TestIntro3.hh
 *
 *  Created on: Jul 9, 2015
 *      Author: edgard
 */

#ifndef TESTINTRO3_HH_
#define TESTINTRO3_HH_

#include "dveb/ProtoVEB.hh"

extern volatile bool test_cancel_;
extern volatile bool test_status_;

extern int test_intro3(uint64_t k);

extern int print_tree(ProtoVEB & vEB, uint64_t used_mem_start);

extern int test_tree(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters, uint64_t loop_count);

extern int test_tree_performance_N_by_N(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters,
		uint64_t loop_count);

extern int test_tree_performance_n_to_N(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters, uint64_t loop_first,
		uint64_t loop_last, int64_t loop, bool insert_only, uint16_t verbose, const std::string & generator);

extern int test_tree_performance_n_to_N_delete_end(ProtoVEB & vEB, uint64_t used_mem_start, uint64_t clusters,
		uint64_t loop_first, uint64_t loop_last, int64_t loop, bool insert_only, uint16_t verbose,
		const std::string & generator);

#endif /* TESTINTRO3_HH_ */
