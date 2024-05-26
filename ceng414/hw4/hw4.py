import numpy as np
import scipy.stats as stats

def grubs_test(data):
    # Step 1: Calculate the mean and standard deviation of the data
    mean = np.mean(data)
    std_dev = np.std(data, ddof=1)  # Use ddof=1 for sample standard deviation
    print(std_dev)
    
    # Step 2: Calculate Grubbs' test statistic for the highest absolute deviation
    G = np.max(np.abs(data - mean)) / std_dev
    
    # Step 3: Calculate the critical value from the Grubbs' test table
    N = len(data)
    t_critical = stats.t.ppf(1 - 0.05/N, N - 2)
    print(t_critical)
    G_critical = ((N - 1) / np.sqrt(N)) * np.sqrt(t_critical**2 / (N - 2 + t_critical**2))
    
    # Step 4: Determine if the calculated G exceeds the critical value
    if G > G_critical:
        return G, G_critical, True  # Outlier detected
    else:
        return G, G_critical, False  # No outlier detected

# Example usage
#data = [180, 183, 127, 224, 183, 207, 190, 140, 203, 73]  # Example list of 10 numbers
#data = [145,125, 190, 135, 220, 130, 210, 3, 165, 165, 150]
data = [248, 162, 246, 165, 211, 163, 204, 198, 131, 51]
G, G_critical, is_outlier = grubs_test(data)

print(f"G: {G}")
print(f"G_critical: {G_critical}")
print(f"Outlier detected: {is_outlier}")