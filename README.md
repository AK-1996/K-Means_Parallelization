# K-Means Clustering Parallelization

## Project Overview
This repository contains both a serial and an MPI-based parallel implementation of the K-Means clustering algorithm in C, along with Python utility scripts for dataset generation and result verification. The goal is to explore different parallelization strategies (observation- vs. cluster-level) and to evaluate performance (strong/weak scaling, intra- vs. inter-regional clusters) on Google Cloud Platform.

## Features
- **Serial implementation** of K-Means with Euclidean distance.
- **MPI parallelization** via observation distribution:
  - Partial centroid updates with `MPI_Reduce` and broadcast.
  - Local assignment updates per process.
- **Utility scripts**:
  - `dataset_generator.py`: create synthetic datasets of configurable size/features.
  - `check.py`: compare serial vs. parallel outputs for correctness.
- **Performance benchmarks**:
  - Strong and weak scaling on “light” (8×2-vcore) and “fat” (2×8-vcore) clusters.
  - Intra- and inter-regional latency effects.

## Requirements
- C compiler supporting MPI (e.g. `mpicc`)
- MPI runtime (e.g. OpenMPI or MPICH)
- Python 3 with:  
  - `numpy` (for dataset generation)  
  - Standard library only for `check.py`

## Building
```bash
# Serial version
gcc -O3 -std=c11 -o kmeans_serial kmeans_serial.c

# Parallel MPI version
mpicc -O3 -std=c11 -o kmeans_mpi kmeans_mpi.c
```

## Usage
### Serial
`./kmeans_serial <dataset.csv> <num_clusters> <num_iterations>`

### Parallel (P processes)
`mpirun -np P ./kmeans_mpi <dataset.csv> <num_clusters> <num_iterations>`

- dataset.csv: first line “<n_points> <n_features>”, followed by one point per line.
- num_clusters (k): desired number of clusters.
- num_iterations: maximum iterations before convergence.

## Python Utilities
# Generate a dataset of 500 000 points with 2 features
`python3 dataset_generator.py --n_points 500000 --n_features 2 --output data.csv`

# Verify serial vs. parallel outputs
`python3 check.py --serial out_serial.txt --parallel out_mpi.txt`

## Performance Summary & Advice
- Observation distribution gave near-ideal speedup (≈P) with minimal serial fraction.
- Cluster-level parallelization plateaued quickly when k < #processes.
- Intra-regional clusters outperform inter-regional—minimize cross-region traffic.
- Advice: Aim for node counts ≤10× your cluster count to balance compute vs. communication overhead.

Next Steps
- Experiment with hybrid MPI+OpenMP to reduce latency.
- Implement centroid initialization strategies (k-means++) for faster convergence.
- Add checkpointing to resume long runs.

## Author
Matteo Poire` (Master’s in Data Science, 2025)
