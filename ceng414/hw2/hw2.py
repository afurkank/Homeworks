import numpy as np
import pandas as pd

def knn(
        existing_data: pd.DataFrame,
        test_data: pd.DataFrame,
        k: int,
        distance_method: str,
        re_training: bool,
        distance_threshold: float,
        weighted_voting: bool
    ):

    assert k >= 3 and k <= 10, "k must be within range [3, 10]"
    assert distance_method in ["euclidean", "manhattan", "chebyshev"], "distance method is not recognized"

    train_labels = existing_data['label'].to_numpy()
    train_values = existing_data.drop(columns=['label']).to_numpy()
    
    test_values = test_data.to_numpy() # test_data.drop(columns=['label']).to_numpy()
    
    predicted_labels = []

    for test_item in test_values:
        distances = None
        if distance_method == 'euclidean':
            distances = np.sqrt(np.sum((test_item - train_values)**2, axis=1))
        elif distance_method == 'manhattan':
            distances = np.sum(np.abs(test_item - train_values), axis=1)
        else: # 'chebyshev'
            distances = np.max(np.abs(test_item - train_values), axis=1)

        # distance threshold
        if distance_threshold is not None:
            distances[distances > distance_threshold] = np.inf

        sorted_indices = np.argsort(distances)
        truncated_indices = sorted_indices[:k]

        closest_labels = train_labels[truncated_indices]
        closest_distances = distances[truncated_indices]

        if weighted_voting:
            # weight labels with inverse of distance
            weights = 1 / (closest_distances + 1e-5)  # add small constant for the division by zero case
            label_weights = np.zeros(len(np.unique(train_labels)))

            for label, weight in zip(closest_labels, weights):
                label_weights[label] += weight

            predicted_label = np.argmax(label_weights)
        else: # majority voting
            label_counts = np.bincount(closest_labels)
            predicted_label = np.argmax(label_counts)

        predicted_labels.append(predicted_label)

        # update training set with test sample and its label
        if re_training:
            train_values = np.vstack([train_values, test_item])
            train_labels = np.append(train_labels, predicted_label)

    return pd.Series(predicted_labels)

def fill_missing_features(
        existing_data: pd.DataFrame,
        test_data: pd.DataFrame,
        k: int,
        distance_method: str,
        distance_threshold: float,
        weighted_voting: bool
    ):

    assert k >= 3 and k <= 10, "k must be within range [3, 10]"
    assert distance_method in ["euclidean", "manhattan", "chebyshev"], "distance method is not recognized"

    # divide training data into two groups based on label
    # these groups will be used for deciding on which data points to consider
    # for calculation of the missing feature. because we cannot consider data points
    # that do not belong to the same label group
    group_0 = existing_data[existing_data['label'] == 0].drop(columns=['label']).to_numpy()
    group_1 = existing_data[existing_data['label'] == 1].drop(columns=['label']).to_numpy()

    # prepare test data
    test_labels = test_data['label'].to_numpy()
    test_values = test_data.drop(columns=['label']).to_numpy()
    missing_indices = np.isnan(test_values)

    # func to calculate distance
    def calculate_distance(x, y):
        #print(f"x:\n{x}")
        #print(f"y:\n{y}")
        mask = ~np.isnan(x)  # mask non-NaN values
        if distance_method == 'euclidean':
            return np.sqrt(np.sum((x[mask] - y[mask]) ** 2))
        elif distance_method == 'manhattan':
            return np.sum(np.abs(x[mask] - y[mask]))
        else:# 'chebyshev'
            return np.max(np.abs(x[mask] - y[mask]))

    # loop through each sample with missing features
    for i, (test_label, test_value) in enumerate(zip(test_labels, test_values)):
        #print(f"test_label:{test_label}")
        #print(f"test_value:{test_value}")
        if np.isnan(test_value).any():
            # select the group corresponding to the test sample's label
            group = group_0 if test_label == 0 else group_1

            # calculate distances to all samples in the selected group
            distances = np.array([calculate_distance(test_value, group_sample) for group_sample in group])
            #print(f"distances before threshold:\n{distances}")
            # apply distance threshold
            if distance_threshold is not None:
                distances[distances > distance_threshold] = np.inf
            #print(f"distances after threshold:\n{distances}")
            # get the indices of the k nearest data points
            nearest_indices = np.argsort(distances)[:k]
            #print(f"nearest_indices:{nearest_indices}")
            # calculate the missing feature
            if weighted_voting:
                # weighted average of feature values of k nearest data points
                weights = 1 / np.maximum(distances[nearest_indices], 1e-5)
                for missing_index in np.where(missing_indices[i])[0]:
                    weighted_average_val = np.average(group[nearest_indices, missing_index], weights=weights)
                    #print(weighted_average_val)
                    test_values[i, missing_index] = weighted_average_val
            else:
                # simple average of the k nearest neighbors' feature values
                for missing_index in np.where(missing_indices[i])[0]:
                    average_val = np.mean(group[nearest_indices, missing_index])
                    #print(average_val)
                    test_values[i, missing_index] = average_val

    return pd.DataFrame(test_values, columns=test_data.columns.drop('label'))

""" if __name__ == '__main__':
    df_train = pd.read_csv("train.csv")
    #print(f"Train dataframe: {df_train}")
    df_test = pd.read_csv("test.csv")
    #print(f"Test dataframe: {df_test}")

    labels = knn(df_train, df_test, 3, "euclidean", True, np.inf, True)
    #print(labels)
    
    df_test_missing = pd.read_csv("test_with_missing.csv")

    completed_values = fill_missing_features(df_train, df_test_missing, 3, "euclidean", np.inf, True)
    print(completed_values)
    
    # check if there is any nan value left
    print(np.isnan(completed_values).any()) """