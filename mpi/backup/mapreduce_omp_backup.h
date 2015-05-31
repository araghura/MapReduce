#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <queue>
#include <omp.h>
#include <functional>

std::map<std::string,int> mapreducefinal(int num_threads,int mpi_size, int mpi_rank);