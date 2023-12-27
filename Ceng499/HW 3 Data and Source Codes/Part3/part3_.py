import numpy as np
from DataLoader import DataLoader
import pandas as pd
from sklearn.neighbors import KNeighborsClassifier
from sklearn.svm import SVC
from sklearn.pipeline import make_pipeline
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import GridSearchCV, RepeatedStratifiedKFold

data_path       = "data/credit.data"

""" ##############   TRAINING DECISION TREE CLASSIFIER      ############## """
# load data without one-hot encoding
dataset_, labels_ = DataLoader.load_credit(data_path)

# standardize
scaler        = StandardScaler()
scaler.fit_transform(dataset_) # calculate mean and std

dt_classifier = DecisionTreeClassifier(max_depth=5, min_samples_split=4)

# train the decision tree
dt_classifier.fit(dataset_, labels_)

# print the importance of features
feature_importance_arr = dt_classifier.feature_importances_
print(f"\nThe importance of features are:")
importance_dict = {f"attr_{i+1}": f"{importance:.2f}" for i,importance in enumerate(feature_importance_arr)}
print(importance_dict)

""" ##############   TRAINING A SUPPORT VECTOR CLASSIFIER   ##############  """
# load data without one-hot encoding
dataset, labels = DataLoader.load_credit_with_onehot(data_path)
print(len(dataset))
# standardize
scaler        = StandardScaler()
scaler.fit_transform(dataset) # calculate mean and std

sv_classifier = SVC(C=1, gamma='scale', kernel="linear")

sv_classifier.fit(dataset, labels)

support_vector_arr = sv_classifier.support_vectors_
print("\n", len(support_vector_arr))
print(sv_classifier.n_support_)
print(support_vector_arr[0])
print(f"\nThe support vectors are:\n{sv_classifier.support_vectors_}")