import os
import sys
import random
import collections

# Test variables
n_cluster = 10
n_points = 50000
n_iter = 100
n_features = 8
n_process = list(range(2))
dataset = "../dataset/set.txt"
hostfile = "../scripts/hostfile"

# Dataset generation
def generate_dataset(n_points, n_features):
    if not (os.path.isfile(dataset)):
        with open(dataset, "w") as f:
            f.write(f"{n_points}\t{n_features}\n")
            for i in range(n_points):
                features = [str(random.uniform(0, 10000)) for j in range(n_features)]
                f.write("\t".join(features) + "\n")

def transfer_data():
    address = []
    i = 0
    with open(hostfile, "r") as f:
        for line in f:
            if i != 0:
                address.append(line.strip().split(sep=" ")[0])
                i += 1
    
    for add in address:
        os.system("scp -q ../dataset/set.txt matteo@" + add + ":~/dataset")

# C programs compiling
def compile():
    os.system("gcc ../scripts/km.c -o ../scripts/km -lm")
    os.system("mpicc ../scripts/mpi_km.c -o ../scripts/mpi_km -lm")

# Program executions
def execution(n_process):
    if not (os.path.isfile("./serial_results.txt") and os.path.isfile("./parallel_results.txt")):
        os.system(f"../scripts/km {n_cluster} {n_iter} {dataset}")
        os.system(f"mpirun --hostfile {hostfile} -np {n_process} ../scripts/mpi_km {n_cluster} {n_iter} {dataset}")

    print("\nRESULTS COMPUTED.\n")
    
# Removing temporary files
def delete():
    os.system("rm ../dataset/set.txt ./serial_results.txt ./parallel_results.txt")

# Gathering points to clusters they belong to
def gather(file):
    #Create a dictionary like 'key:list'
    clusters = collections.defaultdict(list)
    with open(file) as f:
        for line in f:
            point, cluster = line.strip().split(" is in Cluster ")
            clusters[cluster].append(point)
    return clusters

# Cluster dictionary comparisons
def compare(cluster1, cluster2):
    i = 0
    for k1, v1 in cluster1.items():
        for k2, v2 in cluster2.items():
            if set(v1) & set(v2):
                i+=1
                break
    if i == n_cluster:
        print("Check confirmed")
    else:
        print("Check error")
        exit(1)

#Compute speedup
def speedup(ser_time, par_time):
    return ser_time/par_time

#Compute scalability
def scalability(single_core, par_time):
    return single_core/par_time


compile()
#Initialize variable to compute performances
single_core = 0
ser_time = []
par_time = []
#Check if the analysis starts from scratch
if n_process[0] != 0:
    #If not, retrieve execution time of parallel algorithm
    with open("./performance.txt") as f:
        f.readline()
        single_core = float(f.readline().strip().split("\t")[4])
else:
    with open("./performance.txt", "a") as f:
        f.write(f"N_proc\tSpeedup\tScalability\tAvg_Ser_Time\tAvg_Par_Time\n")

#Start performance analysis
for n_proc in n_process:
    print(f"--- N. PROCESSES IN EXECUTION: {n_proc+1} ---")
    print(f"--- N. OBSERVATION: {n_points} ---\n")
    ser_time.clear()
    par_time.clear()

    for n in range(5):
        generate_dataset(n_points, n_features)
        # transfer_data()
        execution(n_proc+1)
        compare(gather("./serial_results.txt"), gather("./parallel_results.txt"))
        delete()

    
    with open("./time.txt", "r") as f:
        i = 0
        for line in f:
            if (i % 2) == 0:
                s_time = float(line.strip().split("\t")[1])
                ser_time.append(s_time)
                print(f"serial time: {s_time}\n")
            else:
                p_time = float(line.strip().split("\t")[1])
                par_time.append(p_time)
                print(f"parallel time: {p_time}\n\n")
            i += 1
    
    os.system("rm ./time.txt")

    avg_ser_time = sum(ser_time)/5
    avg_par_time = sum(par_time)/5
    if n_proc == 0:
        single_core = avg_par_time

    with open("./performance.txt", "a") as f:
        f.write(f"{n_proc+1}\t{speedup(avg_ser_time, avg_par_time)}\t{scalability(single_core, avg_par_time)}\t{avg_ser_time}\t{avg_par_time}\n")