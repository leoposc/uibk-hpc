import re
import plotly.graph_objs as go
from plotly.subplots import make_subplots
import plotly.io as py
import sys

def parse_numbers_from_file_and_plot(file_path):
    # Initialize an empty dictionary to store the parsed data
    data_dict = {}
    
    # Regular expression pattern to match variables and numbers
    pattern = r'\$!(\w+)\{([\d\.]+), ([\d\.]+)\}'
    
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
    
    x_min = min(min(value[0] for value in values) for values in data_dict.values())
    x_max = max(max(value[0] for value in values) for values in data_dict.values())
    y_min = min(min(value[1] for value in values) for values in data_dict.values())
    y_max = max(max(value[1] for value in values) for values in data_dict.values())
    
    # Create a Plotly subplot
    fig = make_subplots(rows=1, cols=1)
    
    # Iterate through the parsed data and create traces
    for var_name, values in data_dict.items():
        print(values)
        x_values = [value[0] for value in values]
        print(x_values)
        y_values = [value[1] for value in values]
        
        # Create a trace for each variable with a unique color
        trace = go.Scatter(x=x_values, y=y_values, mode='lines+markers', name=var_name)
        fig.add_trace(trace)
    
    # Set layout and labels
    fig.update_layout(
        title='Data Plot',
        xaxis=dict(title='package size'),
        yaxis=dict(title='latency'),
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

file_path = str(sys.argv[1])
parse_numbers_from_file_and_plot(file_path)