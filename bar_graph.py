import matplotlib.pyplot as plt

# Read data from the .out file
file_path = 'week10/data.out'
num_ranks, execution_time = [], []

with open(file_path, 'r') as file:
    for line in file:
        data = line.split()
        num_ranks.append(int(data[0]))
        execution_time.append(float(data[1]))

# Plotting the bar chart
plt.bar(num_ranks, execution_time, color='blue')
plt.xlabel('Number of Ranks')
plt.ylabel('Execution Time (s)')
plt.title('Execution Time of particular ranks')
plt.show()
