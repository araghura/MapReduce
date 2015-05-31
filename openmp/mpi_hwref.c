#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<mpi.h>
#define Wcount 1
#define Ncount 256
#define Nproc 16


int main(int argc, char *argv[])
{ 
  srand(time(NULL));
  int pid, numP,i;
  int count = 0;
  int t =0;
  float msg[Wcount];
  float sendmsg1[Wcount];
  float sendmsg2[Wcount];
  float work[Ncount];
  int mymsg;
  
  double t1,t2;
//  printf("Enter the array size\n");
//  scanf("%d",&array_size);
  for(i=0;i<Wcount;i++)
  {
    sendmsg1[i] = -1;
    sendmsg2[i] = -2;
  }
    
  for(i=0;i<Ncount;i++)
  {
   work[i] = ((rand()%5) +1.0)/5;
  }

  int rc = MPI_Init(&argc, &argv); // can be null
  t1 = MPI_Wtime();

  if (rc != MPI_SUCCESS) 
   { // is MPI_ERR_OTHER
   printf("Error in MPI INIT\n");
   exit(1);
   }
   

  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);

 
  while(1)
  {
  
   if(pid ==0 && count < Nproc-1)
   {
    MPI_Recv(&mymsg, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if(t+Wcount<Ncount)
    {
     MPI_Send(work+t, Wcount, MPI_FLOAT, mymsg, 0, MPI_COMM_WORLD);
     printf("Sending work to %d and t is %d\n",mymsg,t);
     t+= Wcount;
    }
    else if(count++ < Nproc-1)
    {
     MPI_Send(&sendmsg1, Wcount, MPI_FLOAT, mymsg, 0, MPI_COMM_WORLD);
     printf("Sending -1 to %d and count is %d\n",mymsg,count);
    }
    else
    {
     printf("I am going to broadcast\n");
     break;
    }
   }
   else if(pid ==0 && count >= Nproc-1)
   {
    printf("Leaving the loop and count is %d\n",count);
    break;
   }

 
   else
   {
    MPI_Send(&pid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Recv(&msg, Wcount, MPI_FLOAT,0,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if(msg[0]>0)
    {
     for(i=0;i<Wcount;i++)
      {
       sleep(msg[i]);
      }
    }
    else if(msg[0] == -1)
    {
     printf("Process %d got -1\n",pid);
     break;
    }
   }
   
  }
 MPI_Barrier(MPI_COMM_WORLD);
 MPI_Bcast(&sendmsg2,Wcount,MPI_FLOAT,0,MPI_COMM_WORLD);
 t2 = MPI_Wtime();
 if(pid ==0)
 printf("The value of the time  is %0.5f\n",(t2-t1));
 MPI_Finalize();

 return 0;
}
