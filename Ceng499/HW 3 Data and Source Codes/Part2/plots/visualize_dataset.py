import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import pickle

# load your dataset
dataset, labels = pickle.load(open("../data/part2_dataset2.data", "rb"))

# create a new matplotlib figure and 3D axis
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# extract each feature for plotting
x = dataset[:, 0]
y = dataset[:, 1]
z = dataset[:, 2]

# plot the data as a 3D scatter plot
# optionally use labels to color the points
scatter = ax.scatter(x, y, z, c=labels, cmap='viridis')

# adding labels for each axis
ax.set_xlabel('Feature 1')
ax.set_ylabel('Feature 2')
ax.set_zlabel('Feature 3')

# adding a color bar to show the label color mapping
colorbar = fig.colorbar(scatter, ax=ax, extend='both')
colorbar.set_label('Labels')

# Show the plot
plt.savefig("dataset2_visualization.png")
plt.show()
plt.close()

# Load your dataset
dataset, labels = pickle.load(open("../data/part2_dataset1.data", "rb"))

# Assuming dataset is a NumPy array of shape (n_samples, 2)
# where each row represents a sample and each column represents a feature

# Create a new matplotlib figure and 3D axis
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Extract each feature for plotting
x = dataset[:, 0]
y = dataset[:, 1]

# Generate a constant or derived z-axis
# Example: setting all z-values to a constant
z = np.zeros_like(x)  # or any other function of x and y

# Plot the data as a 3D scatter plot
# Optionally use labels to color the points
scatter = ax.scatter(x, y, z, c=labels, cmap='viridis')

# Adding labels for each axis
ax.set_xlabel('Feature 1')
ax.set_ylabel('Feature 2')
ax.set_zlabel('Artificial Feature')

# Adding a color bar to show the label color mapping
colorbar = fig.colorbar(scatter, ax=ax, extend='both')
colorbar.set_label('Labels')

# Show the plot
plt.savefig("dataset1_visualization.png")
plt.show()
plt.close()