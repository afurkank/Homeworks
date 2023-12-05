import pickle
import numpy as np
from sklearn.cluster import AgglomerativeClustering

dataset = pickle.load(open("../data/part3_dataset.data", "rb"))

K_values = [2, 3, 4, 5]

linkages = ['single', 'complete']

metrics = ['euclidean', 'cosine']

for K in K_values:
    for linkage in linkages:
        for metric in metrics:
            clustering = AgglomerativeClustering(
                n_clusters=2,
                metric='cosine',
                linkage='single',
            ).fit_predict(dataset)
