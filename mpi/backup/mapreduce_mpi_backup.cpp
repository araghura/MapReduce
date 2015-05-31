#include "mapreducefinal.h"
#include <mpi.h>
#include <cstring>

int main(int argc, char** argv)
{
  using namespace std;
  
  int myrank, mpi_size, interval_calculated;
  int i, j;
  double time1, time2, time3;  // for timing measurements

  vector<queue<pair<string,int>>> local_queues;
  
  int* word_count_proc; 
  int* final_word_count_proc; 


  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);

  MPI_Status ireq[128];
  MPI_Status status;
  word_count_proc = (int*)malloc(mpi_size*sizeof(int));
  final_word_count_proc = (int*)malloc(mpi_size*sizeof(int));

  //Input map. So everybody will use this to get the output of the file that they are reading.
  map<string,int> input_map;
  map<string,int> finalMap; //final output
  int num_threads = 4;

  input_map = mapreducefinal(num_threads,mpi_size,myrank);
  
  // for(map<string,int>::iterator it = input_map.begin(); it != input_map.end(); ++it)
  // {
   //   cout<<it->first<< "=> "<<it->second<<endl;
  // }

 // cout<<"map size of process"<<myrank<<": "<<input_map.size()<<endl;
  //Create empty queues and push into a vector
  for(int i = 0 ; i < mpi_size; i++)
  {
    local_queues.push_back(queue<pair<string,int>>());
  }
  

  hash<string> hash_fn;
  size_t hash_out;
  //use hash function to determine which word goes to that process' queueu
  for(map<string,int>::iterator it = input_map.begin(); it != input_map.end(); ++it)
  {
      string word = it->first;
      int count_of_word = it->second;
      hash_out = hash_fn(word); 
      int identifier = hash_out % mpi_size;
      local_queues[identifier].push(make_pair<string,int>(word, count_of_word));
  }

  //total numbers of words to be sent to other processes 
  int total_send_count = 0;
  // cout<<"====Checking===="<<myrank<<endl; 
  for(int i = 0 ; i < mpi_size; i++)
  {
  // cout<<"Size of queue"<<i<< "= "<<local_queues[i].size()<<endl;
    word_count_proc[i] = local_queues[i].size();
    if(i!=myrank)
        total_send_count+=local_queues[i].size();
  }

  //Mpi reduce all
  MPI_Allreduce(word_count_proc, final_word_count_proc, mpi_size, MPI_INT, MPI_SUM,MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);
  
  // cout<<"Process "<<myrank<<" will receive "<<final_word_count_proc[myrank]<<" words"<<endl;
  


  //Now every process knows how many words to receive

  //Enter a long never ending loop. 
  //flag for sending all words
  //flag for receiving all words
  //if both are set then, exit this loop
  
  
  //if my mpirank is x, I will receive x times
  //then, I will send to everyone
  //then, I will receive mpi_size
  // cout<<"===total words to send from rank=== "<<myrank<<"= "<<total_send_count<<endl;
  
    MPI_Barrier(MPI_COMM_WORLD);
    
//  int receive_count = 0;
//  int send_count = 0;
//  int isSendOver = 0;
//  int isRecvOver = 0;
//  
 
  //for each queue
  //get a word and its count
  //dont send words to yourself.
  for(int j=0;j<myrank;j++)
  {
         MPI_Status status; 
         int queue_size;
         int char_length;
         char *word_array;
      
    // cout<<endl<<endl<<"My rank is "<<myrank<<" and i am going to receive from "<<j<<endl; 
      
         MPI_Recv(
              &queue_size,
              1,
              MPI_INT,
              j,
              0,
              MPI_COMM_WORLD,
              &status);
        
       //cout<<endl<<endl<<"My rank is "<<myrank<<" and i'll receive "<<queue_size<<"from "<<j<<endl; 
              
         while(queue_size-- >0)
         {         
           //receive word length
           MPI_Recv(
             &char_length,
             1,
             MPI_INT,
             MPI_ANY_SOURCE,
             0,
             MPI_COMM_WORLD,
             &status);
     
           //word_array = new char[char_length];
           word_array = (char *)malloc(sizeof(char)*char_length);
           //receive the word
           MPI_Recv(
             word_array, 
             char_length, 
             MPI_CHAR, 
             status.MPI_SOURCE,
             1,
             MPI_COMM_WORLD, 
             &status
             );
             
           int word_count;
           
           //Convert Char array to string
           string word_string(word_array);
                 
           //receive word count
           MPI_Recv(
             &word_count,
             1,
             MPI_INT,
             status.MPI_SOURCE,
             2,
             MPI_COMM_WORLD,
             &status);
             
           // ADD IT TO THE MAP
            map<string,int>::iterator keyfind = finalMap.find(word_string);
            if (keyfind == finalMap.end())
            {
                finalMap.insert(make_pair(word_string,word_count));
            }
                   
            //Insert
            else
            {
                (keyfind->second) = (keyfind->second) + word_count;
            }
     }
  }
         
  for(int i = 0; i < mpi_size ; i++)
  {
      if(i!=myrank)
      {
         int queue_size = local_queues[i].size();
      
     // cout<<endl<<endl<<"My rank is "<<myrank<<" and i am going to send to "<<i<<endl;
      
         MPI_Send(
              &queue_size,
              1,
              MPI_INT,
              i,
              0,
              MPI_COMM_WORLD);
         
        // cout<<endl<<endl<<"My rank is "<<myrank<<" and i'm sending "<<queue_size<<"to "<<i<<endl;
         
         while(!local_queues[i].empty())
         {
            pair<string, int> word_count_pair = local_queues[i].front();
            string word = word_count_pair.first;
            int count = word_count_pair.second;
            
            //Send String as c-string along with length
            const char* temp_word = word.c_str();
            
                      
            int char_length = strlen(temp_word) + 1;
            
            MPI_Send(
              &char_length,
              1,
              MPI_INT,
              i,
              0,
              MPI_COMM_WORLD);
            
            
            MPI_Send(
            (void*)temp_word,
            char_length,
            MPI_CHAR,
            i,
            1,
            MPI_COMM_WORLD);
            
            //send word count
            MPI_Send(
              &count,
              1,
              MPI_INT,
              i,
              2,
              MPI_COMM_WORLD);
            local_queues[i].pop(); 
           //cout<<"I Rank, "<<myrank<<" sent "<<count<<endl;
        }
        
      }
      else //if myrank == i
      {
      //cout<<endl<<endl<<"My rank is "<<myrank<<" and i'm adding to myself "<<local_queues[i].size()<<" elements "<<endl;
         while(!local_queues[i].empty())
         {
            //cout<<endl<<endl<<"My rank is "<<myrank<<" and i have added "<<local_queues[i].front().first<<" and "<<local_queues[i].front().second<<endl;
            string word_string = local_queues[i].front().first;
            int word_count = local_queues[i].front().second;
              // ADD IT TO THE MAP
            map<string,int>::iterator keyfind = finalMap.find(word_string);
            if (keyfind == finalMap.end())
            {
                finalMap.insert(make_pair(word_string,word_count));
            }
                   
            //Insert
            else
            {
                (keyfind->second) = (keyfind->second) + word_count;
            }
            
            local_queues[i].pop();
         }
      }      
  }
  
  
  for(int j=myrank+1;j<mpi_size;j++)
  {
          //cout<<endl<<endl<<"My rank is "<<myrank<<" and i am going to receive from "<<j<<endl; 
         
           
         MPI_Status status; 
  
         int queue_size;
         int char_length;
      
         MPI_Recv(
              &queue_size,
              1,
              MPI_INT,
              j,
              0,
              MPI_COMM_WORLD,
              &status);
         
         //cout<<endl<<endl<<"My rank is "<<myrank<<" and i'll receive "<<queue_size<<"from "<<j<<endl;
          
         while(queue_size-- >0)
         {         
           //receive word length
           MPI_Recv(
             &char_length,
             1,
             MPI_INT,
             MPI_ANY_SOURCE,
             0,
             MPI_COMM_WORLD,
             &status);
     
           char *word_array;
           //word_array = new char[char_length];
           word_array = (char *)malloc(sizeof(char)*char_length);
           //receive the word
           MPI_Recv(
             word_array, 
             char_length, 
             MPI_CHAR, 
             status.MPI_SOURCE,
             1,
             MPI_COMM_WORLD, 
             &status
             );
             
           int word_count;
           
           //Convert Char array to string
           string word_string(word_array);
                 
           //receive word count
           MPI_Recv(
             &word_count,
             1,
             MPI_INT,
             status.MPI_SOURCE,
             2,
             MPI_COMM_WORLD,
             &status);
             
           // ADD IT TO THE MAP
            map<string,int>::iterator keyfind = finalMap.find(word_string);
            if (keyfind == finalMap.end())
            {
                finalMap.insert(make_pair(word_string,word_count));
            }
                   
            //Insert
            else
            {
                (keyfind->second) = (keyfind->second) + word_count;
            }
     }
  }
  
  for(map<string, int>::iterator iter = finalMap.begin(); iter!=finalMap.end(); iter++)
  {
           cout<<"Rank "<<myrank<<" "<<iter->first<<" --> "<<iter->second<<endl;
  }

 // cout<<"I Rank, "<<myrank<<" have fullfilled my duties. I will now take my leave"<<endl;
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
  
  return 0;
 } //main

