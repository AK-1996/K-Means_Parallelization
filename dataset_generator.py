import sys
import random

def generate_dataset(num_points, num_features, path):
    with open(path, "w") as f:
        f.write(f"{num_points}\t{num_features}\n")
        for i in range(num_points):
            features = [str(random.uniform(0, 10000)) for j in range(num_features)]
            f.write("\t".join(features) + "\n")

if len(sys.argv) != 4:
	print("Usage: " + sys.argv[0] + " NUM_POINTS NUM_FEATURES PATH");
else:
	generate_dataset(int(sys.argv[1]), int(sys.argv[2]), sys.argv[3])