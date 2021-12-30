#include <stdio.h>
#include "mpi.h"
#include <string.h>
#include <stdlib.h>

int main(int argc,char ** argv )
{
  int rank, size;
  int msgleft, msgright;
  int send_ltag, send_rtag;
  int recv_ltag, recv_rtag;
  int lvalue, rvalue;
  int lneighbour, rneighbour;
  int cnt;
  int iterations;
  double start_time;
  double end_time;
  MPI_Status status;
  MPI_Init( &argc, &argv );
  
  if(argc==2)
     iterations = (atoi(argv[1])>1)?atoi(argv[1]):1;
  else
     iterations = 1;
  start_time = MPI_Wtime();
  for(int it = 1 ; it <= iterations ; ++it)
  {
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  cnt=0;
  rvalue=0;
  lvalue=0;
  send_rtag = 10* rank;
  send_ltag = 10* rank;
  
  for(int i = 0; i < size; ++i)
    {
      msgleft    = + rank + rvalue ;
      msgright   = - rank + lvalue ;
      lneighbour = ( rank + size - 1) % size ;
      rneighbour = ( rank + 1 ) % size ;
      recv_rtag =  ( rank + 1 + i ) % size * 10 ;
      MPI_Sendrecv( &msgleft, 1, MPI_INT, lneighbour, send_ltag,&rvalue, 1, MPI_INT, rneighbour, recv_rtag, MPI_COMM_WORLD, &status );
      // MPI_Send( &msgleft, 1, MPI_INT, lneighbour, send_ltag, MPI_COMM_WORLD );
      // printf("I am process %d, this is iteration %d, and 1) i am sending to %d a message with tag %d and value %d\n", rank, i, lneighbour, send_ltag, msgleft);
      
      //MPI_Recv( &rvalue, 1, MPI_INT, rneighbour, recv_rtag, MPI_COMM_WORLD, &status );
      // printf( "I am process %d, this is iteration %d, and 2) i am receiving from %d a message with tag %d and value %d\n", rank, i, rneighbour, recv_rtag, rvalue);
      ++cnt;
      recv_ltag =  ( rank + size - 1 - i) % size * 10 ;
      MPI_Sendrecv(  &msgright, 1, MPI_INT, rneighbour, send_rtag, &lvalue, 1, MPI_INT, lneighbour, recv_ltag, MPI_COMM_WORLD, &status );
      //MPI_Send( &msgright, 1, MPI_INT, rneighbour, send_rtag, MPI_COMM_WORLD );
      //printf( "I am process %d, this is iteration %d, and 3) i am sending to %d a message with tag %d and value %d\n",rank,i,rneighbour, send_rtag, msgright);
     
      // MPI_Recv( &lvalue, 1, MPI_INT, lneighbour, recv_ltag, MPI_COMM_WORLD, &status );
      //printf( "I am process %d, this is iteration %d, and 4) i am receiving from %d a message with tag %d and value %d\n",rank,i,lneighbour, recv_ltag, lvalue);
      ++cnt;
      send_rtag = recv_ltag;
      send_ltag = recv_rtag;
    }
 }
  end_time = MPI_Wtime();
  printf ( "I am process %d and I have received %d messages. My final messages have tags %d, %d; values %d, %d\n", rank, cnt, recv_ltag, recv_rtag, lvalue, rvalue );
  printf ( "# walltime on processor %i after %i iterations %10.8f. Average: %10.8f \n", rank, iterations, end_time - start_time, (end_time-start_time)/iterations ) ;
  MPI_Finalize( );
  return 0;
}
