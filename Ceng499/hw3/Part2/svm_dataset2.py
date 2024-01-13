import pickle
import numpy as np
import pandas as pd
from sklearn.svm import SVC
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import GridSearchCV

dataset, labels = pickle.load(open("../data/part2_dataset2.data", "rb"))

# hyperparameters to test
Cs = [0.1, 0.5, 0.8, 1, 1.5, 3, 5, 7, 10]
kernels = ('linear', 'poly', 'rbf', 'sigmoid')
degree = 8

parameters = {
    'kernel' : kernels, 
    'C' : Cs
}

avg_avg_acc = np.zeros((len(kernels) * len(Cs),)) # (len(kernels) * len(Cs),) = (36,) - 36 is the number of configurations tested
avg_mean_scores = np.zeros((len(kernels) * len(Cs),)) # (len(kernels) * len(Cs),) = (36,) - 36 is the number of configurations tested
avg_std_scores = np.zeros((len(kernels) * len(Cs),)) # (len(kernels) * len(Cs),) = (36,) - 36 is the number of configurations tested

for _ in range(5): # repeat the cross validation 5 times
    # shuffle the dataset
    indices = np.arange(dataset.shape[0])
    np.random.shuffle(indices)
    dataset_shuffled = dataset[indices]
    labels_shuffled = labels[indices]

    # standardize
    scaler = StandardScaler()
    scaler.fit(dataset_shuffled) # calculate mean and std
    dataset_shuffled = scaler.transform(dataset_shuffled) # apply standardization to dataset

    svc = SVC(degree=10) # get svm
    scoring = 'accuracy' # accuracy metric for evaluation
    k = 10 # num of folds
    clf = GridSearchCV(svc, parameters, scoring=scoring, cv=k, verbose=True)
    clf.fit(dataset_shuffled, labels_shuffled)

    # calculate avg accuracy
    avg_acc = np.zeros((len(kernels) * len(Cs),)) # (len(kernels) * len(Cs),) = (36,) - 36 is the number of configurations tested
    for i in range(k):
        key = f"split{i}_test_score"
        split_accuracy_arr = clf.cv_results_[key]
        avg_acc += split_accuracy_arr
    avg_acc = avg_acc/k # we divide by k=10 for averaging all split accuracies
    avg_avg_acc += avg_acc

    # calculate avg mean score
    avg_mean_scores += clf.cv_results_["mean_test_score"]
    # calculate avg std score
    avg_std_scores += clf.cv_results_["std_test_score"]

avg_avg_acc = avg_avg_acc / 5
avg_mean_scores = avg_mean_scores / 5
avg_std_scores = avg_std_scores / 5
params = clf.cv_results_["params"]

result_dict = {
    "C": [config['C'] for config in params],
    "Kernel": [config['kernel'] for config in params],
    "Average Accuracy": avg_avg_acc,
    "Average Mean": avg_mean_scores,
    "Average STD": avg_std_scores
}

# create a dataframe
df = pd.DataFrame(result_dict)

# display the dataframe
print(df)

# convert df to HTML
html = df.to_html()

# write it to an HTML file
with open('dataframe.html', 'w') as f:
    f.write(html)