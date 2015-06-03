#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <queue>
#include <omp.h>
#include <functional>

std::map<std::string,int> mapreduce_omp(int num_threads,int mpi_size, int mpi_rank);
