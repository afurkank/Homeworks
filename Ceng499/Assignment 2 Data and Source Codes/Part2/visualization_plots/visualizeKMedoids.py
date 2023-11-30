from Kmedoids import KMedoids
import numpy as np
import matplotlib.pyplot as plt
from sklearn.decomposition import PCA
from sklearn.datasets import make_blobs
import pickle

# Step 1: Prepare the Dataset
# Generate a synthetic 4-dimensional dataset
X, _ = make_blobs(n_samples=300, centers=4, n_features=4, random_state=42)

dataset1 = pickle.load(open("../data/part2_dataset_1.data", "rb"))

# Step 2: Run K-medoids Algorithm
kmedoids = KMedoids(dataset=X, K=5, distance_metric='cosine')
medoids, clusters, loss = kmedoids.run()
print("Cluster Medoids:", medoids)
print("Loss:", loss)

# Step 3: Dimensionality Reduction
pca = PCA(n_components=2)  # Reduce to 2 dimensions for visualization
X_reduced = pca.fit_transform(X)

# Step 4: Visualize the Clusters
plt.figure(figsize=(8, 6))
for k, cluster in clusters.items():
    cluster_points = X_reduced[np.array(cluster)]
    plt.scatter(cluster_points[:, 0], cluster_points[:, 1], label=f"Cluster {k}")
plt.scatter(X_reduced[[medoids[k] for k in medoids], 0], X_reduced[[medoids[k] for k in medoids], 1], 
            color='black', marker='x', label='Medoids')
plt.title("K-medoids Clustering")
plt.xlabel("PCA Component 1")
plt.ylabel("PCA Component 2")
plt.legend()
plt.savefig('visualizeKMedoids.png')
plt.close()
