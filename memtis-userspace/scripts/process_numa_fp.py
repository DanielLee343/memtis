import pandas as pd
import matplotlib.pyplot as plt
import sys
import numpy as np

file_path = sys.argv[1]
output_img = sys.argv[2]
if output_img == None:
    exit(1)
df = pd.read_csv(file_path, sep=':', header=None, names=['Node', 'Memory_MB'])

# Process data
df['Node'] = df['Node'].apply(lambda x: x.split()[1])  # Extract node number
df['Memory_MB'] = df['Memory_MB'].apply(
    lambda x: x.split()[0].strip())  # Extract memory value
# Convert memory values to numeric
df['Memory_MB'] = pd.to_numeric(df['Memory_MB'])

# Find maximum memory values for each node
max_memory_node_0 = df[df['Node'] == '0']['Memory_MB'].max()
max_memory_node_1 = df[df['Node'] == '1']['Memory_MB'].max()

# Subtract current memory from the maximum memory for each node
df['Memory_Used'] = df.apply(lambda row: (max_memory_node_0 - row['Memory_MB'])
                             if row['Node'] == '0' else (max_memory_node_1 - row['Memory_MB']), axis=1)

# Split data into two series for node 0 and node 1
node_0 = df[df['Node'] == '0']['Memory_Used']
node_1 = df[df['Node'] == '1']['Memory_Used']
print("DRAM max: ", node_0.max(), "MB")
print("CXL max: ", node_1.max(), "MB")

# Plotting the dat``
x_range = np.arange(1, len(node_0) + 1) / 10
plt.figure(figsize=(10, 6))
plt.plot(x_range, node_0, label='DRAM Usage')
plt.plot(x_range, node_1, label='CXL Usage')
plt.title('Memory Footprint for DRAM and CXL')
plt.xlabel('Time')
plt.ylabel('Memory Footprint(MB)')
plt.legend()
plt.savefig(output_img + '_fp.png')

# get combined max
node_0 = node_0.reset_index()
node_1 = node_1.reset_index()
node_0.drop('index', axis=1, inplace=True)
node_1.drop('index', axis=1, inplace=True)
min_length = min(len(node_0), len(node_1))
node_0 = node_0.iloc[:min_length]
node_1 = node_1.iloc[:min_length]

node_0.rename(columns={'Memory_Used': 'Node_0_Memory'}, inplace=True)
node_1.rename(columns={'Memory_Used': 'Node_1_Memory'}, inplace=True)
combined = node_0.join(node_1, how='inner')
combined['Total_Memory_Used'] = combined['Node_0_Memory'] + \
    combined['Node_1_Memory']
max_combined_memory = combined['Total_Memory_Used'].max()

print("Max mem total: ", max_combined_memory)
