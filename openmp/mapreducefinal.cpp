#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <queue>
#include <omp.h>
#include <functional>


using namespace std;

int main(int argc, char* argv[])
{
    bool allPushed = false;
    ifstream file;
    //output stream
    ofstream ofile;
    //file list
    vector<string> fileList;
    
    //queue
    queue<string> wordStream;
    //Lock for word outputs
    omp_lock_t wordStream_lock;

    //Lock for word outputs initialization
    omp_init_lock(&wordStream_lock);

    //open a file
    file.open("list.txt");
    string fileName;
    double elapsedTime;
    int number_of_threads = 4; //default;
    
    
    //Stuff to be added inside main, but before the start of parallelization
    //array of queues for the reducers
    vector<queue<pair<string,int>>> reducerQueues; // vector of queues
    int num_e_threads;
    
    if(argc > 1)
    {
        sscanf(argv[1], "%d", &number_of_threads);
    }
    
    omp_set_num_threads(number_of_threads);
    
    
    
    #pragma omp parallel
    {
        #pragma omp critical
        {
            num_e_threads = (omp_get_num_threads() +1) /2; //Number of threads with even indices
        }
        
    }
    
    cout<<" num of thread: "<<number_of_threads<<endl;
    cout<<"num e threads :"<<num_e_threads<<endl;
    int j;

    //Lock for reducer queues
    omp_lock_t reducerQueues_lock[num_e_threads];

    for(j=0; j<num_e_threads; j++)
    {
        reducerQueues.push_back(queue<pair<string,int>>()); // add a queue
        //Lock for reducer queues initialization
    	omp_init_lock(&reducerQueues_lock[j]);
    }
    
    
    //datasent for reducers to keep track of which mappers have finished sending data
    unsigned short datasent=0;

    ////End of stuff to be put before the parallelization ////
    
    
    //build a list of input files
    while(file >> fileName)
    {
        fileList.push_back(fileName);
        //cout<<fileName<<endl;
    }
    
    
    file.close();
    int filenum = fileList.size()-1;
    map<string,int> wordMap;
    
    elapsedTime = -omp_get_wtime();
    #pragma omp parallel shared(filenum, wordStream) private(wordMap)
    {
        while(1)
        {
            if(omp_get_thread_num()%2 ==0) //Reader
            {
                //#pragma omp critical
                //cout<<"file num:"<<filenum<< ": "<< omp_get_thread_num()<<endl;
                if(filenum < 0)
                {
                    break;
                }
                
                
                ifstream file;
                string word;
                int fileOpen = 0;
                #pragma omp critical
                {
                    if(filenum >= 0)
                    {
                        //cout<<"Opening file: "<<fileList[filenum]<<endl;
                        file.open(fileList[filenum]);
                        if(!file.good())
                        {
                            cout<<fileList[filenum]<<" -> File is wrong"<<endl;
                        }
                        
                        // //cout<<"file num:"<<filenum<<endl;
                        filenum--;
                        fileOpen = 1;
                        
                    }
                    
                }
                
                if(fileOpen)
                {
                    while ( file >> word )
                    {
                        //cout<<word<<endl;
                        omp_set_lock(&wordStream_lock);
                        wordStream.push(word);
                        omp_unset_lock(&wordStream_lock);
                        
                    }
                    
                    file.close();
                    if(filenum < 0)
                    {
                        #pragma omp critical
                        {
                            allPushed = true;
                            //cout <<omp_get_thread_num()<< "-> set all Pushed" <<endl;
                        }
                        
                    }
                    
                }
                
                
                if(number_of_threads == 1)
                {
                    while(filenum>=0)
                    {
                        //cout<<"Opening file: "<<fileList[filenum]<<endl;
                        file.open(fileList[filenum]);
                        if(!file.good())
                        {
                            cout<<fileList[filenum]<<" -> File is wrong"<<endl;
                        }
                        
                        // //cout<<"file num:"<<filenum<<endl;
                        filenum--;
                        fileOpen = 1;
                        while ( file >> word )
                        {
                        	omp_set_lock(&wordStream_lock);
                            wordStream.push(word);
                            omp_unset_lock(&wordStream_lock);
                                                        
                        }
                        
                        file.close();
                    }
                    
                    allPushed = true;
                }
                
            }
            //if even threads
            
            //Odd Thread Execution Mappers
            if(number_of_threads == 1 || (omp_get_thread_num()%2!=0))
            {
                string w;
                int newword = 0;
                omp_set_lock(&wordStream_lock);
                if(wordStream.empty() && allPushed)
                {
          	     omp_unset_lock(&wordStream_lock);
                     break;
                }
                
                
                
                if(!wordStream.empty())
                {
                        newword = 1;
                        w = wordStream.front();
                        wordStream.pop();
                }
                
                omp_unset_lock(&wordStream_lock);
                
                
                if(newword && number_of_threads>1)
                {
                    map<string,int>::iterator keyfind = wordMap.find(w);
                    if (keyfind == wordMap.end())
                    {
                        wordMap.insert(make_pair(w,1));
                    }
                    
                    //Insert
                    else
                    {
                        (keyfind->second)++;
                    }
                    
                }
                
                
                else if(newword && number_of_threads == 1)
                {
                    map<string,int>::iterator keyfind1t = wordMap.find(w);
                    if (keyfind1t == wordMap.end())
                    {
                        wordMap.insert(make_pair(w,1));
                    }
                    
                    //Insert
                    else
                    {
                        (keyfind1t->second)++;
                    }
                    
                    omp_set_lock(&wordStream_lock);
                    while(!wordStream.empty())
                    {
                        w = wordStream.front();
                        wordStream.pop();
                        keyfind1t = wordMap.find(w);
                        if (keyfind1t == wordMap.end())
                        {
                            wordMap.insert(make_pair(w,1));
                        }
                        
                        //Insert
                        else
                        {
                            (keyfind1t->second)++;
                        }
                        
                        
                    }
                    omp_unset_lock(&wordStream_lock);
                    //wordstream empty
                    break;
                }
                //else if num thread = 1
            }
            
            
            
            
        }
        //while(1)
        
//        #pragma omp barrier lock_ananth
        ////////////////// Ananth's code  /////////////////////////////
        
        omp_destroy_lock(&wordStream_lock);
        //Even numbers are reducers
        //Odd numbers are mappers
        
        //Done with reading and mapping. Now, moving onto reducing and writing
        
        
        
        if(omp_get_thread_num()%2 ==1 || number_of_threads == 1) // Mapper
        {
            int count_word, identifier;
            string word;
            hash<string> hash_fn;
            size_t hash_out;
            
            for(map<string,int>::iterator words_it = wordMap.begin(); words_it!=wordMap.end(); words_it++)
            {
                count_word = words_it->second;
                word = words_it->first;
                hash_out= hash_fn(word);
                identifier = hash_out % num_e_threads;
                if(number_of_threads == 0)
                identifier = 0;
                
                omp_set_lock(&reducerQueues_lock[identifier]);
                reducerQueues[identifier].push(make_pair<string,int>(word,count_word)); // push an element into queue for reducers
                omp_unset_lock(&reducerQueues_lock[identifier]);
                
                
            }
            
            #pragma omp critical
            {
                datasent++;
                //cout<<"DAta sent incremented to: "<<datasent<<endl;
            }
            
        }
        
        //End of Ananth's code //////////////////////////////////
        
        
        
        //reducers - > even threads
        if(omp_get_thread_num() % 2 == 0)
        {
            
            map<string, int> word_count_redux;
            while(!reducerQueues[omp_get_thread_num()/2].empty() || (datasent!=num_e_threads))
            {
            	omp_set_lock(&reducerQueues_lock[omp_get_thread_num()/2]);
                if(!reducerQueues[omp_get_thread_num()/2].empty())
                {
                    pair<string, int> word_count_pair = reducerQueues[omp_get_thread_num()/2].front();
                    reducerQueues[omp_get_thread_num()/2].pop();
                    omp_unset_lock(&reducerQueues_lock[omp_get_thread_num()/2]);
                    
                    map<string,int>::iterator keyfound = word_count_redux.find(word_count_pair.first);
                    if (keyfound == word_count_redux.end())
                    {
                        word_count_redux.insert(make_pair(word_count_pair.first,word_count_pair.second));
                    }
                    
                    //Insert
                    else
                    {
                        (keyfound->second)+=word_count_pair.second;
                    }
                    
                }
                else
                {
                	omp_unset_lock(&reducerQueues_lock[omp_get_thread_num()/2]);
                }
                
            }
            //while (!reducerQueue)
            
            
            
            
            //cout<<"Reducer queues exhausted by thread: "<<omp_get_thread_num()<<endl;
            //iterate word_count_redux
            for(map<string, int>::iterator iter = word_count_redux.begin(); iter!=word_count_redux.end(); iter++)
            {
                #pragma omp critical
                {
                    if(!ofile.is_open())
                    {
                        cout<<"File opened for output by "<<omp_get_thread_num()<<endl;
                        ofile.open("output.txt", ofstream::out | ofstream::app);
                    }
                    
                    if(ofile.is_open())
                    {
                        ofile<<iter->first <<" => "<<iter->second<<endl;
                    }
                    
                    //cout<<iter->first <<" => "<<iter->second<<" id: "<<omp_get_thread_num()<<endl;
                }
                
            }
            
            
        }

    }

    for(int i=0;i<num_e_threads;i++)
    	omp_destroy_lock(&reducerQueues_lock[i]);

    //omp parallel
    elapsedTime += omp_get_wtime();
    ofile.close();
    cout<<"Elapsed time with "<<number_of_threads<<" threads: "<<elapsedTime<<endl;
    
    return 0;
}
