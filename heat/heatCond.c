#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define Length 1.0
#define Temperature_1 1.0
#define Temperature_2 5.0

int main(int argc, char **argv)
{
    // Preferable input values of Time and
    // number of divisions
	double Time = 0.000001;
	int num_divisions = 60000;

    // Checking if you have all needed input parameters
    if (argc < 3)
	{
        printf("No expected time and step\nUsage: %s TIME Number_of_divisions \n", argv[0]);
		exit(1);
	}

    // Setting up size and rank 
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Setting up input parameters
    Time = atof(argv[1]);
	if (Time < 0 && rank == 0) { // Time can't be less than zero
		printf("Time can't be less than zero");
		return -1;
	}
	num_divisions = atoi(argv[2]);
	if (num_divisions < 2 && rank == 0) { // Method won't converge
	    printf("Invalid values!\n");
	    return -1;
    }
	if(num_divisions <= size && rank == 0) { // Too many processes for this number of divisions
	    printf("Too many processes for this divisions\n");
	    return -1;
	}

    // Coordinate step
	double coord_step = Length / num_divisions;
    // Kurrant number (time  step)
	double time_step = 0.3 * coord_step * coord_step;
    // Number of divisions by time
	int num_time_division = Time / time_step;

    // Temperature arrays for time moments n and n + 1, respectively
	double *u0 = (double*) malloc(sizeof(double) * num_divisions);
	double *u1 = (double*) malloc(sizeof(double) * num_divisions);

    // Counters for time and coordinate 
	int time_counter, coord_counter;

	// Starting time counter
	double time = MPI_Wtime();
	
	// Initial Conditions
    // Set everything to 0.0
	for (coord_counter = 0; coord_counter < num_divisions; coord_counter++) 
    {
		u0[coord_counter] = u1[coord_counter] = 0.0;
	}

	// Border Conditions
    // Here we use Temperatures that we have defined
    // in the beginning of our program
	u0[0] = u1[0] = Temperature_1;
	u0[num_divisions - 1] = u1[num_divisions - 1] = Temperature_2;
	
	// Array of transferred points indexes
	int *left_index = (int*) malloc(sizeof(int) * size + 1);
	left_index[0] = 1;
	// Here we define the right end of exit array
	left_index[size] = num_divisions - 1;
	// Iteratively define lefts ends of line segments
    // which come to differents procesess.
    // Right end of i line segment stands for left end  of i+1 
	for(int i = 1; i < size; i++) {
		left_index[i] = left_index[i - 1] + (num_divisions / size) + ((i - 1) < ((num_divisions % size) - 2));
	}

	// Time cycle
	for (time_counter = 0; time_counter < num_time_division; time_counter++) {
		// Exchange of border points
		if(rank != 0) {
			MPI_Send(u0 + left_index[rank], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
			MPI_Recv(u0 + left_index[rank] - 1, 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		if(rank != size - 1) {
			MPI_Send(u0 + left_index[rank + 1] - 1, 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
			MPI_Recv(u0 + left_index[rank + 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		// Even method
		for (coord_counter = left_index[rank]; coord_counter < left_index[rank + 1]; coord_counter++) {
			u1[coord_counter] = u0[coord_counter] + 0.3  * (u0[coord_counter - 1] - 2.0 * u0[coord_counter] + u0[coord_counter + 1]);
		}
		// Updating results
		double *tmp = u0;
		u0 = u1;
		u1 = tmp;
	}

	// Making reduction for data collected from the last iteration in cycle
	if(size > 1) {
		if(rank == 0) {
			for(int i = 1; i < size; i++) {
				MPI_Recv(u1 + left_index[i], left_index[i + 1] - left_index[i], MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		} else {
			MPI_Send(u1 + left_index[rank], left_index[rank + 1] - left_index[rank], MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
		}
	}

	// Evaluation of time
	time = MPI_Wtime() - time;
	
	// Printing results on the screen
	if(rank == 0) {
		printf("%d %lf\n", size, time);
	}
	
    // Freeing memory (malloc())
	free(u0);
	free(u1);

    // Stop working with MPI
    MPI_Finalize();
	return 0;
}
