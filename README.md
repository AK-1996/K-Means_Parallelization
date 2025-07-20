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
