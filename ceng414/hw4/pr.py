import numpy as np

def pagerank(graph, p=0.5, tol=1.0e-8, max_iter=10):
    N = len(graph)
    M = np.zeros((N, N))
    
    # Create transition matrix
    for i in range(N):
        out_links = sum(graph[i])
        if out_links > 0:
            M[i] = graph[i] / out_links
    
    # Initialize PageRank values
    PR = np.ones(N)
    
    # Iteratively calculate PageRank
    for _ in range(max_iter):
        new_PR = p + (1-p) * np.dot(M.T, PR)
        if np.linalg.norm(new_PR - PR, 1) < tol:
            break
        PR = new_PR
    
    return PR

# Example graph with 5 nodes and 10 edges represented as an adjacency matrix
graph = np.array([
    [0, 1, 1, 0, 0],  # Node 0 has edges to Node 1, Node 2, and Node 4
    [1, 0, 1, 1, 0],  # Node 1 has edges to Node 0, Node 2, and Node 4
    [0, 0, 0, 0, 0],  # Node 2 has edges to Node 0, Node 1, and Node 3
    [1, 0, 0, 0, 0],  # Node 3 has edges to Node 2 and Node 4
    [1, 1, 1, 1, 0]   # Node 4 has edges to Node 0, Node 1, and Node 3
])
# graph = np.array([
#     [0, 1, 1],
#     [0, 0, 1],
#     [1, 0, 0]
# ])

# Calculate PageRank
pagerank_values = pagerank(graph, p=0.5)
print("PageRank values:", pagerank_values)