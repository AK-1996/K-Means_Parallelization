#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define MAX_FEATURES 8

struct Point {
    int cluster;
    double features[MAX_FEATURES];
};

// Similarity function
double euclidean_distance(struct Point a, struct Point b, int num_features) {
    int i;
    double distance = 0;
    for (i = 0; i < num_features; i++) {
        distance += (a.features[i] - b.features[i]) * (a.features[i] - b.features[i]);
    }
    return sqrt(distance);
}

void km(struct Point *points, struct Point *centroids, int num_points, int num_features, int num_clusters, int iterations, int rank, MPI_Datatype MPI_POINT) {
    int i, j;
    int n = 0;
    while (n < iterations) {
    	for (j = 0; j < num_clusters; j++) {
            int total[num_features], num_assigned = 0;

            int local_total[num_features], local_num_assigned= 0;
            for (i = 0; i < num_features; i++) {
                local_total[i] = 0;
                total[i] = 0;
            } 

            for (i = 0; i < num_points; i++) {
                if (points[i].cluster == j) {
                    for (int k = 0; k < num_features; k++) {
                        local_total[k] += points[i].features[k];
                    }
                    local_num_assigned++;
                }
            }

            // Partial results computed by each process are summed up and sent to the root process
            MPI_Reduce(&local_num_assigned, &num_assigned, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(local_total, total, num_features, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        	
            // Root process update centroid position of each cluster according to current point positions
	        if (rank == 0 && num_assigned > 0) {
	            for (int k = 0; k < num_features; k++) {
	                centroids[j].features[k] = (double)total[k] / num_assigned;
	            }
	        }
        }
        
        // Broadcasting of centroid information
        MPI_Bcast(centroids, num_clusters, MPI_POINT, 0, MPI_COMM_WORLD);
        
        // Update cluster assignment for each point
        for (i = 0; i < num_points; i++) {
            double min_distance = INFINITY;
            for (j = 0; j < num_clusters; j++) {
                double distance = euclidean_distance(points[i], centroids[j], num_features);
                if (distance < min_distance) {
                    min_distance = distance;
                    points[i].cluster = j;
                }
            }
        }

        n++;
    }
}

void read_data(char *filename, int num_clusters, int *num_points, int *num_features, int *process_num_points, int *remainder, struct Point **process_points, int rank, int size) {
    int i = 0;

    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

    fscanf(fp, "%d %d", num_points, num_features);
    
    // Compute the number of lines per process
	*process_num_points = (*num_points) / size;
	*remainder = (*num_points) % size;
	int start_line;

	if (rank < (*remainder))
	{
		(*process_num_points)++;
		start_line = rank * (*process_num_points);
		
	}else
	{
		start_line = rank * (*process_num_points) + (*remainder);
	}

    // Initialize vector of partial points
    *process_points = (struct Point *)malloc((*process_num_points) * sizeof(struct Point));
    if (*process_points == NULL)
	{
		printf("Error: Memory allocation failed");
	}

    if (rank != 0)
    {
        char buffer[1024];
        for (i = 0; i < start_line+1; i++)
        {
            fgets(buffer, sizeof(buffer), fp);
        }
    }

    for (i = 0; i < (*process_num_points); i++)
    {
        (*process_points)[i].cluster = i % num_clusters;

        for (int j = 0; j < (*num_features); j++)
        {
            fscanf(fp, "%lf", &(*process_points)[i].features[j]);
        }
    }
    
    fclose(fp);
}

void read_data_serial(char *path, int num_clusters, int *num_points, int *num_features, struct Point **points) {
    FILE *file = fopen(path, "r");
    fscanf(file, "%d\t%d", num_points, num_features);

    // Point initializations
    *points = (struct Point *) malloc((*num_points) * sizeof(struct Point));
    if (*points == NULL)
	{
		printf("Error: Memory allocation failed");
	}

    for (int i = 0; i < *num_points; i++) {
        (*points)[i].cluster = i % num_clusters;
        for (int j = 0; j < *num_features; j++) {
            fscanf(file, "%lf", &(*points)[i].features[j]);
        }
    }
    fclose(file);
}

void output(struct Point *points, int num_points, double elapsed){
	FILE *fp = fopen("parallel_results.txt", "w");
    if(fp == NULL)
	{
		fprintf(stderr,"can't open input file \"result_MPI.txt\"\n");
		exit(1);
	}

    for (int i = 0; i < num_points; i++) {
        fprintf(fp, "Point %d is in Cluster %d\n", i, points[i].cluster);
    }
    fclose(fp);

    FILE *fp_time = fopen("time.txt", "a");
    if(fp_time == NULL)
	{
		fprintf(stderr,"can't open input file \"time.txt\"\n");
		exit(1);
	}

    fprintf(fp_time, "Parallel time:\t%f\n", elapsed);
    fclose(fp_time);
}

int main(int argc, char *argv[]) {

    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Start measuring time
    struct timeval begin, end;
    gettimeofday(&begin, 0);

    int num_points, num_features, num_clusters, iterations;
    struct Point *points, *centroids;
    
    num_clusters = atoi(argv[1]);
    iterations = atoi(argv[2]);
    char *path = argv[3];

    // Process individual vector of partial points
    struct Point *process_points;
    int process_num_points;
    int remainder;
	read_data(path, num_clusters, &num_points, &num_features, &process_num_points, &remainder, &process_points, rank, size);

    // Observation distribution alternative
    // if (rank == 0)
    // {
    //     read_data_serial(path, num_clusters, &num_points, &num_features, &points);
    // }
    // MPI_Bcast(&num_points, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// MPI_Bcast(&num_features, 1, MPI_INT, 0, MPI_COMM_WORLD);
    

    points = (struct Point *) malloc(num_points * sizeof(struct Point));
    if (points == NULL)
	{
		printf("Error: Memory allocation failed");
	}

    centroids = (struct Point *) malloc(num_clusters * sizeof(struct Point));
    if (centroids == NULL)
	{
		printf("Error: Memory allocation failed");
	}

	// MPI datatype definition for Point struct
	MPI_Datatype MPI_POINT;
  	MPI_Datatype T[2] = {MPI_INT, MPI_DOUBLE};
  	int B[2] = {1, MAX_FEATURES};
  	MPI_Aint D[2] = {
        offsetof(struct Point, cluster),
        offsetof(struct Point, features)
    };
  	MPI_Type_create_struct(2, B, D, T, &MPI_POINT);
  	MPI_Type_commit(&MPI_POINT);

    // Uncomment if read_data_serial is used
    // process_num_points = (num_points) / size;
	// remainder = (num_points) % size;

	// if (rank < (remainder))
	// {
	// 	(process_num_points)++;
	// }

    // process_points = (struct Point *)malloc((process_num_points) * sizeof(struct Point));
    // if (process_points == NULL)
	// {
	// 	printf("Error: Memory allocation failed");
	// }

    // Scatterv and Gatherv arguments
    int *displ, *scounts;

	displ = (int *)malloc(size * sizeof(int));
	if (displ == NULL)
	{
		printf("Error: Memory allocation failed");
	}

	scounts = (int *)malloc(size * sizeof(int));
	if (scounts == NULL)
	{
		printf("Error: Memory allocation failed");
	}

	for (int i = 0; i < size; ++i) 
	{
		scounts[i] = process_num_points;
		if (i < remainder)
		{
			scounts[i]++;
		}
		
		if (i > 0)
		{
			displ[i] = displ[i-1] + scounts[i-1];
		}else
		{
			displ[i] = 0;
		}		
	}

    // Uncomment if read_data_serial is used
    // MPI_Scatterv(points, scounts, displ, MPI_POINT, process_points, process_num_points, MPI_POINT, 0, MPI_COMM_WORLD);

    km(process_points, centroids, process_num_points, num_features, num_clusters, iterations, rank, MPI_POINT);

    MPI_Gatherv(process_points, process_num_points, MPI_POINT, points, scounts, displ, MPI_POINT, 0, MPI_COMM_WORLD);
    
    if(rank == 0){
        // Stop measuring time and calculate the elapsed time
        gettimeofday(&end, 0);
        long seconds = end.tv_sec - begin.tv_sec;
        long microseconds = end.tv_usec - begin.tv_usec;
        double elapsed = seconds + microseconds*1e-6;
    	output(points, num_points, elapsed);
    	free(points);
    }
    free(centroids);
    free(process_points);
    free(scounts);
    free(displ);

    MPI_Type_free(&MPI_POINT);
	MPI_Finalize();

    return 0;
}