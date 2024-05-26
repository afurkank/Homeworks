import numpy as np
from itertools import permutations
from itertools import combinations

def pagerank(graph, p=0.5, tol=1.0e-2, max_iter=10):
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

# def find_graph_for_pagerank(target_pagerank, p=0.5):
#     nodes = 5
#     edges = 10
#     possible_edges = list(permutations(range(nodes), 2))
#     print(f"possible_edges:\n{possible_edges}")
    
#     # Try different combinations of edges
#     k=0
#     for edges_comb in permutations(possible_edges, edges):
#         print(f"Testing {k}th permutation")
#         k+=1
#         graph = np.zeros((nodes, nodes))
#         for (i, j) in edges_comb:
#             graph[i][j] = 1
        
#         pr = pagerank(graph, p=p)
#         pr_dict = {i + 1: pr[i] for i in range(nodes)}
        
#         if np.allclose(list(pr_dict.values()), list(target_pagerank.values()), atol=1e-2):
#             return pr_dict, edges_comb
    
#     return None, None

def find_graph_for_pagerank(target_pagerank, p=0.5):
    nodes = 5
    edges = 10
    possible_edges = [(i, j) for i in range(nodes) for j in range(nodes) if i != j]
    print(f"possible_edges:\n{possible_edges}")
    
    # Try different combinations of edges
    k=0
    for edges_comb in combinations(possible_edges, edges):
        print(f"Testing {k}th permutation")
        k+=1
        graph = np.zeros((nodes, nodes))
        for (i, j) in edges_comb:
            graph[i][j] = 1
        
        pr = pagerank(graph, p=p)
        pr_dict = {i + 1: pr[i] for i in range(nodes)}
        
        if np.allclose(list(pr_dict.values()), list(target_pagerank.values()), atol=1e-2):
            return pr_dict, edges_comb
    
    return None, None

# Desired PageRank results
target_pagerank = {
    1: 1.0500007545670038,
    2: 0.8250005444708511,
    3: 0.9625008443091959,
    4: 0.7000002998383448,
    5: 0.5
}

# Find the graph configuration that achieves the target PageRank
pagerank_result, edges_comb = find_graph_for_pagerank(target_pagerank)

if pagerank_result:
    print("Graph found with the following PageRank values:")
    print(pagerank_result)
    print("Edges:")
    print(edges_comb)
else:
    print("No graph configuration found to achieve the desired PageRank results.")
