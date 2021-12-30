#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"
#include <string.h>
#define SEED 35791246

void first_method(int *local_array_sizes, int *starting_point_array, int * dims, int * gsizes, int ndims, int world_size )
{
  int array_sizes[world_size*ndims];
  int coordinates[world_size*ndims];
  int rem;
  int p = 0;
  for(int p1 = 0 ; p1 < dims[0]; ++p1)
    for(int p2 = 0 ; p2 < dims[1]; ++p2)
      for(int p3 = 0 ; p3 < dims[2]; ++p3)
    {
      coordinates[p*ndims+0]=p1;
      coordinates[p*ndims+1]=p2;
      coordinates[p*ndims+2]=p3;
      p++;
    }
  
  for(int p = 0 ; p <  world_size; ++p)
    for(int d = 0; d < ndims; ++d)
      {
    array_sizes[p*ndims+d] = gsizes[d]/dims[d];
    rem = gsizes[d]%dims[d];
    if (rem && coordinates[p*ndims+d] < rem)
      array_sizes[p*ndims+d]++;
      }
  
  for(int p = 0;p<world_size;++p)
      local_array_sizes[p] = array_sizes[p*ndims+0]* array_sizes[p*ndims+1] *array_sizes[p*ndims+2];
  starting_point_array[0]=0;
  for(int p = 1; p<world_size;++p)
    starting_point_array[p]=(starting_point_array[p-1]+local_array_sizes[p-1]);
}

void zero_method(int *local_array_sizes, int *starting_point_array, int world_size, int total_number)
{
  int rem = (total_number) % world_size ;
  for(int p = 0; p < world_size; ++p)
    {
      local_array_sizes[p] = (total_number) / world_size;
      if(p<rem)
      ++local_array_sizes[p];
    }
  starting_point_array[0]=0;
  for(int p = 1; p<world_size;++p)
    starting_point_array[p]=(starting_point_array[p-1]+local_array_sizes[p-1]);
}

int main( int argc, char *argv[] )
{
  int n_iterations = 1; // 50
  int cnt;
  //  Declare variables (or do it later)
  int n ; // rows of global matrix (i)
  int m ; // cols of global matrix (j)
  int l ; // 3rd dimensional rows of global matrix (k)
  int method;
  double end_time, scatter_time_f, gather_time_f;
  double start_time, scatter_time_i, gather_time_i;

  //  Start MPI
  MPI_Init( &argc, &argv );
  start_time = MPI_Wtime();
  //  Setting up
  int world_size; // total number of processors
  int old_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &old_rank);

  if( argc == 4  || argc == 7 ||  argc == 8 )
    {
      n=atoi(argv[1]);
      m=atoi(argv[2]);
      l=atoi(argv[3]);
    }
  else
    {
      if ( old_rank == 0 )
      printf("Usage:\n mpirun ./parallel_tensor_sum.c n m l\n");
      MPI_Finalize();
      exit(1);
    }

  int ndims = 3;  // for 3D matrices
  int dims[ndims];
  if(argc != 7 && argc != 8)
    {
      dims[0] = 0; // If we do no specify,
      dims[1] = 0; // let the program choose the topology
      dims[2] = 0;
    }
  else // otherwise, take the topology from command line
    {
      dims[0] = atoi(argv[4]);
      dims[1] = atoi(argv[5]);
      dims[2] = atoi(argv[6]);
      if(dims[0] * dims[1] *  dims[2] != world_size) // check!
        {
          if(old_rank == 0)
        printf("Topology not compatible with processors!\n");
          MPI_Finalize();
          exit(1);
        }
    }
  if( argc == 8 )
    method = atoi(argv[7]);
  else
    method = 0;
  //  Set cartesian topology
  MPI_Dims_create(world_size, ndims, dims);
  int periods[ndims];
  periods[0] = 0; periods[1] = 0; periods[2] = 0; // No periodic bc
  MPI_Comm comm;
  MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, 1, &comm);
  int rank;
  MPI_Comm_rank(comm, &rank);
  int coords[ndims];
  MPI_Cart_coords(comm, rank, ndims, coords);

  for(int iteration = 0; iteration < n_iterations ; ++iteration){
     
  int gsizes[3];
  gsizes[0] = n;  /* no. of rows in global array */
  gsizes[1] = m;  /* no. of columns in global array*/
  gsizes[2] = l;  /* no. of columns in global array*/

  int local_array_sizes [ world_size ];
  int starting_point_array [ world_size ];

    method ? first_method( local_array_sizes,  starting_point_array,  dims,  gsizes,   ndims,   world_size ) :
    zero_method( local_array_sizes, starting_point_array, world_size, m*n*l);

  double *local_Matrix_A   = (double *) malloc( local_array_sizes[rank] * sizeof(double) );
  if (!local_Matrix_A)
  {
    fprintf(stderr, "Unable to allocate space\n");
    return MPI_Abort(MPI_COMM_SELF, MPI_ERR_NO_MEM);
  }
  double *local_Matrix_B   = (double *) malloc( local_array_sizes[rank] * sizeof(double) );
  if (!local_Matrix_B)
    {
      fprintf(stderr, "Unable to allocate space\n");
      return MPI_Abort(MPI_COMM_SELF, MPI_ERR_NO_MEM);
    }
  double *local_Matrix_C   = (double *) malloc( local_array_sizes[rank] * sizeof(double) );
  if (!local_Matrix_C)
    {
      fprintf(stderr, "Unable to allocate space\n");
      return MPI_Abort(MPI_COMM_SELF, MPI_ERR_NO_MEM);
    }
  double *global_Matrix_A=NULL;
  double *global_Matrix_B=NULL;
  double *global_Matrix_C=NULL;
  if (rank == 0)
       {
        srand48(SEED) ;
        global_Matrix_A = malloc ( m * n * l * sizeof(double) );
        if (!global_Matrix_A)
        {
         fprintf(stderr, "Unable to allocate space\n");
         return MPI_Abort(MPI_COMM_SELF, MPI_ERR_NO_MEM);
        }
        global_Matrix_B = malloc ( m * n * l * sizeof(double) );
        if (!global_Matrix_B)
        {
         fprintf(stderr, "Unable to allocate space\n");
         return MPI_Abort(MPI_COMM_SELF, MPI_ERR_NO_MEM);
        }
        global_Matrix_C = malloc ( m * n * l * sizeof(double) );
        if (!global_Matrix_C)
        {
         fprintf(stderr, "Unable to allocate space\n");
         return MPI_Abort(MPI_COMM_SELF, MPI_ERR_NO_MEM);
        }
     
       for (int i = 0; i < m * n * l; i++)
           {
               global_Matrix_A[i] = drand48();
               global_Matrix_B[i] = drand48();
           }
           
           scatter_time_i = MPI_Wtime();
           MPI_Scatterv(global_Matrix_A, local_array_sizes, starting_point_array, MPI_DOUBLE, local_Matrix_A, local_array_sizes[rank], MPI_DOUBLE, 0, comm);
           MPI_Scatterv(global_Matrix_B, local_array_sizes, starting_point_array, MPI_DOUBLE, local_Matrix_B, local_array_sizes[rank], MPI_DOUBLE, 0, comm);
           scatter_time_f = MPI_Wtime();
       }
   else {
       scatter_time_i = MPI_Wtime();
       MPI_Scatterv( (void *)0, (void *)0, (void *)0, MPI_DATATYPE_NULL,local_Matrix_A, local_array_sizes[rank], MPI_DOUBLE, 0, comm );
       MPI_Scatterv( (void *)0, (void *)0, (void *)0, MPI_DATATYPE_NULL,local_Matrix_B, local_array_sizes[rank], MPI_DOUBLE, 0, comm );
       scatter_time_f = MPI_Wtime();
       
   }
    /*
    scatter_time_i = MPI_Wtime();
    MPI_Scatterv(global_Matrix_A, local_array_sizes, starting_point_array, MPI_DOUBLE, local_Matrix_A, local_array_sizes[rank], MPI_DOUBLE, 0, comm);
    MPI_Scatterv(global_Matrix_B, local_array_sizes, starting_point_array, MPI_DOUBLE, local_Matrix_B, local_array_sizes[rank], MPI_DOUBLE, 0, comm);
    scatter_time_f = MPI_Wtime();
    */
    cnt = 0;
    for (int k=0; k<local_array_sizes[rank]; k++)
          {
          local_Matrix_C[k]= local_Matrix_A[k] + local_Matrix_B[k];
            ++cnt;
          }
    
    gather_time_i = MPI_Wtime();
    MPI_Gatherv(local_Matrix_C, local_array_sizes[rank], MPI_DOUBLE, global_Matrix_C, local_array_sizes, starting_point_array, MPI_DOUBLE, 0, comm);
    gather_time_f = MPI_Wtime();

    if (rank == 0)
    {
      #ifdef CHECK
      printf("Check!\n");
      for (int i = 0; i < n*m*l; i++)
        if(global_Matrix_C[ i] != (global_Matrix_A[ i ]+global_Matrix_B[ i ]))
           printf("Error!\n");
      #endif
        
    #ifdef DEBUG
        printf("Check!\n");
        for (int i = 0; i < n*m*l; i++)
        {
            printf("%f=%f+%f=%f\n", global_Matrix_C[ i] , global_Matrix_A[ i ],global_Matrix_B[ i ], global_Matrix_A[ i ]+global_Matrix_B[ i ]);
            if(global_Matrix_C[ i] != (global_Matrix_A[ i ]+global_Matrix_B[ i ]))
               printf("Error!\n");
        }
    #endif
      free(global_Matrix_A);
      free(global_Matrix_B);
      free(global_Matrix_C);
    }
  
  free(local_Matrix_A);
  free(local_Matrix_B);
  free(local_Matrix_C);
  }
  end_time = MPI_Wtime();
  if(rank == 0)
    {
      printf("Matrix size: %d x %d x %d \n", n, m, l);
      printf("Topology: (%d,%d,%d)\n", dims[0], dims[1], dims[2] );
    }

  printf ( "# walltime on processor %i : %10.8f \n", rank, (end_time - start_time)/n_iterations ) ;
  printf ( "@) scatter time on processor %i : %10.8f \n", rank, scatter_time_f - scatter_time_i ) ;
  printf ( "@) gather time on processor %i : %10.8f \n", rank, gather_time_f - gather_time_i ) ;
  printf ( "@) communication time on processor %i : %10.8f \n", rank, scatter_time_f - scatter_time_i + gather_time_f - gather_time_i ) ;
  printf ( "*) Processor %d has %d data\n",rank,cnt);
  MPI_Finalize();
  return 0;
}


// mpicc sum3Dmatrix.c -DCHECK -o sum3Dmatrix.x
// mpicc sum3Dmatrix.c -DDEBUG -o sum3Dmatrix.x
// mpirun -np 18 ./sum3Dmatrix.x 101 11 11 3 3 2 1 | sort
// mpirun -np 18 ./sum3Dmatrix.x 101 11 11 3 3 2 0 | sort

