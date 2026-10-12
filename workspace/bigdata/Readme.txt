# create eclipse project

mkdir eclipse
cd eclipse
cmake -G"Eclipse CDT4 - Unix Makefiles" -D_ECLIPSE_VERSION=4.6 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER_ARG1=-std=c++1y ..


