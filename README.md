# DvEB: Distributed van Emde Boas Tree

Research prototype of a **distributed van Emde Boas (vEB) tree**: a vEB tree whose universe
grows on demand (from 2^(2^k) to 2^(2^(k+1))), whose nodes are named by identifiers derived
from their position in the tree, and whose nodes are spread over machines and located by
IP multicast. Keys are arbitrary-precision integers (GMP); the experiments used universes up
to 2^131072.

This repository is a snapshot of the code used in the experiments of the master's
dissertation

> E. N. A. G. Lima. *Distributed van Emde Boas Tree*. Master's dissertation, Graduate Program
> in Applied Informatics, Federal Rural University of Pernambuco (UFRPE), Recife, Brazil, 2017.
> DOI: [10.5281/zenodo.23308871](https://doi.org/10.5281/zenodo.23308871)

The dissertation describes the design, the software architecture (Appendix B), the program
options (Appendix A) and the exact configuration used in each experiment (Appendix C).

**Status:** research code, published as is (last change: March 2017). It is not maintained.

## Layout

| Path | Content |
|------|---------|
| `workspace/bigdata/` | Core library: vEB algorithms (`src/veb/`), root proxy and universe growth (`veb_root.cc`), node identifiers (`id.cc`), node placement (`factory.cc`), registry of hosted nodes, multicast/unicast channel and server, request–reply protocol (`tree_rpc_multicast.cc`), flow control, statistics, and the GMP wrapper (`natural.hh`). |
| `workspace/test/` | Test and experiment program: correctness tests that compare every operation with a reference `std::vector`, and the performance experiments. |
| `workspace/DvEB/`, `workspace/DvEBserver/` | An earlier prototype (2015–2016), superseded by `bigdata` and `test` in January 2017; kept for reference. |
| `workspace/*.sh` | Helper scripts used to prepare the experiment machines. |

## Requirements

As used in 2017 (Debian 9, g++ 6.3):

- Linux, a C++14 compiler and CMake
- Boost: log, log_setup, system, thread, program_options, unit_test_framework
- GMP with C++ bindings (`libgmp`, `libgmpxx`)
- libcoap (the experimental CoAP transport is linked as `coap-1`, i.e. the libcoap 4.1 API;
  headers expected in `/usr/local/include/coap`)

Newer compilers and library versions (notably libcoap ≥ 4.2) may require small changes.

## Build

```sh
cd workspace
mkdir build_b && cd build_b && cmake -DCMAKE_BUILD_TYPE=Release ../bigdata && make
cd ..
mkdir build_t && cd build_t && cmake -DCMAKE_BUILD_TYPE=Release ../test && make
```

The library and the `test` executable are written to `workspace/build/`.

## Run

Every machine runs the same program with a configuration file read from standard input:

```sh
./test help > name.conf      # write a configuration file with all options
# edit name.conf (role = root | node | cheater, multicast group, timeouts, test mode, ...)
./test -- < name.conf
```

In the experiments of the dissertation, one machine ran the root and the workload, several
machines hosted tree nodes, and one machine ran the optional *cheater*. Appendix C of the
dissertation lists the configuration file of each role and the kernel settings
(socket buffer sizes) used.

## License

MIT; see [LICENSE](LICENSE).
