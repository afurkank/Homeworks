from utils import *

import pickle

dataset1 = pickle.load(open("../data/part2_dataset_1.data", "rb"))
dataset2 = pickle.load(open("../data/part2_dataset_2.data", "rb"))

datasets = [
    {"name":"dataset1","data":dataset1},
    {"name":"dataset2","data":dataset2},
]

K = 3 # I chose K as 3 from the graphs using elbow method

# dimensionality reduction methods
methods = ["UMAP", "TSNE", "PCA"]

# dimensionality reduction hyperparameters for TSNE and UMAP
metrics = ["euclidean", "cosine"]

for method in methods:
    for item in datasets:
        dataset_name = item['name']
        dataset = item['data']

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