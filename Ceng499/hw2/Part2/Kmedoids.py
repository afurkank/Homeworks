import numpy as np

class KMedoids:
    def __init__(self, dataset, K=2, distance_metric="cosine"):
        """
        :param dataset: 2D numpy array, the dataset to be clustered.
        :param K: integer, the number of clusters to form.
        """
        self.K = K
        self.dataset = dataset
        self.distance_metric = distance_metric
        # each cluster is represented with an integer index
        # self.clusters stores the data points of each cluster in a dictionary
        # In this dictionary, you can keep either the data instance themselves or their corresponding indices in the dataset (self.dataset).
        self.clusters = {i: [] for i in range(K)}
        # self.cluster_medoids stores the cluster medoid for each cluster in a dictionary
        # # In this dictionary, you can keep either the data instance themselves or their corresponding indices in the dataset (self.dataset).
        self.cluster_medoids = {i: None for i in range(K)}
        # you are free to add further variables and functions to the class
        self.norms = np.linalg.norm(dataset, axis=1)  # precompute norms for distance calculations
    def initialize(self):
        """
        initialize the cluster medoids from random data points
        """
        # get K random indices
        indices = np.random.choice(len(self.dataset), self.K, replace=False)
        # loop through the randomly selected indices
        for k, index in enumerate(indices):
            # assign the random indices as cluster medoids(centers)
            self.cluster_medoids[k] = index
    def calculateDistance(self, data_point, instance):
        """
        calculate the cosine similarity between 2 points
        """
        numerator = np.dot(self.dataset[data_point], self.dataset[instance])
        denominator = self.norms[data_point] * self.norms[instance]
        return 1 - (numerator / denominator)
    def calculateLoss(self):
        """Loss function implementation of Equation 2"""
        loss = 0
        for k in range(self.K):
            loss += np.sum([self.calculateDistance(index, self.cluster_medoids[k]) for index in self.clusters[k]])
        return loss
    def run(self):
        """Kmedoids algorithm implementation"""
        # first initialize the medoids
        self.initialize()

        # update the clusters until convergence
        while True:
            # empty all clusters
            self.clusters = {i: [] for i in range(self.K)}
            # loop through each data point
            for i, data_point in enumerate(self.dataset):
                # calculate the data point's distance to each cluster medoid
                distances = [self.calculateDistance(i, self.cluster_medoids[k]) for k in range(self.K)]
                # get the index of closest medoid
                closest_cluster = np.argmin(distances) 
                # assign the data point to the closest cluster
                self.clusters[closest_cluster].append(i)

            # update the cluster medoids

            # copy the old medoids first for comparison
            old_medoids = self.cluster_medoids.copy()
            # loop through each medoid
            for k in range(self.K):
                min_loss = float('inf')
                best_medoid = None
                # loop through all the data points in that medoids' cluster
                for index in self.clusters[k]:
                    # sum all the distances between a data point and all the other data points in that cluster
                    current_loss = sum([self.calculateDistance(i, index) for i in self.clusters[k]])
                    # check if the loss is better(less) than the min loss obtained so far
                    if current_loss < min_loss:
                        # update min loss
                        min_loss = current_loss
                        # update best medoid as the data point that results in min loss
                        best_medoid = index
                # update the cluster medoid
                self.cluster_medoids[k] = best_medoid

            # check for convergence
            if old_medoids == self.cluster_medoids:
                # break if there is no update
                break

        return self.cluster_medoids, self.clusters, self.calculateLoss()