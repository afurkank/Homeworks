import pickle
import numpy as np
import matplotlib.cm as cm
import matplotlib.pyplot as plt

from sklearn.cluster import AgglomerativeClustering

from umap import UMAP
from sklearn.manifold import TSNE

dataset = pickle.load(open("../data/part3_dataset.data", "rb"))

linkage = 'single'
hac_metric = 'cosine'
# dimensionality reduction methods
dim_reduc_methods = ["UMAP", "TSNE"]
# dimensionality reduction hyperparameters for TSNE and UMAP
dim_reduc_metrics = ["euclidean", "cosine"]
# K is chosen as 4 via the silhouette method
K = 4

for method in dim_reduc_methods:
    for metric in dim_reduc_metrics:
        embedded_data = None
        if metric == "TSNE":
            x_TSNE = TSNE(
                n_components=2,
                learning_rate='auto',
                init='random',
                perplexity=30,
                metric=metric,
                random_state=42
            )
            embedded_data = x_TSNE.fit_transform(dataset)
        else:
            x_UMAP = UMAP(
                n_components=2,
                metric=metric,
                random_state=42,
            )
            embedded_data = x_UMAP.fit_transform(dataset)

        clusterer = AgglomerativeClustering(n_clusters=K, metric=hac_metric, linkage=linkage)
        cluster_labels = clusterer.fit_predict(dataset)

        # calculate the cluster centers
        # calculate cluster centers
        centers = []
        for i in np.unique(cluster_labels):
            centers.append(embedded_data[cluster_labels == i].mean(axis=0))

        centers = np.array(centers)

        plt.figure(figsize=(8,6))

        colors = cm.nipy_spectral(cluster_labels.astype(float) / K)
        plt.scatter(
            embedded_data[:,0], 
            embedded_data[:,1], 
            marker=".", 
            s=30, lw=0, alpha=0.7, 
            c=colors, 
            edgecolor="k"
        )

        for i, c in enumerate(centers):
            plt.scatter(c[0], c[1], marker="*", c='black', alpha=1, s=200)
        plt.suptitle(
            f"Dim Reduction for Method = {method}, Metric = {metric}, K = {K}",
            fontsize=14,
            fontweight="bold",
        )
        plt.savefig(f"DimReduction_{method}_{metric}")