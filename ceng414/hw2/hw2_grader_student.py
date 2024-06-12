import traceback
from typing import Union

import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)
warnings.simplefilter(action='ignore', category=RuntimeWarning)

import pandas as pd
import numpy as np

from tqdm import tqdm
from sklearn.metrics import pairwise_distances

# TODO: Uncomment the following lines to import the functions from the student's submission
from hw2 import knn, fill_missing_features


def check_any_import_exist_except_pandas_and_numpy(path):
    with open(path, 'r') as file:
        lines = file.readlines()

    for line in lines:
        if 'import' in line and ('pandas' not in line and 'numpy' not in line):
            return False
        if 'import' in line and ';' in line and ('pandas' not in line or 'numpy' not in line):
            return False

    return True


def get_neighbors(X_train, y_train, sample, k, distance_method, weighted_voting=False, distance_threshold=None):
    distances = pairwise_distances(X_train, sample.reshape(1, -1), metric=distance_method).flatten()
    if distance_threshold is not None:
        # remove the samples that are further than the threshold
        mask = distances <= distance_threshold
        distances = distances[mask]
        y_train = y_train[mask]

    if weighted_voting:
        # Apply the custom weight formula
        weights = 1 / (distances ** 2 + 1e-10)  # Add a small constant to avoid division by zero
        sorted_indices = np.argsort(distances)[:k]
        nearest_labels = y_train.iloc[sorted_indices]
        nearest_weights = weights[sorted_indices]
        return nearest_labels, nearest_weights
    else:
        sorted_indices = np.argsort(distances)[:k]
        nearest_labels = y_train.iloc[sorted_indices]
        return nearest_labels, None


def weighted_majority_vote(nearest_labels, nearest_weights):
    unique_labels = np.unique(nearest_labels)
    weighted_votes = {label: 0 for label in unique_labels}
    for label, weight in zip(nearest_labels, nearest_weights):
        weighted_votes[label] += weight
    try:
        return max(weighted_votes, key=weighted_votes.get)
    except:
        raise ValueError("Distance threshold has problem")


def custom_knn(existing_data: pd.DataFrame, test_data: pd.DataFrame,
               k: int, distance_method: str, re_training: bool,
               distance_threshold: Union[float, None], weighted_voting: bool,
               remove_label_from_test: bool = True) -> list:
    # Extract features and labels in the first column
    X_train = existing_data.iloc[:, 1:]
    y_train = existing_data.iloc[:, 0]
    if remove_label_from_test:
        X_test = test_data.iloc[:, 1:]
    else:
        X_test = test_data

    predictions = []

    for i in range(X_test.shape[0]):
        sample = X_test.iloc[i, :].values
        nearest_labels, nearest_weights = get_neighbors(X_train, y_train, sample, k, distance_method,
                                                        weighted_voting, distance_threshold)

        if nearest_weights is not None:
            prediction = weighted_majority_vote(nearest_labels, nearest_weights)
        else:
            prediction = nearest_labels.value_counts().idxmax()

        distances = pairwise_distances(X_train, sample.reshape(1, -1), metric=distance_method).flatten()
        if distances[np.argsort(distances)[0]] <= (distance_threshold or np.inf):
            predictions.append(prediction)

            if re_training:
                # Add the sample to the training set, sample is an array, keep the columns in the same order
                X_train = pd.concat([X_train, pd.DataFrame([sample], columns=X_train.columns)])
                y_train = pd.concat([y_train, pd.Series([prediction])])
        else:
            predictions.append(None)

    return predictions


def generate_combinations(list_of_options: dict):
    # for distance method key use chebyshev only for 1 k, and distance threshold option
    option_set = []
    for k in list_of_options['k']:
        for distance_method in list_of_options['distance_method']:
            for re_training in list_of_options['re_training']:
                for distance_threshold in list_of_options['distance_threshold']:
                    for weighted_voting in list_of_options['weighted_voting']:
                        if (distance_method in ['chebyshev', 'manhattan'] and (
                                k != 3 or distance_threshold is not None or
                                weighted_voting is False or re_training is True)):
                            continue
                        if distance_threshold is not None and weighted_voting is False:
                            continue
                        option_set.append((k, distance_method, re_training, distance_threshold, weighted_voting))
    return option_set


def generate_combinations2(list_of_options: dict):
    # for distance method key use chebyshev only for 1 k, and distance threshold option
    option_set = []
    for k in list_of_options['k']:
        for distance_method in list_of_options['distance_method']:
            for distance_threshold in list_of_options['distance_threshold']:
                for weighted_voting in list_of_options['weighted_voting']:
                    if distance_threshold is not None and weighted_voting is False:
                        continue
                    if distance_method == 'manhattan' and distance_threshold is not None:
                        continue
                    option_set.append((k, distance_method, distance_threshold, weighted_voting))
    return option_set


def grading_part1():
    # read the data
    train = pd.read_csv('train_normalized_v2.csv')
    test1 = pd.read_csv('my_test_normalized.csv')

    # parameters sets
    existing_data = train
    test_data = test1
    k_values = [3, 7]
    distance_methods = ['euclidean', 'manhattan', 'chebyshev']
    re_training_values = [True, False]
    distance_threshold_values = [None, 8.82, 9.22]
    weighted_voting_values = [True, False]

    param_combinations = generate_combinations({'k': k_values, 'distance_method': distance_methods,
                                                're_training': re_training_values,
                                                'distance_threshold': distance_threshold_values,
                                                'weighted_voting': weighted_voting_values})
    # total try equals to the number of all possible combinations of parameters
    total_try = len(param_combinations)

    grade = 0
    user_feedback = []
    submission_fails = 0
    submission_none = 0
    for k, distance_method, re_training, distance_threshold, weighted_voting in tqdm(
            param_combinations, total=total_try, desc=f'Tests'):
        key = str((k, distance_method, re_training, distance_threshold, weighted_voting))
        try:
            test_data_copy = test_data.drop(columns='label')
            student = knn(existing_data, test_data_copy, k, distance_method, re_training, distance_threshold,
                          weighted_voting)
        except Exception as e:
            traceback.print_exc()
            submission_fails += 1
            if submission_fails == 3:
                user_feedback.append(f"Failed 3 times, skipping Exception:{e}")
                break
            continue

        if student is None:
            submission_none += 1
            if submission_none == 3:
                user_feedback.append(f"Got None output 3 times, skipping")
                break
            continue

        expected = custom_knn(existing_data, test_data, k, distance_method, re_training, distance_threshold,
                              weighted_voting)

        if isinstance(student, pd.Series):
            student = student.tolist()

        try:
            correct_guess = sum([1 for s, e in zip(student, expected) if s == e])
        except:
            user_feedback.append(f"Failed to compare outputs, student output is not an iterable of labels")
            continue
        if correct_guess != len(expected):
            user_feedback.append(f"Parameters: {key}, correct guesses: {correct_guess}/{len(expected)}")
        grade += (50/total_try)*(correct_guess/len(expected))

    # add row to grading csv
    if grade == 50:
        user_feedback.append("All tests passed")

    print("Grade:", grade)


def grading_part2():
    # read the data
    train = pd.read_csv('train_normalized_v2.csv')
    test1 = pd.read_csv('my_test_with_missing_normalized.csv')

    # parameters sets
    existing_data = train
    test_data = test1.copy(deep=True)
    k_values = [3, 7]
    distance_methods = ['euclidean', 'manhattan']
    distance_threshold_values = [None, 13.9, 14.1]
    weighted_voting_values = [True, False]

    param_combinations = generate_combinations2({'k': k_values, 'distance_method': distance_methods,
                                                'distance_threshold': distance_threshold_values,
                                                'weighted_voting': weighted_voting_values})
    # total try equals to the number of all possible combinations of parameters
    total_try = len(param_combinations)

    grade = 0
    user_feedback = []
    submission_fails = 0
    submission_none = 0
    for k, distance_method, distance_threshold, weighted_voting in tqdm(
            param_combinations, total=total_try, desc=f'Tests'):
        key = str((k, distance_method, distance_threshold, weighted_voting))

        try:
            student = fill_missing_features(existing_data, test_data, k, distance_method, distance_threshold,
                                            weighted_voting)
        except Exception as e:
            submission_fails += 1
            if submission_fails == 3:
                user_feedback.append(f"Failed 3 times, skipping Exception:{e}")
                break
            continue

        if student is None:
            submission_none += 1
            if submission_none == 3:
                user_feedback.append(f"Got None output 3 times, skipping")
                break
            continue

        if not isinstance(student, pd.DataFrame):
            user_feedback.append(f"Output is not a DataFrame")
            break

        if 'label' in student.columns:
            student = student.drop(columns='label')

        if not student.shape[0] == test1.shape[0] and student.shape[1] == test1.shape[1] - 1:
            user_feedback.append(f"Output shape is not correct")
            break

        if student.isnull().sum().sum() > 0:
            user_feedback.append(f"Output has missing values")
            continue

        try:
            expected = custom_knn(existing_data, student, k, distance_method, False, distance_threshold,
                                  weighted_voting, False)
        except Exception as e:
            user_feedback.append(f"Failed to run custom_knn function: {e}")
            continue
        # if expected is equal to test data labels, then the output is correct
        real_labels = test1.iloc[:, 0]

        # count correct guesses
        correct_guess = sum([1 for s, e in zip(real_labels, expected) if s == e])
        if correct_guess < len(real_labels) - 6:  # accept 6 wrong guesses
            user_feedback.append(f"Parameters: {key}, correct guesses: {correct_guess}/{len(expected)}")
        else:
            correct_guess = len(real_labels)
        grade += (50 / total_try) * (correct_guess / len(expected))

    # add row to grading csv
    if grade == 50:
        user_feedback.append("All tests passed")

    print("Grade:", grade)


grading_part1()
grading_part2()
