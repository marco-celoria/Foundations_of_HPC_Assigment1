## Setup: ring.c
To compile the code on ORFEO (or on other machines with the required modules installed):
```
$ module load openmpi-4.1.1+gnu-9.3.0
$ mpicc ring.c -o ring.x
```
To run run the code:
```
$ mpirun -np N ./ring.x
```
or
```
$ mpirun -np N ./ring.x n_iterations
```
In the second case, the code performs n_iterations and measures the average time for one cycle. 
By default n_iterations = 1. 
If the user accidentally passes to the program `n_iterations < 1`, by default n_iterations = 1 is automatically set.
For example, on login node by entering:
```
$ mpirun -np 4 ./ring.x 10 
```
we get
```
I am process 0 and I have received 8 messages. My final messages have tags 0, 0; values -6, 6
# walltime on processor 0 after 10 iterations 0.00018397. Average: 0.00001840 
I am process 1 and I have received 8 messages. My final messages have tags 10, 10; values -6, 6
# walltime on processor 1 after 10 iterations 0.00017372. Average: 0.00001737 
I am process 2 and I have received 8 messages. My final messages have tags 20, 20; values -6, 6
# walltime on processor 2 after 10 iterations 0.00021022. Average: 0.00002102 
I am process 3 and I have received 8 messages. My final messages have tags 30, 30; values -6, 6
# walltime on processor 3 after 10 iterations 0.00020380. Average: 0.00002038 
```
while for 
```
$ mpirun -np 4 ./ring.x -1
```
we get
```
I am process 0 and i have received 8 messages. My final messages have tags 0, 0; values -6, 6
# walltime on processor 0 after 1 iterations 0.00006643. Average: 0.00006643 
I am process 1 and i have received 8 messages. My final messages have tags 10, 10; values -6, 6
# walltime on processor 1 after 1 iterations 0.00011525. Average: 0.00011525 
I am process 2 and i have received 8 messages. My final messages have tags 20, 20; values -6, 6
# walltime on processor 2 after 1 iterations 0.00018692. Average: 0.00018692 
I am process 3 and i have received 8 messages. My final messages have tags 30, 30; values -6, 6
# walltime on processor 3 after 1 iterations 0.00010550. Average: 0.00010550 
```
Note that it is possible to compile also with Intel MPI libraries with
```
$ module load intel/20.4
$ mpicc -std=c99 ring.c -o ring.x
```
## Setup: sum3Dmatrix.c
To compile the code on ORFEO (or on other machines with the required modules installed):
```
$ module load openmpi-4.1.1+gnu-9.3.0
$ mpicc sum3Dmatrix.c -o sum3Dmatrix.x
```
If the user wants to check that the matrix sum is correct (note it slows down the code):
```
$ module load openmpi-4.1.1+gnu-9.3.0
$ mpicc sum3Dmatrix.c -DCHECK -o sum3Dmatrix.x
```
If the user wants to check the result and print on standard output the final matrix elements (it is not recommended for large matrix sizes and it slows down the code), it is possible to compile with DEBUG option: 
```
$ module load openmpi-4.1.1+gnu-9.3.0
$ mpicc sum3Dmatrix.c -DDEBUG -o sum3Dmatrix.x
```
There are several ways to run the code, the simplest way on P processors (letting the program to choose the virtual topology) is
```
$ mpirun -np P ./sum3Dmatrix.x  Nx Ny Nz
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 
```
The output is of the form (`$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 | sort`)
```
@) communication time on processor 0 : 0.00010489 
@) communication time on processor 1 : 0.00004653 
@) communication time on processor 2 : 0.00005831 
@) communication time on processor 3 : 0.00008170 
@) gather time on processor 0 : 0.00005795 
@) gather time on processor 1 : 0.00001535 
@) gather time on processor 2 : 0.00001970 
@) gather time on processor 3 : 0.00001775 
Matrix size: 5 x 5 x 1 
*) Processor 0 has 7 data
*) Processor 1 has 6 data
*) Processor 2 has 6 data
*) Processor 3 has 6 data
@) scatter time on processor 0 : 0.00004694 
@) scatter time on processor 1 : 0.00003118 
@) scatter time on processor 2 : 0.00003861 
@) scatter time on processor 3 : 0.00006395 
Topology: (2,2,1)
# walltime on processor 0 : 0.00027460 
# walltime on processor 1 : 0.00059829 
# walltime on processor 2 : 0.00041496 
# walltime on processor 3 : 0.00057428 
```
The code implements two different algoriths to distribute the matrix elements among the processors. By default (without specifying the dedicated option by command line) it uses the most balanced and democratic one.
The user can specify the virtual topology by means of 
```
$ mpirun -np P ./sum3Dmatrix.x  Nx Ny Nz px py pz
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 4 1 1
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 2 2 1
```
With `N = px * py * pz`, otherwise the program exits throwing an error.
Finally, to specify the algorithm, run the following:
```
$ mpirun -np P ./sum3Dmatrix.x  Nx Ny Nz px py pz a
```
With `a = 0` corresponding to the democratic / balanced algorithm, 
```
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 2 2 1 0
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 4 1 1 0
```
while `a != 0` corresponding to the virtual topology dependent algorithm. For example, running  
```
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 2 2 1 1 | sort
```
the  result is 
```
@) communication time on processor 0 : 0.00012748 
@) communication time on processor 1 : 0.00004581 
@) communication time on processor 2 : 0.00005116 
@) communication time on processor 3 : 0.00008371 
@) gather time on processor 0 : 0.00007289 
@) gather time on processor 1 : 0.00001593 
@) gather time on processor 2 : 0.00000445 
@) gather time on processor 3 : 0.00001578 
Matrix size: 5 x 5 x 1 
*) Processor 0 has 9 data
*) Processor 1 has 6 data
*) Processor 2 has 6 data
*) Processor 3 has 4 data
@) scatter time on processor 0 : 0.00005459 
@) scatter time on processor 1 : 0.00002988 
@) scatter time on processor 2 : 0.00004671 
@) scatter time on processor 3 : 0.00006793 
Topology: (2,2,1)
# walltime on processor 0 : 0.00032111 
# walltime on processor 1 : 0.00055110 
# walltime on processor 2 : 0.00073855 
# walltime on processor 3 : 0.00076919 
```
while running
```
$ mpirun -np 4 ./sum3Dmatrix.x  5 5 1 4 1 1 1 | sort
```
we have
```
@) communication time on processor 0 : 0.00011512 
@) communication time on processor 1 : 0.00004585 
@) communication time on processor 2 : 0.00005387 
@) communication time on processor 3 : 0.00007247 
@) gather time on processor 0 : 0.00006716 
@) gather time on processor 1 : 0.00000966 
@) gather time on processor 2 : 0.00001417 
@) gather time on processor 3 : 0.00000655 
Matrix size: 5 x 5 x 1 
*) Processor 0 has 10 data
*) Processor 1 has 5 data
*) Processor 2 has 5 data
*) Processor 3 has 5 data
@) scatter time on processor 0 : 0.00004796 
@) scatter time on processor 1 : 0.00003618 
@) scatter time on processor 2 : 0.00003970 
@) scatter time on processor 3 : 0.00006592 
Topology: (4,1,1)
# walltime on processor 0 : 0.00030856 
# walltime on processor 1 : 0.00065954 
# walltime on processor 2 : 0.00073101 
# walltime on processor 3 : 0.00077050 
```
Note that it is possible to compile also with Intel MPI libraries with
```
$ module load intel/20.4
```
together with one of the following
```
$ mpicc -std=gnu99 sum3Dmatrix.c -o sum3Dmatrix.x 
$ mpicc -std=gnu99 sum3Dmatrix.c -DCHECK -o sum3Dmatrix.x
$ mpicc -std=gnu99 sum3Dmatrix.c -DDEBUG -o sum3Dmatrix.x
```
