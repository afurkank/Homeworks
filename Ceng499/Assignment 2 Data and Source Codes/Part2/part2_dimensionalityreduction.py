import pickle

from Kmedoids import KMedoids
from Kmeans import KMeans

import numpy as np
import matplotlib.pyplot as plt

from umap import UMAP
from sklearn.manifold import TSNE
from sklearn.decomposition import PCA

"""
#######################################################################################################

UTILITY FUNCTIONS BEGIN HERE

#######################################################################################################

"""

def execute_kmedoids(dataset, k, distance_metric='cosine'):
    losses = []
    results = []
    for _ in range(5):
        model = KMedoids(dataset=dataset, K=k, distance_metric=distance_metric)
        medoids, clusters, loss = model.run()
        losses.append(loss)
        results.append((medoids, clusters, loss))
    medoids, clusters, loss = results[np.argmin(losses)]
    return medoids, clusters, loss

def execute_kmeans(dataset, k):
    losses = []
    results = []
    for _ in range(5):
        model = KMeans(dataset=dataset, K=k)
        cluster_centers, clusters, loss = model.run()
        losses.append(loss)
        results.append((cluster_centers, clusters, loss, model))
    cluster_centers, clusters, loss, model = results[np.argmin(losses)]
    return cluster_centers, clusters, loss, model

def get_embedding_class(method, metric):
    emb_class = None
    if method == "TSNE":
        emb_class = TSNE(
            n_components=2,
            learning_rate='auto',
            init='random',
            perplexity=30,
            metric=metric,
            random_state=42
        )
    elif method == "UMAP":
        emb_class = UMAP(
        n_components=2,
        metric=metric,
        random_state=42,
    )
    else:
        emb_class = PCA(
            n_components=2,
            random_state=42,
        )
    return emb_class

def plot_graph_kmedoids(embedded_data, medoids, clusters, graph_name, method, title):
    plt.figure(figsize=(8,6))

    # get indices of all points in a cluster for every cluster
    for k, cluster in clusters.items():
        # for a single cluster, get its embedded version
        cluster_points = embedded_data[np.array(cluster)]
        # scatter the points in the x-y axis(reduced to only 2 features)
        plt.scatter(cluster_points[:, 0], cluster_points[:, 1], label=f"Cluster {k}")
    
    # scatter the medoids(embedded version) on the x-y axis
    plt.scatter(embedded_data[[medoids[k] for k in medoids], 0], embedded_data[[medoids[k] for k in medoids], 1], 
                color='black', marker='*', label="Medoids")
    
    plt.title(title)
    plt.xlabel(method + " Component 1")
    plt.ylabel(method + " Component 2")
    plt.legend()
    plt.savefig(graph_name)
    plt.show()
    plt.close()

def plot_graph_kmeans(embedded_data, model:KMeans, graph_name, method, title):
    plt.figure(figsize=(8,6))

    colors = ['r', 'g', 'b', 'y', 'c', 'm', 'k', 'orange', 'purple', 'brown']
    
    # get indices of all points in a cluster for every cluster
    for k, cluster in model.cluster_indices.items():
        # for a single cluster, get its embedded version
        cluster_points = embedded_data[cluster]
        # scatter the points in the x-y axis(reduced to only 2 features)
        plt.scatter(cluster_points[:, 0], cluster_points[:, 1], s=7, color=colors[k % len(colors)], label=f"Cluster {k}")

    # scatter cluster centers
    for k, center in model.cluster_centers.items():
        plt.scatter(center[0], center[1], color='black', marker='*', s=200, label=f"Center of Cluster {k}" if k == 0 else None)

    plt.title(title)
    plt.xlabel(method + " Component 1")
    plt.ylabel(method + " Component 2")
    plt.legend()
    plt.savefig(graph_name)
    plt.show()
    plt.close()

"""
#######################################################################################################

UTILITY FUNCTIONS END HERE

#######################################################################################################

"""

dataset1 = pickle.load(open("../data/part2_dataset_1.data", "rb"))
dataset2 = pickle.load(open("../data/part2_dataset_2.data", "rb"))

datasets = [
    {"name":"dataset1","data":dataset1},
    {"name":"dataset2","data":dataset2},
]

# dimensionality reduction methods
methods = ["UMAP", "TSNE", "PCA"]

# dimensionality reduction hyperparameters for TSNE and UMAP
metrics = ["euclidean", "cosine"]

for method in methods:
    for item in datasets:
        dataset_name = item['name']
        dataset = item['data']
        K = None
        if dataset_name == "dataset1":
            K = 5 # for dataset1, using the elbow method, I chose K as 5
        else:
            K = 3 # similarly, I chose K as 3 using the elbow method for dataset2
        if method == 'PCA':
            emb_class = get_embedding_class(
                method=method,
                metric=None,
            )
            embedded_data = emb_class.fit_transform(dataset)

            cluster_centers, clusters_kmeans, loss_kmeans, model = execute_kmeans(embedded_data, K)
            medoids, clusters_kmedoids, loss_kmedoids = execute_kmedoids(embedded_data, K)

            plot_graph_kmeans(
                embedded_data=embedded_data,
                model=model,
                graph_name='KMeans_' + method + '_' + str(dataset_name) + '.png',
                method=method,
                title="K-means Clustering-" + str(method),
            )

            plot_graph_kmedoids(
                embedded_data= embedded_data,
                medoids=medoids,
                clusters=clusters_kmedoids,
                graph_name='KMedoids_' + method + '_' + str(dataset_name) + '.png',
                method=method,
                title="K-medoids Clustering-" + str(method),
            )
        else:
            for metric in metrics:
                emb_class = get_embedding_class(
                    method=method,
                    metric=metric,
                )
                embedded_data = emb_class.fit_transform(dataset)

                cluster_centers, clusters_kmeans, loss_kmeans, model = execute_kmeans(embedded_data, K)
                medoids, clusters_kmedoids, loss_kmedoids = execute_kmedoids(embedded_data, K)
                
                plot_graph_kmeans(
                    embedded_data=embedded_data,
                    model=model,
                    graph_name='KMeans_' + method + '_' + str(metric) + '_' + str(dataset_name) + '.png',
                    method=method,
                    title="K-means Clustering-" + str(method),
                )

                plot_graph_kmedoids(
                    embedded_data= embedded_data,
                    medoids=medoids,
                    clusters=clusters_kmedoids,
                    graph_name='KMedoids_' + method + '_' + str(metric) + '_' + str(dataset_name) + '.png',
                    method=method,
                    title="K-medoids Clustering-" + str(method),
                )