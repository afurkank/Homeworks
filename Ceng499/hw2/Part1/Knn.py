import math

class KNN:
    def __init__(self, dataset, data_label, similarity_function, similarity_function_parameters=None, K=1):
        """
        :param dataset: dataset on which KNN is executed, 2D numpy array
        :param data_label: class labels for each data sample, 1D numpy array
        :param similarity_function: similarity/distance function, Python function
        :param similarity_function_parameters: auxiliary parameter or parameter array for distance metrics
        :param K: how many neighbors to consider, integer
        """
        self.K = K
        self.dataset = dataset
        self.dataset_label = data_label
        self.similarity_function = similarity_function
        self.similarity_function_parameters = similarity_function_parameters

    def predict(self, instance):
        labels = [] # record the voted labels
        points_voted_before = [] # keep track of which points voted before
        
        for _ in range(self.K):
            closest_point = 0 # hold the index of data point closest to instance
            min_dist = math.inf

            for j,data_point in enumerate(self.dataset):
                if (j in points_voted_before):
                    # if this point voted before, continue
                    continue
                
                # calculate distance between data point and instance
                distance = self.similarity_function(
                    instance,
                    data_point,
                    self.similarity_function_parameters,
                )
                
                # if the data point is closest, record it
                if (min_dist > distance):
                    min_dist = distance
                    closest_point = j
            
            # we completed the search, now our closes point votes
            points_voted_before.append(closest_point) # make sure it doesn't vote twice
            # record its label
            closest_label = self.dataset_label[closest_point]
            labels.append(closest_label)
        
        # after we are done with all the points, we look at the majority vote
        voted_label = None
        num_majority_vote = 0
        for label in labels:
            count = labels.count(label) # count how many times this label was voted

            if (num_majority_vote < count):
                # if there is a label with more votes, record it
                num_majority_vote = count
                voted_label = label
        
        return voted_label # return the majority voted label