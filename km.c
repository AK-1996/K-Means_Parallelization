#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>


struct Point {
    int cluster;
    double *features;
};

void point_initialization(int num_points, int num_features, struct Point **points) {
    *points = (struct Point *) malloc(num_points * sizeof(struct Point));
    if (*points == NULL)
	{
		printf("Error: Memory allocation failed");
	}

    for (int i = 0; i < num_points; i++) {
        (*points)[i].features = (double *) malloc(num_features * sizeof(double));
        if ((*points)[i].features == NULL) {
            printf("Error: Memory allocation for features failed");
            exit(1);
        }
    }
}

void centroid_initialization(int num_clusters, int num_features, struct Point **centroids) {
    *centroids = (struct Point *) malloc(num_clusters * sizeof(struct Point));
    if (*centroids == NULL)
	{
		printf("Error: Memory allocation failed");
	}

    for (int i = 0; i < num_clusters; i++) {
        (*centroids)[i].features = (double *) malloc(num_features * sizeof(double));
        if ((*centroids)[i].features == NULL) {
            printf("Error: Memory allocation for features failed");
            exit(1);
        }
    }
}

// Similarity function
double euclidean_distance(struct Point a, struct Point b, int num_features) {
    int i;
    double distance = 0;
    for (i = 0; i < num_features; i++) {
        distance += (a.features[i] - b.features[i]) * (a.features[i] - b.features[i]);
    }
    return sqrt(distance);
}

void km(struct Point *points, struct Point *centroids, int num_points, int num_features, int num_clusters, int iterations) {
    int i, j;
    int n = 0;
    while (n < iterations) {
    	for (j = 0; j < num_clusters; j++) {
            int total[num_features];
            int num_assigned = 0;
            for (i = 0; i < num_features; i++) total[i] = 0;

            for (i = 0; i < num_points; i++) {
                if (points[i].cluster == j) {
                    for (int k = 0; k < num_features; k++) {
                        total[k] += points[i].features[k];
                    }
                    num_assigned++;
                }
            }
            
            // Update centroid position of each cluster according to the current point positions
            if (num_assigned > 0) {
                for (int k = 0; k < num_features; k++) {
                    centroids[j].features[k] = (double)total[k] / num_assigned;
                }
            }
        }
        
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


void read_data(char *path, int num_clusters, int *num_points, int *num_features, struct Point **points) {
    FILE *file = fopen(path, "r");
    fscanf(file, "%d\t%d", num_points, num_features);

    // Point initializations
    point_initialization(*num_points, *num_features, points);
    for (int i = 0; i < *num_points; i++) {
        (*points)[i].cluster = i % num_clusters;
        for (int j = 0; j < *num_features; j++) {
            fscanf(file, "%lf", &(*points)[i].features[j]);
        }
    }
    fclose(file);
}

void output(struct Point *points, int num_points, double elapsed){
	FILE *file = fopen("serial_results.txt", "w");
    if(file == NULL)
	{
		fprintf(stderr,"can't open input file \"serial_results.txt\"\n");
		exit(1);
	}

    for (int i = 0; i < num_points; i++) {
        fprintf(file, "Point %d is in Cluster %d\n", i, points[i].cluster);
    }
    fclose(file);

    FILE *fp = fopen("time.txt", "a");
    if(fp == NULL)
	{
		fprintf(stderr,"can't open input file \"time.txt\"\n");
		exit(1);
	}

    fprintf(fp, "Serial time:\t%f\n", elapsed);
    fclose(fp);
}

void free_point_memory(struct Point *points, int num_points){
	for (int i = 0; i < num_points; i++) free(points[i].features);
    free(points);
}

void free_centroid_memory(struct Point *centroids, int num_clusters){
	for (int i = 0; i < num_clusters; i++) free(centroids[i].features);
    free(centroids);
}


int main(int argc, char *argv[]) {
    // Start measuring time
    struct timeval begin, end;
    gettimeofday(&begin, 0);

    int num_points, num_features, num_clusters, iterations;
    struct Point *points, *centroids;
    
    num_clusters = atoi(argv[1]);
    iterations = atoi(argv[2]);
    char *path = argv[3];

    read_data(path, num_clusters, &num_points, &num_features, &points);
    centroid_initialization(num_clusters, num_features, &centroids);
    km(points, centroids, num_points, num_features, num_clusters, iterations);

    // Stop measuring time and calculate the elapsed time
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

    output(points, num_points, elapsed);

	free_point_memory(points, num_points);
    free_centroid_memory(centroids, num_clusters);

    return 0;
}
