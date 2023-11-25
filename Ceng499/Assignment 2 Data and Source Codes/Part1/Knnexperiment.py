import pickle
from Distance import Distance
from Knn import KNN
import numpy as np
from sklearn.model_selection import StratifiedKFold
from scipy import stats

dataset, labels = pickle.load(open("../data/part1_dataset.data", "rb"))

# Hyperparameter values to test
K = [3, 4, 5]
distance_metrics = [
    "cos",
    "minkowski",
    "mahalanobis",
]

accuracies = [] # record mean accuracy for each hyperparameter configuration
for k in K: # k = 3, 4, 5
    for distance_func in distance_metrics: # distance = cos, minkowski, mahalanobis
        accuracy_per_experiment = []
        
        for _ in range(5): # repeat the experiment 5 times
            # shuffle the dataset
            indices = np.arange(dataset.shape[0])
            np.random.shuffle(indices)
            dataset_shuffled = dataset[indices]
            labels_shuffled = labels[indices]
            
            # do stratified 10-fold cross validation
            skf = StratifiedKFold(n_splits=10, shuffle=True, random_state=42)
            
            for fold, (train_index, test_index) in enumerate(skf.split(dataset, labels)):
                # get training split
                X_train, y_train = dataset_shuffled[train_index], labels_shuffled[train_index]
                # get test split
                X_test, y_test = dataset_shuffled[test_index], labels_shuffled[test_index]

                model = None
                # initialize model based on distance metric
                if distance_func == "cos":
                    model = KNN(
                        dataset=X_train,
                        data_label=y_train,
                        similarity_function=Distance.calculateCosineDistance,
                        similarity_function_parameters=None,
                        K=k,
                    )
                elif distance_func == "minkowski":
                    model = KNN(
                        dataset=X_train,
                        data_label=y_train,
                        similarity_function=Distance.calculateMinkowskiDistance,
                        similarity_function_parameters=2,
                        K=k,
                    )
                else:
                    cov_matrix = np.cov(X_train, rowvar=False)
                    inv_matrix = np.linalg.inv(cov_matrix)
                    model = KNN(
                        dataset=X_train,
                        data_label=y_train,
                        similarity_function=Distance.calculateMahalanobisDistance,
                        similarity_function_parameters=inv_matrix,
                        K=k,
                    )

                # predict instances from test dataset
                predictions = [model.predict(instance) for instance in X_test]
                # calculate mean accuracy
                accuracy = np.mean([pred == label for pred, label in zip(predictions, y_test)])
                # record the accuracy for this experiment
                accuracy_per_experiment.append(accuracy)
        
        accuracies.append(
            {
                "K": k,
                "distance_metric": distance_func,
                "mean_accuracy": np.mean(accuracy_per_experiment),
                "accuracy_per_experiment": accuracy_per_experiment,
            }
        )

confidence_level = 0.95
degrees_freedom = (5*10) - 1 # (number of experiments * number of folds) - 1
t_value = stats.t.ppf((1 + confidence_level) / 2, degrees_freedom)

conf_intervals = [] # records the configurations, their mean accuracies and confidence intervals
for result in accuracies:
    mean_accuracy = result["mean_accuracy"]
    sd = np.std(result["accuracy_per_experiment"])  # standard deviation
    sem = sd / np.sqrt(5) # standard error of mean
    
    conf_interval = t_value * sem
    conf_intervals.append({
        "K": result["K"],
        "distance_metric": result["distance_metric"],
        "mean_accuracy": f"{mean_accuracy:.2f}",
        "confidence_interval": (f"{(mean_accuracy - conf_interval):.2f}", f"{(mean_accuracy + conf_interval):.2f}")
    })

for result in conf_intervals:
    for key, value in result.items():
        print(f"{key}: {value}")