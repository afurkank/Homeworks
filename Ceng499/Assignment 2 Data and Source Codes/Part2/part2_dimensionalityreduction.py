from Kmedoids import KMedoids

import pickle
import numpy as np
import matplotlib.pyplot as plt

from sklearn.manifold import TSNE

from umap import UMAP

from sklearn.decomposition import PCA

dataset1 = pickle.load(open("../data/part2_dataset_1.data", "rb"))
dataset2 = pickle.load(open("../data/part2_dataset_2.data", "rb"))

datasets = [dataset1, dataset2]

k_values = [2, 3, 4, 5, 6, 7, 8, 9, 10]

results_for_k = []
for k in k_values:
    losses = []
    results = []
    for _ in range(5):
        kmedoids = KMedoids(dataset=dataset1, K=k, distance_metric='cosine')

        medoids, clusters, loss = kmedoids.run()
        print("Cluster Medoids:", medoids)
        print("Loss:", loss)

        losses.append(loss)
        results.append((k, medoids, clusters, loss))
    min_loss_idx = np.argmin(losses)
    min_loss_result = results[min_loss_idx]
    results_for_k.append(min_loss_result)

embeddings = []
for i,dataset in enumerate(datasets):
    X_TSNE = TSNE(
            n_components=2,
            learning_rate='auto',
            init='random',
            perplexity=30,
            metric="cosine",
            random_state=42
    ).fit_transform(dataset)

    X_UMAP = UMAP(
            n_components=2,
            metric="cosine",
            random_state=42,
    ).fit_transform(dataset)

    X_PCA = PCA(
            n_components=2,
            random_state=42,
    ).fit_transform(dataset)

    embeddings.append(
        {"dataset":i+1,"embedding":X_TSNE,"method_name":"TSNE"},
        {"dataset":i+1,"embedding":X_UMAP,"method_name":"UMAP"},
        {"dataset":i+1,"embedding":X_PCA,"method_name":"PCA"},
    )

for result in results_for_k:
    k, medoids, clusters, _ = results_for_k
    for item in embeddings:

    """ PLOT FOR TSNE DIM REDUCTION METHOD"""

    

    plt.figure(figsize=(8,6))
    # get indices of all points in a cluster for every cluster
    for k, cluster in clusters.items():
        # for a single cluster, get its embedded version
        cluster_points = X_TSNE[np.array(cluster)]
        # scatter the points in the x-y axis(reduced to only 2 features)
        plt.scatter(cluster_points[:, 0], cluster_points[:, 1], label=f"Cluster {k}")
    # scatter the medoids(embedded version) on the x-y axis
    plt.scatter(X_TSNE[[medoids[k] for k in medoids], 0], X_TSNE[[medoids[k] for k in medoids], 1], 
                color='black', marker='*', label='Medoids')
    plt.title("K-medoids Clustering-TSNE")
    plt.xlabel("TSNE Component 1")
    plt.ylabel("TSNE Component 2")
    plt.legend()
    plt.savefig('KMedoids_TSNE.png')
    plt.close()


    """PLOT FOR UMAP DIM REDUCTION METHOD"""

    
    
    plt.figure(figsize=(8,6))
    # get indices of all points in a cluster for every cluster
    for k, cluster in clusters.items():
        # for a single cluster, get its embedded version
        cluster_points = X_UMAP[np.array(cluster)]
        # scatter the points in the x-y axis(reduced to only 2 features)
        plt.scatter(cluster_points[:, 0], cluster_points[:, 1], label=f"Cluster {k}")
    # scatter the medoids(embedded version) on the x-y axis
    plt.scatter(X_UMAP[[medoids[k] for k in medoids], 0], X_UMAP[[medoids[k] for k in medoids], 1], 
                color='black', marker='*', label='Medoids')
    plt.title("K-medoids Clustering-TSNE")
    plt.xlabel("TSNE Component 1")
    plt.ylabel("TSNE Component 2")
    plt.legend()
    plt.savefig('KMedoids_TSNE.png')
    plt.close()


    """PLOT FOR PCA DIM REDUCTION METHOD"""

    

    plt.figure(figsize=(8,6))
    # get indices of all points in a cluster for every cluster
    for k, cluster in clusters.items():
        # for a single cluster, get its embedded version
        cluster_points = X_PCA[np.array(cluster)]
        # scatter the points in the x-y axis(reduced to only 2 features)
        plt.scatter(cluster_points[:, 0], cluster_points[:, 1], label=f"Cluster {k}")
    # scatter the medoids(embedded version) on the x-y axis
    plt.scatter(X_PCA[[medoids[k] for k in medoids], 0], X_PCA[[medoids[k] for k in medoids], 1], 
                color='black', marker='*', label='Medoids')
    plt.title("K-medoids Clustering-TSNE")
    plt.xlabel("TSNE Component 1")
    plt.ylabel("TSNE Component 2")
    plt.legend()
    plt.savefig('KMedoids_TSNE.png')
    plt.close()