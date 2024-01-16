import os
import matplotlib.pyplot as plt

# Function to parse the data from a file
def parse_file(file_path):
    with open(file_path, 'r') as file:
        data = file.readlines()

    times = []
    sizes = []
    
    for line in data:
        if "sharedPointerCollective" in line:
            _, time, size, array_size = line.split("::")
            times.append(float(time))
            sizes.append(int(array_size))
    
    return sizes, times

# Function to plot the data
def plot_graph(files):
    plt.figure(figsize=(10, 6))
    plt.title('Benchmarking Graph')
    plt.xlabel('Array Size')
    plt.ylabel('Execution Time (seconds)')

    for file_path in files:
        sizes, times = parse_file(file_path)
        label = os.path.basename(file_path).split('.')[0]
        plt.plot(sizes, times, label=label)

    plt.legend()
    plt.show()

# Get a list of "*.out" files in the specified folder
folder_path = './benchmarks/1705336057'
files = [os.path.join(folder_path, file) for file in os.listdir(folder_path) if file.endswith('.out')]

# Create separate graphs for different sizes
for size in set([int(os.path.basename(file).split('_')[1]) for file in files]):
    size_files = [file for file in files if f'_{size}_' in file]
    plot_graph(size_files)
