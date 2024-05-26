import numpy as np

def pagerank(graph, p=0.5, max_iter=100, tol=1e-6):
    N = len(graph)
    A = np.array(graph, dtype=float)
    
    # Initialize importance scores to 1
    R = np.ones(N)
    
    # Calculate out-degree for each node
    out_degree = np.sum(A, axis=0)
    print(f"out_degree:\n{out_degree}")

    # Build transition matrix M
    M = np.zeros_like(A)
    for i in range(N):
        if out_degree[i] > 0:
            M[:, i] = A[:, i] / out_degree[i]
    print(f"Transition matrix:\n{M}")
    
    print(f"Iteration: {0} / Values: {R}")
    # Apply the PageRank formula iteratively
    for i in range(max_iter):
        R_new = p + (1 - p) * np.dot(M, R)
        if np.linalg.norm(R_new - R, ord=1) < tol:
            break
        R = R_new
        print(f"Iteration: {i+1} / Values: {R}")
    
    return R

# Example usage:
graph = [
    [0, 1, 1],
    [0, 0, 1],
    [1, 0, 0]
]

importance_scores = pagerank(graph)
print("Node importances:", importance_scores)