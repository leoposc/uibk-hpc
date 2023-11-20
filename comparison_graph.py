import re
import os
import pandas as pd
import matplotlib.pyplot as plt

def parse_numbers_from_file(folder_list):
    # Initialize an empty dictionary to store the parsed data
    
    # Regular expression pattern to match variables and numbers
    
    pattern = r'\$!(\w+)\{([\d\.]+), ([\d\.]+)\}'
    pattern2 = r'\$!(\w+)\{([\d\.]+), ([\d\.]+), ([\d\.]+)\}'
    
    data_collection = None
    for filename in os.listdir(folder_list[0]):
      current_pattern = None
      with open(os.path.join(folder_list[0], filename), 'r') as file:
              content = file.read()
              matches = re.findall(pattern, content)
              if len(matches) == 0:
                # 3 columns MPI
                current_pattern = pattern2
                data_collection = pd.DataFrame(columns=['name', 'ranks', 'size', 'runtime'])
              else:
                # 2 columns MPI rank variation
                current_pattern = pattern
                data_collection = pd.DataFrame(columns=['name', 'ranks', 'runtime'])
                # 2 columns single rank
                # current_pattern = pattern
                # data_collection = pd.DataFrame(columns=['name', 'size', 'seq runtime'])
                

    # Iterate over all files in the directory
    for folder in folder_list:
      for filename in os.listdir(folder):
          file_path = os.path.join(folder, filename)

          # Open the file for reading
          with open(file_path, 'r') as file:
              content = file.read()

              # Use re.findall to find all matches in the content
              matches = re.findall(current_pattern, content) 
            
              numbers = [folder.split('/')[-1]]
              for match in matches:
                for i, num in enumerate(match):
                  if i == 0:
                    continue
                  numbers.append(float(num))
              
              data_collection.loc[len(data_collection.index)] = numbers

    return data_collection.sort_values(by=['ranks'])
            
def plot_comparison(df, labels):
    names = pd.unique(df['name'])
    for i in range(len(names)):
      plt.plot(df[df['name']==names[i]]['ranks'], df[df['name']==names[i]]['efficiency'], marker="o", label=labels[i])
    plt.xlabel('Ranks')
    plt.ylabel('Efficiency')
    plt.title("Efficiency comparison for fixed problem size (4992)")
    plt.grid(True)
    plt.legend()
    plt.show()

# Example usage:

list_of_dirs = ['week07/ex01/benchmarks/aos_diff_ranks',
                'week07/ex01/benchmarks/aos_p2p_diff_ranks',
                'week07/ex01/benchmarks/soa_diff_ranks']


mpi_df =parse_numbers_from_file(list_of_dirs)

# sequential_df = parse_numbers_from_file(['week06/ex01/benchmarks/baseline_diff_problem_sizes'])
# mpi_df = pd.merge(mpi_df, sequential_df[['size','seq runtime']], on='size')
mpi_df['speedup'] = 20.936755 / mpi_df['runtime'] 
mpi_df['efficiency'] = mpi_df['speedup'] / mpi_df['ranks']

plot_comparison(mpi_df, ['AOS collective', 'SOA collective', 'AOS P2P'])