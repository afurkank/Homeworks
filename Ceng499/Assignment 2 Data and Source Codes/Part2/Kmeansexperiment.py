import matplotlib.pyplot as plt
import numpy as np
from scipy import stats
from Kmeans import KMeans
import pickle

dataset1 = pickle.load(open("../data/part2_dataset_1.data", "rb"))

dataset2 = pickle.load(open("../data/part2_dataset_2.data", "rb"))

K_values = [2, 3, 4, 5, 6, 7, 8, 9, 10]

dataset1_losses = []
for K in K_values:
    alphas = []
    for _ in range(10):
        losses = []
        for _ in range(10):
            kmeans = KMeans(dataset1, K=K)
            _, _, loss = kmeans.run()
            losses.append(loss)
        smallest_loss_idx = np.argmin(losses)
        alphas.append(losses[smallest_loss_idx])
    avg_loss = np.mean(alphas)
    dataset1_losses.append({"K": K, "loss": alphas})
    print(f"Dataset1 - K = {K}, Loss = {avg_loss}")

dataset2_losses = []
for K in K_values:
    alphas = []
    for _ in range(10):
        losses = []
        for _ in range(10):
            kmeans = KMeans(dataset2, K=K)
            _, _, loss = kmeans.run()
            losses.append(loss)
        smallest_loss_idx = np.argmin(losses)
        alphas.append(losses[smallest_loss_idx])
    avg_loss = np.mean(alphas)
    dataset2_losses.append({"K": K, "loss": alphas})
    print(f"Dataset2 - K = {K}, Loss = {avg_loss}")

# this function calculates the confidence interval for given data
def confidence_interval(data, confidence=0.98):
    n = len(data)
    mean = np.mean(data)
    sem = stats.sem(data)
    margin = sem * stats.t.ppf((1 + confidence) / 2., n-1)
    return mean, margin

# this function gets K values, mean losses and confidence intervals
def get_k_loss_conf(data):
    ks = []
    mean_losses = []
    conf_intervals = []
    for item in data:
        K = item['K']
        losses = item['loss']
        mean_loss, margin = confidence_interval(losses)
        ks.append(K)
        mean_losses.append(mean_loss)
        conf_intervals.append(margin)
    return ks, mean_losses, conf_intervals

# get k values, mean losses and confidence intervals
k_values_dataset1, mean_losses_dataset1, ci_dataset1 = get_k_loss_conf(dataset1_losses)
k_values_dataset2, mean_losses_dataset2, ci_dataset2 = get_k_loss_conf(dataset2_losses)

# print confidence intervals with table format
print("Dataset 1 Confidence Intervals")
print("K\tMean Loss\tConfidence Interval")
for K, loss, ci in zip(k_values_dataset1, mean_losses_dataset1, ci_dataset1):
    print(f"{K}\t{loss:.4f}\t±{ci:.4f}")

print("\nDataset 2 Confidence Intervals")
print("K\tMean Loss\tConfidence Interval")
for K, loss, ci in zip(k_values_dataset2, mean_losses_dataset2, ci_dataset2):
    print(f"{K}\t{loss:.4f}\t±{ci:.4f}")

# plot K-Loss graph with confidence interval for dataset1
plt.figure(figsize=(8, 6))
plt.errorbar(k_values_dataset1, mean_losses_dataset1, yerr=ci_dataset1, fmt='o', capsize=5, label='Confidence Interval')
plt.plot(k_values_dataset1, mean_losses_dataset1, label='Mean Loss', linestyle='-', marker='o')
plt.title('Dataset 1 Losses with Confidence Intervals')
plt.xlabel('K Value')
plt.ylabel('Loss')
plt.legend()
plt.grid(True)
plt.savefig('KMeans_Dataset1_K-Loss_Graph.png')
plt.show()
plt.close()

# plot K-Loss graph with confidence interval for dataset2
plt.figure(figsize=(8, 6))
plt.errorbar(k_values_dataset2, mean_losses_dataset2, yerr=ci_dataset2, fmt='o', capsize=5, label='Confidence Interval')
plt.plot(k_values_dataset2, mean_losses_dataset2, label='Mean Loss', linestyle='-', marker='o')
plt.title('Dataset 2 Losses with Confidence Intervals')
plt.xlabel('K Value')
plt.ylabel('Loss')
plt.legend()
plt.grid(True)
plt.savefig('KMeans_Dataset2_K-Loss_Graph.png')
plt.show()
plt.close()