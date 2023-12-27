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
dataset, labels = DataLoader.load_credit_with_onehot(data_path)

""" All hyperparameter configs """
# knn configs
n_neighbors           = [3, 5, 7, 9]
# svc configs
Cs                    = [1, 10]
gammas                = ['scale', 'auto']
kernels               = ['rbf', 'linear']
# decision tree configs
max_depths_dt         = [5, 7, 9]
min_samples_splits_dt = [4, 6]
# random forest configs
n_estimators          = [50, 100]
max_depths_rf         = [5, 7]
min_samples_splits_rf = [4, 6]

# outer loop
rskf_outer = RepeatedStratifiedKFold(n_splits=3, n_repeats=5, random_state=42) # 3 splits, 5 repetitions

# Define scoring metrics
scoring = ('accuracy', 'f1')

# Define parameter grids for each algorithm
param_grid_knn = {'n_neighbors': n_neighbors}
param_grid_svc = {'C': Cs, 'gamma': gammas, 'kernel': kernels}
param_grid_dt  = {'max_depth': max_depths_dt, 'min_samples_split': min_samples_splits_dt}
param_grid_rf  = {'n_estimators': n_estimators, 'max_depth': max_depths_rf, 'min_samples_split': min_samples_splits_rf}

# lists for holding average acc and f1 over outer splits - this is overall average
avg_avg_acc_knn = np.zeros((4,)) # 4 configs are tested for knn
avg_avg_f1_knn  = np.zeros((4,)) # same as above
avg_avg_acc_svc = np.zeros((8,)) # 8 configs are tested for svc
avg_avg_f1_svc  = np.zeros((8,)) # same as above
avg_avg_acc_dt  = np.zeros((6,)) # 6 configs are tested for dt
avg_avg_f1_dt   = np.zeros((6,)) # same as above
avg_avg_acc_rf  = np.zeros((8,)) # 8 configs are tested for rf
avg_avg_f1_rf   = np.zeros((8,)) # same as above

# same as above but for means and stds
avg_mean_acc_knn = np.zeros((4,)) # 4 configs are tested for knn
avg_mean_f1_knn  = np.zeros((4,)) # same as above
avg_std_acc_knn  = np.zeros((4,)) # same as above
avg_std_f1_knn   = np.zeros((4,)) # same as above
avg_mean_acc_svc = np.zeros((8,)) # 8 configs are tested for svc
avg_mean_f1_svc  = np.zeros((8,)) # same as above
avg_std_acc_svc  = np.zeros((8,)) # same as above
avg_std_f1_svc   = np.zeros((8,)) # same as above
avg_mean_acc_dt  = np.zeros((6,)) # 6 configs are tested for dt
avg_mean_f1_dt   = np.zeros((6,)) # same as above
avg_std_acc_dt   = np.zeros((6,)) # same as above
avg_std_f1_dt    = np.zeros((6,)) # same as above
avg_mean_acc_rf  = np.zeros((8,)) # 8 configs are tested for rf
avg_mean_f1_rf   = np.zeros((8,)) # same as above
avg_std_acc_rf   = np.zeros((8,)) # same as above
avg_std_f1_rf    = np.zeros((8,)) # same as above

# for best score calculation
avg_knn_score = 0
avg_svc_score = 0
avg_dt_score  = 0
avg_rf_score  = 0

# keep the best models for later use
best_knn_model = None
best_svc_model = None
best_dt_model  = None
best_rf_model  = None

# keep best scores
best_knn_score = 0
best_svc_score = 0
best_dt_score  = 0
best_rf_score  = 0

for i, (train_index_outer, test_index_outer) in enumerate(rskf_outer.split(dataset, labels)):
    # get training split
    X_train_outer, y_train_outer = dataset[train_index_outer], labels[train_index_outer]
    # get test split
    X_test_outer, y_test_outer   = dataset[test_index_outer], labels[test_index_outer]

    # standardize
    scaler        = StandardScaler()
    scaler.fit(X_train_outer) # calculate mean and std
    X_train_outer = scaler.transform(X_train_outer) # apply standardization
    X_test_outer  = scaler.transform(X_test_outer)
    
    # inner loop
    rskf_inner = RepeatedStratifiedKFold(n_splits=5, n_repeats=5, random_state=42) # 5 splits, 5 repetitions

    # perform grid search for each algorithm
    grid_search_knn = GridSearchCV(KNeighborsClassifier(), param_grid_knn, scoring=scoring, refit='accuracy', cv=rskf_inner, verbose=True).fit(X_train_outer, y_train_outer)
    grid_search_svc = GridSearchCV(SVC(), param_grid_svc, scoring=scoring, refit='accuracy', cv=rskf_inner, verbose=True).fit(X_train_outer, y_train_outer)
    grid_search_dt  = GridSearchCV(DecisionTreeClassifier(), param_grid_dt, scoring=scoring, refit='accuracy', cv=rskf_inner, verbose=True).fit(X_train_outer, y_train_outer)
    grid_search_rf  = GridSearchCV(RandomForestClassifier(), param_grid_rf, scoring=scoring, refit='accuracy', cv=rskf_inner, verbose=True).fit(X_train_outer, y_train_outer)

    avg_acc_knn = np.zeros((4,)) # 4 configs are tested for knn
    avg_f1_knn  = np.zeros((4,)) # same as above
    avg_acc_svc = np.zeros((8,)) # 8 configs are tested for svc
    avg_f1_svc  = np.zeros((8,)) # same as above
    avg_acc_dt  = np.zeros((6,)) # 6 configs are tested for dt
    avg_f1_dt   = np.zeros((6,)) # same as above
    avg_acc_rf  = np.zeros((8,)) # 8 configs are tested for rf
    avg_f1_rf   = np.zeros((8,)) # same as above

    # average accuracy and f1 over all inner splits
    for j in range(25): # 5 splits * 5 repetitions
        key_acc = f"split{j}_test_accuracy"
        key_f1  = f"split{j}_test_f1"

        avg_acc_knn += grid_search_knn.cv_results_[key_acc]
        avg_f1_knn  += grid_search_knn.cv_results_[key_f1]

        avg_acc_svc += grid_search_svc.cv_results_[key_acc]
        avg_f1_svc  += grid_search_svc.cv_results_[key_f1]

        avg_acc_dt  += grid_search_dt.cv_results_[key_acc]
        avg_f1_dt   += grid_search_dt.cv_results_[key_f1]

        avg_acc_rf  += grid_search_rf.cv_results_[key_acc]
        avg_f1_rf   += grid_search_rf.cv_results_[key_f1]

    # calculate average
    avg_acc_knn /= 25
    avg_f1_knn  /= 25
    avg_acc_svc /= 25
    avg_f1_svc  /= 25
    avg_acc_dt  /= 25
    avg_f1_dt   /= 25
    avg_acc_rf  /= 25
    avg_f1_rf   /= 25

    # add inner average to overall average
    avg_avg_acc_knn += avg_acc_knn
    avg_avg_f1_knn  += avg_f1_knn
    avg_avg_acc_svc += avg_acc_svc
    avg_avg_f1_svc  += avg_f1_svc
    avg_avg_acc_dt  += avg_acc_dt
    avg_avg_f1_dt   += avg_f1_dt
    avg_avg_acc_rf  += avg_acc_rf
    avg_avg_f1_rf   += avg_f1_rf

    # add inner mean and std to overall average
    avg_mean_acc_knn += grid_search_knn.cv_results_["mean_test_accuracy"]
    avg_mean_f1_knn  += grid_search_knn.cv_results_["mean_test_f1"]
    avg_std_acc_knn  += grid_search_knn.cv_results_["std_test_accuracy"]
    avg_std_f1_knn   += grid_search_knn.cv_results_["std_test_f1"]
    avg_mean_acc_svc += grid_search_svc.cv_results_["mean_test_accuracy"]
    avg_mean_f1_svc  += grid_search_svc.cv_results_["mean_test_f1"]
    avg_std_acc_svc  += grid_search_svc.cv_results_["std_test_accuracy"]
    avg_std_f1_svc   += grid_search_svc.cv_results_["std_test_f1"]
    avg_mean_acc_dt  += grid_search_dt.cv_results_["mean_test_accuracy"]
    avg_mean_f1_dt   += grid_search_dt.cv_results_["mean_test_f1"]
    avg_std_acc_dt   += grid_search_dt.cv_results_["std_test_accuracy"]
    avg_std_f1_dt    += grid_search_dt.cv_results_["std_test_f1"]
    avg_mean_acc_rf  += grid_search_rf.cv_results_["mean_test_accuracy"]
    avg_mean_f1_rf   += grid_search_rf.cv_results_["mean_test_f1"]
    avg_std_acc_rf   += grid_search_rf.cv_results_["std_test_accuracy"]
    avg_std_f1_rf    += grid_search_rf.cv_results_["std_test_f1"]

    # get the best model for all algorithms
    best_knn = grid_search_knn.best_estimator_
    best_svc = grid_search_svc.best_estimator_
    best_dt  = grid_search_dt.best_estimator_
    best_rf  = grid_search_rf.best_estimator_

    # calculate scores from best models
    knn_score = best_knn.score(X_test_outer, y_test_outer)
    svc_score = best_svc.score(X_test_outer, y_test_outer)
    dt_score  = best_dt.score(X_test_outer, y_test_outer)
    rf_score  = best_rf.score(X_test_outer, y_test_outer)

    # add scores to avg score
    avg_knn_score += knn_score
    avg_svc_score += svc_score
    avg_dt_score  += dt_score
    avg_rf_score  += rf_score

    # update best scores and models
    if knn_score > best_knn_score:
        best_knn_score = knn_score
        best_knn_model = best_knn
    if svc_score > best_svc_score:
        best_svc_score = svc_score
        best_svc_model = best_svc
    if dt_score > best_dt_score:
        best_dt_score = dt_score
        best_dt_model = best_dt
    if rf_score > best_rf_score:
        best_rf_score = rf_score
        best_rf_model = best_rf

    print(f"------------   Outer loop {i+1}/{15} is completed   ------------")

# calculate average
# 3 splits * 5 repetitions
avg_avg_acc_knn /= 15
avg_avg_f1_knn  /= 15
avg_avg_acc_svc /= 15
avg_avg_f1_svc  /= 15
avg_avg_acc_dt  /= 15
avg_avg_f1_dt   /= 15
avg_avg_acc_rf  /= 15
avg_avg_f1_rf   /= 15

# calculate average
avg_mean_acc_knn /= 15
avg_mean_f1_knn  /= 15
avg_std_acc_knn  /= 15
avg_std_f1_knn   /= 15
avg_mean_acc_svc /= 15
avg_mean_f1_svc  /= 15
avg_std_acc_svc  /= 15
avg_std_f1_svc   /= 15
avg_mean_acc_dt  /= 15
avg_mean_f1_dt   /= 15
avg_std_acc_dt   /= 15
avg_std_f1_dt    /= 15
avg_mean_acc_rf  /= 15
avg_mean_f1_rf   /= 15
avg_std_acc_rf   /= 15
avg_std_f1_rf    /= 15

# average best scores
knn_score /= 15
svc_score /= 15
dt_score  /= 15
rf_score  /= 15

# get params
params_knn = grid_search_knn.cv_results_["params"]
params_svc = grid_search_svc.cv_results_["params"]
params_dt  = grid_search_dt.cv_results_["params"]
params_rf  = grid_search_rf.cv_results_["params"]

# create dictionaries to convert them to dataframes
result_dict_knn = {
    "K": [config['n_neighbors'] for config in params_knn],
    "Average Accuracy": avg_avg_acc_knn,
    "Average F1": avg_avg_f1_knn,
    "Average Mean Accuracy": avg_mean_acc_knn,
    "Average Mean F1": avg_mean_f1_knn,
    "Average STD Accuracy": avg_std_acc_knn,
    "Average STD F1": avg_std_f1_knn
}

df_knn = pd.DataFrame(result_dict_knn)

result_dict_svc = {
    "C": [config['C'] for config in params_svc],
    "Gamma": [config['gamma'] for config in params_svc],
    "Kernel": [config['kernel'] for config in params_svc],
    "Average Accuracy": avg_avg_acc_svc,
    "Average F1": avg_avg_f1_svc,
    "Average Mean Accuracy": avg_mean_acc_svc,
    "Average Mean F1": avg_mean_f1_svc,
    "Average STD Accuracy": avg_std_acc_svc,
    "Average STD F1": avg_std_f1_svc
}

df_svc = pd.DataFrame(result_dict_svc)

result_dict_dt = {
    "Max Depth": [config['max_depth'] for config in params_dt],
    "Min Samples Split": [config['min_samples_split'] for config in params_dt],
    "Average Accuracy": avg_avg_acc_dt,
    "Average F1": avg_avg_f1_dt,
    "Average Mean Accuracy": avg_mean_acc_dt,
    "Average Mean F1": avg_mean_f1_dt,
    "Average STD Accuracy": avg_std_acc_dt,
    "Average STD F1": avg_std_f1_dt
}

df_dt = pd.DataFrame(result_dict_dt)

result_dict_rf = {
    'N Estimators': [config['n_estimators'] for config in params_rf],
    "Max Depth": [config['max_depth'] for config in params_rf],
    "Min Samples Split": [config['min_samples_split'] for config in params_rf],
    "Average Accuracy": avg_avg_acc_rf,
    "Average F1": avg_avg_f1_rf,
    "Average Mean Accuracy": avg_mean_acc_rf,
    "Average Mean F1": avg_mean_f1_rf,
    "Average STD Accuracy": avg_std_acc_rf,
    "Average STD F1": avg_std_f1_rf
}

df_rf = pd.DataFrame(result_dict_rf)

result_dict_best = {
    "Algorithm": ["KNN", "SVC", "Decision Tree", "Random Forest"],
    "Accuracy": [best_knn_score, best_svc_score, best_dt_score, best_rf_score],
}

df_best = pd.DataFrame(result_dict_best)

# function to calculate the confidence interval
from scipy import stats
def calculate_confidence_interval(mean, std_dev, sample_size, z_score=stats.norm.ppf(0.975)):
    margin_of_error = z_score * (std_dev / np.sqrt(sample_size))
    lower_bound = round(mean - margin_of_error, 2)
    upper_bound = round(mean + margin_of_error, 2)
    return lower_bound, upper_bound

# function to apply the CI calculation to a dictionary of results
def add_confidence_intervals_to_dict(result_dict, sample_size):
    for metric in ["Accuracy", "F1"]:
        avg_metric = f"Average {metric}"
        std_metric = f"Average STD {metric}"
        ci_metric  = f"CI {metric}"
        # calculate confidence intervals for each configuration
        result_dict[ci_metric] = [calculate_confidence_interval(mean, std, sample_size)
                                  for mean, std in zip(result_dict[avg_metric], result_dict[std_metric])]
    return result_dict

# apply the function to the result dictionary
result_dict_knn = add_confidence_intervals_to_dict(result_dict_knn, sample_size=25)
df_knn = pd.DataFrame(result_dict_knn)

result_dict_svc = add_confidence_intervals_to_dict(result_dict_svc, sample_size=25)
df_svc = pd.DataFrame(result_dict_svc)

result_dict_dt = add_confidence_intervals_to_dict(result_dict_dt, sample_size=25)
df_dt  = pd.DataFrame(result_dict_dt)

result_dict_rf = add_confidence_intervals_to_dict(result_dict_rf, sample_size=25)
df_rf  = pd.DataFrame(result_dict_rf)

# convert dataframe to HTML
html_knn  = df_knn.to_html()
html_svc  = df_svc.to_html()
html_dt   = df_dt.to_html()
html_rf   = df_rf.to_html()
html_best = df_best.to_html()

# print dataframes
print(f"###   KNN TABLE   ###{df_knn}")
print(f"###   SVC TABLE   ###{df_svc}")
print(f"###   DT  TABLE   ###{df_dt}")
print(f"###   RF  TABLE   ###{df_rf}")
print(f"###   BEST SCORES TABLE   ###{df_best}")

# write it to an HTML file
with open('dataframe_knn.html', 'w') as f:
    f.write(html_knn)
with open('dataframe_svc.html', 'w') as f:
    f.write(html_svc)
with open('dataframe_dt.html', 'w') as f:
    f.write(html_dt)
with open('dataframe_rf.html', 'w') as f:
    f.write(html_rf)
with open('dataframe_all.html', 'w') as f:
    f.write(html_best)

""" ##############   TRAINING DECISION TREE CLASSIFIER      ############## """

print("###   TRAIING THE DECISION TREE CLASSIFIER   ###")

# load data without one-hot encoding
dataset_, labels_ = DataLoader.load_credit(data_path)

# standardize
scaler        = StandardScaler()
scaler.fit_transform(dataset_)

dt_classifier = best_dt_model # get the best model

# train the decision tree
dt_classifier.fit(dataset_, labels_)

# print the importance of features
feature_importance_arr = dt_classifier.feature_importances_
print(f"\nThe importance of features for decision tree are:")
importance_dict = {f"attr_{i}": f"{importance:.2f}" for i,importance in enumerate(feature_importance_arr)}
print(importance_dict)

""" ##############   TRAINING SUPPORT VECTOR CLASSIFIER   ##############  """

print("###   TRAIING THE SUPPORT VECTOR CLASSIFIER   ###")

# load data without one-hot encoding
dataset, labels = DataLoader.load_credit_with_onehot(data_path)

# standardize
scaler        = StandardScaler()
scaler.fit_transform(dataset)

sv_classifier = best_svc_model # get the best model

# train the svc
sv_classifier.fit(dataset, labels) # use one-hot encoded data

print(f"\nThe support vectors are:\n{sv_classifier.support_vectors_}")