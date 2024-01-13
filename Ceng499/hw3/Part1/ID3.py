import numpy as np
from collections import Counter
# In the decision tree, non-leaf nodes are going to be represented via TreeNode
class TreeNode:
    def __init__(self, attribute):
        self.attribute = attribute
        # dictionary, k: subtree, key (k) an attribute value, value is either TreeNode or TreeLeafNode
        self.subtrees = {}

# In the decision tree, leaf nodes are going to be represented via TreeLeafNode
class TreeLeafNode:
    def __init__(self, data, label):
        self.data = data
        self.labels = label

class DecisionTree:
    def __init__(self, dataset: list, labels, features, criterion="information gain"):
        """
        :param dataset: array of data instances, each data instance is represented via an Python array
        :param labels: array of the labels of the data instances
        :param features: the array that stores the name of each feature dimension
        :param criterion: depending on which criterion ("information gain" or "gain ratio") the splits are to be performed
        """
        self.dataset = dataset
        self.labels = labels
        self.features = features
        self.criterion = criterion
        # it keeps the root node of the decision tree
        self.root = None

        # further variables and functions can be added...


    def calculate_entropy__(self, labels):
        """
        :param dataset: array of the data instances
        :param labels: array of the labels of the data instances
        :return: calculated entropy value for the given dataset
        """
        entropy_value = 0.0
        """
        Entropy calculations
        """
        label_counts = Counter(labels) # count the occurrences of each label
        probabilities = [count / len(labels) for count in label_counts.values()] # calculate the probabilities of each label
        entropy_value = -sum([p * np.log2(p) for p in probabilities if p > 0]) # calculate the entropy
        return entropy_value

    def calculate_average_entropy__(self, dataset, labels, attribute):
        """
        :param dataset: array of the data instances on which an average entropy value is calculated
        :param labels: array of the labels of those data instances
        :param attribute: for which attribute an average entropy value is going to be calculated...
        :return: the calculated average entropy value for the given attribute
        """
        average_entropy = 0.0
        """
            Average entropy calculations
        """
        attribute_index = self.features.index(attribute) # find the index of the attribute
        attribute_values = set([data[attribute_index] for data in dataset]) # get unique values of the attribute

        for value in attribute_values:
            subset = [data for data, label in zip(dataset, labels) if data[attribute_index] == value] # filter data for each attribute value
            subset_labels = [label for data, label in zip(dataset, labels) if data[attribute_index] == value] # get labels for the filtered data
            weight = len(subset) / len(dataset) # calculate weight of the subset
            entropy = self.calculate_entropy__(subset_labels) # calculate entropy for the subset
            average_entropy += weight * entropy # accumulate the weighted average entropy

        return average_entropy

    def calculate_information_gain__(self, dataset, labels, attribute):
        """
        :param dataset: array of the data instances on which an information gain score is going to be calculated
        :param labels: array of the labels of those data instances
        :param attribute: for which attribute the information gain score is going to be calculated...
        :return: the calculated information gain score
        """
        information_gain = 0.0
        """
            Information gain calculations
        """
        total_entropy = self.calculate_entropy__(labels) # calculate total entropy for the labels
        average_entropy = self.calculate_average_entropy__(dataset, labels, attribute) # calculate average entropy for the attribute
        information_gain = total_entropy - average_entropy # calculate information gain
        return information_gain

    def calculate_intrinsic_information__(self, dataset, labels, attribute):
        """
        :param dataset: array of data instances on which an intrinsic information score is going to be calculated
        :param labels: array of the labels of those data instances
        :param attribute: for which attribute the intrinsic information score is going to be calculated...
        :return: the calculated intrinsic information score
        """
        intrinsic_info = None
        """
            Intrinsic information calculations for a given attribute
        """
        attribute_index = self.features.index(attribute) # find the index of the attribute
        attribute_values = set([data[attribute_index] for data in dataset]) # get unique values of the attribute

        intrinsic_info = 0
        for value in attribute_values:
            subset = [data for data in dataset if data[attribute_index] == value] # filter data for each attribute value
            weight = len(subset) / len(dataset) # calculate weight of the subset
            if weight > 0:
                intrinsic_info -= weight * np.log2(weight) # accumulate the intrinsic information

        return intrinsic_info
    def calculate_gain_ratio__(self, dataset, labels, attribute):
        """
        :param dataset: array of data instances with which a gain ratio is going to be calculated
        :param labels: array of labels of those instances
        :param attribute: for which attribute the gain ratio score is going to be calculated...
        :return: the calculated gain ratio score
        """
        """
            Your implementation
        """
        information_gain = self.calculate_information_gain__(dataset, labels, attribute) # calculate information gain
        intrinsic_info = self.calculate_intrinsic_information__(dataset, labels, attribute) # calculate intrinsic information
        gain_ratio = information_gain / intrinsic_info if intrinsic_info != 0 else 0 # calculate gain ratio
        return gain_ratio

    def ID3__(self, dataset, labels, used_attributes):
        """
        Recursive function for ID3 algorithm
        :param dataset: data instances falling under the current  tree node
        :param labels: labels of those instances
        :param used_attributes: while recursively constructing the tree, already used labels should be stored in used_attributes
        :return: it returns a created non-leaf node or a created leaf node
        """
        """
            Your implementation
        """
        # check if all labels are the same, or no more features to split
        if len(set(labels)) == 1:
            return TreeLeafNode(dataset, labels[0]) # return a leaf node if all labels are the same
        if len(used_attributes) == len(self.features):
            most_common_label = Counter(labels).most_common(1)[0][0] # find the most common label
            return TreeLeafNode(dataset, most_common_label) # return a leaf node with the most common label

        # select the best attribute to split
        best_attribute = None
        best_score = -float('inf')
        for attribute in self.features:
            if attribute not in used_attributes:
                if self.criterion == "information gain":
                    score = self.calculate_information_gain__(dataset, labels, attribute)
                elif self.criterion == "gain ratio":
                    score = self.calculate_gain_ratio__(dataset, labels, attribute)
                if score > best_score:
                    best_score = score
                    best_attribute = attribute

        if best_attribute is None:
            most_common_label = Counter(labels).most_common(1)[0][0]
            return TreeLeafNode(dataset, most_common_label)

        # split the dataset based on the best attribute and recurse
        node = TreeNode(best_attribute)
        attribute_index = self.features.index(best_attribute)
        for value in set(data[attribute_index] for data in dataset):
            subset = [data for data in dataset if data[attribute_index] == value]
            subset_labels = [label for data, label in zip(dataset, labels) if data[attribute_index] == value]
            node.subtrees[value] = self.ID3__(subset, subset_labels, used_attributes + [best_attribute])

        return node
    def predict(self, x):
        """
        :param x: a data instance, 1 dimensional Python array 
        :return: predicted label of x
        
        If a leaf node contains multiple labels in it, the majority label should be returned as the predicted label
        """
        predicted_label = None
        """
            Your implementation
        """
        node = self.root
        while isinstance(node, TreeNode):
            attribute_value = x[self.features.index(node.attribute)] # get the value of the attribute for the input
            node = node.subtrees.get(attribute_value, None) # move to the next node based on the attribute value
            if node is None:
                return None  # not able to classify

        return node.labels # return the label of the leaf node

    def train(self):
        self.root = self.ID3__(self.dataset, self.labels, [])
        print("Training completed")