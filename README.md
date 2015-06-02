MapReduce Implemented in C++ Using OpenMP and MPI Frameworks
=============================================================

Authors: Anantha Raghuraman and Jayakumaran Ravi 
------------------------------------------------
  


# Goal of the Project

1) Read all the words from multiple files and obtain a count of all the unique words.

2) Use MPI framework to parallelize the work and run on multiple nodes. 

3) Further, on each node, use OpenMP to run the work on multiple threads.


Project Overview
-----------------------
MapReduce is a programming model that involves two steps. The first, the map step, takes an input set I and groups it into N equivalence classes I0, I1, I2, ..., IN-1. I can be thought of as a set of tuples <key, data>, and the function map maps I into the equivalence classes based on the value of key. In the second reduce step, the equivalence classes are processed, and the set of tuples in an equivalence class Ij are reduced into a single value.

1) The project uses MPI and OpenMP frameworks by including 'mpi.h' and 'omp.h' header files and using the right compilation commands (see later).


2) There are 4 important files in this implementation. They are described below:

	a) list.txt: The list of files to be read is stored here.

	b) mapreduce_omp.cpp: This defines a function that does initial mapreduce. This function is called in mapreduce_mpi.cpp.

	c) mapreduce_omp.h: This header file declares the function defined in mapreduce_omp.cpp.

	d) mapreduce_mpi.cpp: This uses the MPI libraries to parallelzie Mapreduce oprations on multiple nodes (processes). It finally prints the result to stdout.


mapreduce_omp.cpp
-----------------

a) If there are N files to read and k processes, then (approx) N/k files are read by each process. Each process further divides the workload amongst multiple threads.

b) There are three kinds of threads:

	i. Reader threads (all even thread ids), which read files and put the data read into a work queue 1. Each work item will be a word.

	ii. Mapper threads (all odd thread ids), which execute in parallel with Reader threads, create combined records of words. That is, if one mapper thread dequeues 100 instances of "cat" and 50 instances of "dog" and so on.., its output will have {<"cat", 100>, <"dog", 50>, ...}.  

	Mappers and Readers work in parallel and interact by means of the work queue 1. The access to the work queue is controlled by locks. After Reader threads are finished and the work queue is empty, the Reader threads become Reducer threads.  

	iii. Reducer threads that operate on work queue entries created by mapper threads and combine (reduce) them to a single record.  Thus, for the word “cat”, there is potentially a <“cat”,counti> record sent by every mapper thread ti in the system and it will sum all of the counts and place it on a work queue 2.  

	Note: For each word there is exactly one Reducer thread in the system that handles it. A hash function's output is used to figure out which reducer will reduce a particular word.

c) As each reducer finishes its reduce work, the result for a word is updated in a final key-value pair map (e.g. <"cat", 1000>) and after all of them are done, the map is returned.

mapreduce_mpi.cpp
-----------------

a) After each process obtains its rank, it calls the mapreduce_omp function that performs the above mentioned tasks.

b) Now, each process has an initial key-value map that contains set of all unique words encountered in the books that it read and their counts.

c) Now, for the final reduce step, all the instances of a particular word (let's say "Python"), must be sent to a single process to reduce. 

d) As before, a hash function output is used to decide which process should receive the entry corresponding to a particular word. 

e) Now, each of the k processes keep a set of k maps, one for each process and stores the key-value pair that it has to send to that particular process in that particular map. For example, if "Python" should go to process 2, all process will store the <"Python", local count> in map 2. 

f) Finally, all processes communicate with each other to send the words and counts.

g) Now, we are ready to perform the final reduction. As processes receive messages from others, they perform the final reduction.

h) The outputs are then sent to stdout.


Setting Number of Threads
-------------------------
Number of threads (on each process) on which the algorithm must run can be set in mapreduce_mpi.cpp. At the begining of the code, there is a #define directive to set this.


Compilation Command
-------------------
mpic++ -std=c++0x mapreduce_mpi.cpp mapreduce_omp.cpp -o mapreduce_mpi -fopenmp  


Run Command
-----------
mpiexec -n 4 ./mapreduce_mpi

This specifies the number of processes on which we want to run the algorithm.


Test Run
--------
Few files have been uploaded and list.txt contains the names of these files, one per line. The code can be run on this dataset to test.



