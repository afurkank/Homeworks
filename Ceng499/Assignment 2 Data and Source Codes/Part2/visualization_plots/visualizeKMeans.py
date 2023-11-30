from sklearn.datasets import load_iris
from Kmeans import KMeans
import numpy as np
import matplotlib.pyplot as plt
from sklearn.decomposition import PCA

# Load the Iris dataset
iris = load_iris()
data = iris.data

# Initialize and run KMeans
kmeans = KMeans(data, K=2)
_, _, loss = kmeans.run()
print(f"Loss: {loss:.3f}")
# Print the cluster centers
for k, center in kmeans.cluster_centers.items():
    print(f"Cluster {k}: {center}")

# Print the clusters
for k, cluster in kmeans.clusters.items():
    print(f"Cluster {k}: {len(cluster)} points")

colors = ['r', 'g', 'b', 'y', 'c', 'm']
fig, ax = plt.subplots()

# Perform PCA on the data
pca = PCA(n_components=2)
pca.fit(kmeans.dataset)
dataset_2d = pca.transform(kmeans.dataset)

# Perform PCA on the cluster centers
centers_2d = pca.transform(list(kmeans.cluster_centers.values()))

for i in range(kmeans.K):
    points = np.array([dataset_2d[j] for j in range(len(dataset_2d)) if kmeans.labels[j] == i])
    ax.scatter(points[:, 0], points[:, 1], s=7, c=colors[i])

ax.scatter(centers_2d[:, 0], centers_2d[:, 1], marker='*', s=200, c='#050505')

plt.savefig('visualizeKMeans.png')
plt.show()
plt.close()