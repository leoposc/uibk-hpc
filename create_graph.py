import re
import os
import plotly.graph_objs as go
from plotly.subplots import make_subplots
import plotly.io as py
import sys

def parse_numbers_from_file_and_plot(directory_path):
    # Initialize an empty dictionary to store the parsed data
    data_dict = {}
    
    # Regular expression pattern to match variables and numbers
    pattern = r'\$!(\w+)\{([\d\.]+),([\d\.]+)s\}'
    
    # Iterate over all files in the directory
    for filename in os.listdir(directory_path):
        file_path = os.path.join(directory_path, filename)
        
        # Open the file for reading
        with open(file_path, 'r') as file:
            content = file.read()
            
            # Use re.findall to find all matches in the content
            matches = re.findall(pattern, content)
            
            for match in matches:
                print("match")
                print(match)
                var_name = match[0]
                
                # Split the comma-separated numbers and convert them to floats
                numbers = [float(match[1]), float(match[2])]
                
                # Append the numbers to the data_dict
                if var_name not in data_dict:
                    data_dict[var_name] = []
                data_dict[var_name].append(numbers)
    
    # Sort the data_dict
    for var_name in data_dict:
        data_dict[var_name].sort(key=lambda x: x[0])

    # Create a Plotly subplot
    fig = make_subplots(rows=1, cols=1)
    
    print("Enter title")
    title = input()
    print("what to plot? speedup(s) efficiency(e) domain-specific(d) or time(t)")
    speedup = input()
    y_label= ""
    # Iterate through the parsed data and create traces
    for var_name, values in data_dict.items():
        y_values = []
        if(speedup == 'e'):
          print(values[0][1])
          print("input base speed for:")
          print(var_name)
          domain_size = float(input())
          print(value[0])
          y_values = [((values[0][1]/domain_size)/value[0]) for value in values]
          y_label = "efficiency"
        elif(speedup == 'd'):
          print(var_name)
          print("enter domain size")
          domain_size = float(input())
          print(values[0][1])
          y_values = [domain_size*value[0]/value[1] for value in values]
          y_label = "domain-specific"
        elif(speedup == 's'):
          print(values[0][1])
          y_values = [(values[0][1]/value[1]) for value in values]
          y_label = "speedup"
        else:
          y_values = [value[1] for value in values]
          y_label = "time"
        x_values = [value[0] for value in values]
        print(values)
        print(x_values)
        
        # Create a trace for each variable with a unique color
        trace = go.Scatter(x=x_values, y=y_values, mode='lines+markers', name=var_name)
        fig.add_trace(trace)

    # Set layout and labels
    if(y_label == "domain-specific"):
      print("enter y-label")
      y_label=input()
    fig.update_layout(
        title=title,
        xaxis=dict(title='rank'),
        yaxis=dict(title=y_label),
        width=1080,
        height=720
    )
    fig.update_xaxes(type="log")

    # Set y-axis to logarithmic scale
    # fig.update_yaxes(type="log")
    
    # Show the plot
    fig.show()
    py.write_image(fig, file_path + '.png')
    

# Example usage:
directory_path = str(sys.argv[1])
parse_numbers_from_file_and_plot(directory_path)