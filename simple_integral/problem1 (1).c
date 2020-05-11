#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

//Length of section
#define A 2.0

//Initialize function to integrate
double f(double x){
    return sqrt(4 - x * x);
}

int main(int argc, char *argv[]){

    if (argc < 2){
        printf("Usage: %s num_intervals.\n", argv[0]);
        return -1;
    }
     
    int size, rank, N;
    double sumI = 0;
    N = atoi(argv[1]);

    //Step in section
    double h = A / N;
        
    MPI_Init(&argc, &argv); //MPI initialization
    MPI_Comm_size(MPI_COMM_WORLD, &size); //Number of threads in communicator
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //Num of current thread, first thread - 0)

    //Uncomment if you want see real time.
    //double t = MPI_Wtime();

    //Memory allocation for array of x's and sum on sections
    double *x, *Sum;
    x = (double *) calloc(N, sizeof(double));
    for (int i = 0; i < N; i++){
        x[i] = 0;
    }
    Sum = (double *) calloc(N, sizeof(double));
    for (int i = 0; i < N - 1; i++){
        Sum[i] = 0;
    }

    //Main iterations which count integral.
    for (int i=rank; i < N; i += size){
        x[i] = i*h;
        x[i+1] = (i+1)*h;
        Sum[rank] = Sum[rank] + h * ( f(x[i]) + f(x[i+1]) ) / 2;
    }
    if (rank == 0){
        sumI = Sum[rank];
        for (int i = 1; i < size; i++) {
            MPI_Recv(&Sum[i], sizeof(double), MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sumI += Sum[i];
        }  
        printf("%f\n", sumI);

        //Uncomment if you want see real time.
        //t = MPI_Wtime() - t;
        //printf("Time: %f seconds \n", t);
    }
    else{
        MPI_Send(&Sum[rank], sizeof(double), MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    free(x);
    free(Sum);

    MPI_Finalize(); //End of work with MPI
    return 0;
}
