import sys
import pprint

from lab3 import cluster_centers


def print_centroids(centroids):
    for key, value in centroids.items():
        print(f"##### Cluster: {key} #####")
        print("[" + ", ".join([f"{val:.3f}" for val in value]) + "]")
        

clusters = {'a': [(0.31, 0.38, 0.34, 0.43),
       (0.49, 0.53, 0.14, 0.26),
       (0.28, 0.41, 0.35, 0.44),
       (0.37, 0.43, 0.18, 0.3)],
 'b': [(0.44, 0.5, 0.79, 0.82),
       (0.45, 0.6, 0.78, 0.8),
       (0.59, 0.52, 0.76, 0.79),
       (0.54, 0.62, 0.76, 0.79)],
 'e': [(0.33, 0.57, 0.77, 0.79),
       (0.45, 0.67, 0.61, 0.66),
       (0.49, 0.61, 0.76, 0.79),
       (0.43, 0.59, 0.74, 0.77)],
 'f': [(0.71, 0.68, 0.43, 0.36),
       (0.76, 0.83, 0.57, 0.3),
       (0.84, 0.74, 0.54, 0.37),
       (0.81, 0.72, 0.38, 0.38)],
 'h': [(0.65, 0.63, 0.48, 0.41),
       (0.65, 0.39, 0.44, 0.35),
       (0.39, 0.53, 0.28, 0.38),
       (0.64, 0.47, 0.51, 0.47)]}
original_centers = {'a': (0.36250000000000004, 0.4375, 0.2525, 0.3575),
 'b': (0.505, 0.56, 0.7725, 0.8),
 'e': (0.425, 0.61, 0.72, 0.7525000000000001),
 'f': (0.78, 0.7424999999999999, 0.48, 0.3524999999999999),
 'h': (0.5825, 0.505, 0.4275, 0.4025)}
try:
    centers = cluster_centers(cluster_samples=clusters)
except Exception as e:
    print("\n[RUNTIME ERROR]", e)
    sys.exit(0)
success = True
try:
    if not isinstance(centers, dict):
        print("\n[FAIL] Check your return values.")
        sys.exit(0)
        
    if not len(centers.keys()):
        success = False
    for cluster, s_centers in centers.items():
        if not len(s_centers):
            success = False
            break
        for i, each in enumerate(s_centers):
            if not ((each > 0 and each*0.98 < original_centers[cluster][i] < each*1.02) or (each < 0 and each*0.98 > original_centers[cluster][i] > each*1.02)):
                success = False
                break
    
    if success:
        print("\n[SUCCESSFUL]")
    else:
        print(f"\n\nEXPECTED OUTPUT:")
        print_centroids(original_centers)
        print(f"\nGIVEN OUTPUT:")
        print_centroids(centers)
except Exception as e:
    print("RUNTIME ERROR", e)
