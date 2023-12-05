import pickle
import numpy as np
from sklearn.cluster import AgglomerativeClustering
from scipy.cluster.hierarchy import dendrogram, linkage
import matplotlib.pyplot as plt
import matplotlib.cm as cm
from sklearn.metrics import silhouette_samples, silhouette_score

dataset = pickle.load(open("../data/part3_dataset.data", "rb"))

K_values = [2, 3, 4, 5]
linkages = ['single', 'complete']
metrics = ['euclidean', 'cosine']

"""  THIS PART IS FOR PLOTTING THE DENDROGRAM GRAPHS  """
for linkage_method in linkages:
    for metric in metrics:
        # create a linkage matrix
        linkage_mat = linkage(dataset, method=linkage_method, metric=metric)

        # Plot the dendrogram
        plt.figure(figsize=(10, 7))
        plt.title(f"Dendrogram for Linkage={linkage_method}, Metric={metric}")
        dendrogram(linkage_mat)
        plt.savefig(f"Dendrogram_{linkage_method}_{metric}")
        plt.show()

"""  THIS PART IS FOR PLOTTING THE SILHOUETTE AVG SCORE GRAPHS AND FINDING THE BEST CONFIG  """
# loop through hyperparameter configurations
for linkage_method in linkages:
    for metric in metrics:
        best_K = None
        best_score = -1
        for n_clusters in K_values:
            # Create a subplot with 1 row and 2 columns
            fig, (ax1, ax2) = plt.subplots(1, 2)
            fig.set_size_inches(18, 7)

            # The 1st subplot is the silhouette plot
            # The silhouette coefficient can range from -1, 1 but in this example all
            # lie within [-0.1, 1]
            ax1.set_xlim([-0.1, 1])
            # The (n_clusters+1)*10 is for inserting blank space between silhouette
            # plots of individual clusters, to demarcate them clearly.
            ax1.set_ylim([0, len(dataset) + (n_clusters + 1) * 10])

            # HAC clustering
            clusterer = AgglomerativeClustering(n_clusters=n_clusters, metric=metric, linkage=linkage_method)
            cluster_labels = clusterer.fit_predict(dataset)

            # calculate the cluster centers
            # Calculate cluster centers
            centers = []
            for i in np.unique(cluster_labels):
                centers.append(dataset[cluster_labels == i].mean(axis=0))

            centers = np.array(centers)
            
            # The silhouette_score gives the average value for all the samples.
            # This gives a perspective into the density and separation of the formed
            # clusters
            silhouette_avg = silhouette_score(dataset, cluster_labels)
            print(
                f"For linkage: {linkage_method}, metric: {metric}, K: {n_clusters} the average silhouette_score is: {silhouette_avg}")

            # Compute the silhouette scores for each sample
            sample_silhouette_values = silhouette_samples(dataset, cluster_labels)

            y_lower = 10
            for i in range(n_clusters):
                # Aggregate the silhouette scores for samples belonging to
                # cluster i, and sort them
                ith_cluster_silhouette_values = sample_silhouette_values[cluster_labels == i]

                ith_cluster_silhouette_values.sort()

                size_cluster_i = ith_cluster_silhouette_values.shape[0]
                y_upper = y_lower + size_cluster_i

                color = cm.nipy_spectral(float(i) / n_clusters)
                ax1.fill_betweenx(
                    np.arange(y_lower, y_upper),
                    0,
                    ith_cluster_silhouette_values,
                    facecolor=color,
                    edgecolor=color,
                    alpha=0.7,
                )

                # Label the silhouette plots with their cluster numbers at the middle
                ax1.text(-0.05, y_lower + 0.5 * size_cluster_i, str(i))

                # Compute the new y_lower for next plot
                y_lower = y_upper + 10  # 10 for the 0 samples

            ax1.set_title("The silhouette plot for the various clusters.")
            ax1.set_xlabel("The silhouette coefficient values")
            ax1.set_ylabel("Cluster label")

            # The vertical line for average silhouette score of all the values
            ax1.axvline(x=silhouette_avg, color="red", linestyle="--")

            ax1.set_yticks([])  # Clear the yaxis labels / ticks
            ax1.set_xticks([-0.1, 0, 0.2, 0.4, 0.6, 0.8, 1])

            # 2nd Plot showing the actual clusters formed
            colors = cm.nipy_spectral(cluster_labels.astype(float) / n_clusters)
            ax2.scatter(
                dataset[:, 0], dataset[:, 1], marker=".", s=30, lw=0, alpha=0.7, c=colors, edgecolor="k"
            )

            # Labeling the clusters
            # Draw white circles at cluster centers
            ax2.scatter(
                centers[:, 0],
                centers[:, 1],
                marker="o",
                c="white",
                alpha=1,
                s=200,
                edgecolor="k",
            )

            for i, c in enumerate(centers):
                ax2.scatter(c[0], c[1], marker="$%d$" % i, alpha=1, s=50, edgecolor="k")

            ax2.set_title("The visualization of the clustered data.")
            ax2.set_xlabel("Feature space for the 1st feature")
            ax2.set_ylabel("Feature space for the 2nd feature")

            plt.suptitle(
                f"Silhouette analysis, Linkage = {linkage_method}, Metric = {metric}, K = {n_clusters}",
                fontsize=14,
                fontweight="bold",
            )

            plt.savefig(f"Silhouette_{linkage_method}_{metric}_K{n_clusters}")

