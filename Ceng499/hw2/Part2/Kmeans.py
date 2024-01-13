import numpy as np

class KMeans:
    def __init__(self, dataset, K=2):
        """
        :param dataset: 2D numpy array, the whole dataset to be clustered
        :param K: integer, the number of clusters to form
        """
        self.K = K
        self.dataset = dataset
        # each cluster is represented with an integer index
        # self.clusters stores the data points of each cluster in a dictionary
        self.clusters = {i: [] for i in range(K)}
        # self.cluster_centers stores the cluster mean vectors for each cluster in a dictionary
        self.cluster_centers = {i: None for i in range(K)}
        # you are free to add further variables and functions to the class
        self.cluster_indices = {i: [] for i in range(K)}
        # I use the labels for visualization purposes
        self.labels = [-1 for _ in range(len(self.dataset))]
    def initialize(self):
        """
        initialize the cluster medoids from random data points
        """
        # get K random indices
        indices = np.random.choice(len(self.dataset), self.K, replace=False)
        # loop through the randomly selected indices
        for k, index in enumerate(indices):
            # assign the random indices as cluster centers
            self.cluster_centers[k] = self.dataset[index]
    def calculateLoss(self):
        """Loss function implementation of Equation 1"""
        loss = 0
        for k in range(self.K):
            loss += np.sum([np.linalg.norm(self.dataset[index]-self.cluster_centers[k])**2 for index in self.cluster_indices[k]])
            """cluster = self.clusters[k]
            cluster_center = self.cluster_centers[k]
            for data_point in cluster:
                loss += np.linalg.norm(data_point - cluster_center)**2"""
        return loss
    def run(self):
        """Kmeans algorithm implementation"""
        # first initialize the cluster centers
        self.initialize()

        # update the clusters until there is convergence
        while True:
            # empty all clusters
            self.clusters = {i: [] for i in range(self.K)}
            self.cluster_indices = {i: [] for i in range(self.K)}
            # clear labels
            self.labels = [-1 for _ in range(len(self.dataset))]

            # loop through each data point
            for i, data_point in enumerate(self.dataset):
                # calculate the distance between the data point and every cluster center
                distances = [np.linalg.norm(data_point - self.cluster_centers[k])**2 for k in range(self.K)]
                # get the index of closest cluster
                closest_cluster = np.argmin(distances)
                # assign data point to the closest cluster
                self.clusters[closest_cluster].append(data_point)
                self.cluster_indices[closest_cluster].append(i)
                # record cluster index as its label
                self.labels[i] = closest_cluster

            # update cluster centers and check for convergence
            
            new_centers = {}
            # loop through each cluster
            for k in range(self.K):
                # get the mean(new center) of updated clusters
                new_centers[k] = np.mean(self.clusters[k], axis=0) if len(self.clusters[k]) > 0 else self.cluster_centers[k]
            
            # check if centers have changed
            if all(np.array_equal(new_centers[k], self.cluster_centers[k]) for k in range(self.K)):
                # convergence achieved
                break

            self.cluster_centers = new_centers

        return self.cluster_centers, self.clusters, self.calculateLoss()