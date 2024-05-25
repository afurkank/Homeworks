import sys
import pprint

from lab3 import cluster_centers


def print_centroids(centroids):
    for key, value in centroids.items():
        print(f"##### Cluster: {key} #####")
        print("[" + ", ".join([f"{val:.3f}" for val in value]) + "]")
        

clusters = {'a': [(0.17, 0.39, 0.53, 0.3, 0.39),
       (0.34, 0.33, 0.38, 0.35, 0.44),
       (0.65, 0.47, 0.59, 0.3, 0.4),
       (0.4, 0.5, 0.45, 0.39, 0.47)],
 'b': [(0.47, 0.55, 0.58, 0.71, 0.75),
       (0.34, 0.51, 0.67, 0.9, 0.9),
       (0.59, 0.56, 0.55, 0.8, 0.82),
       (0.63, 0.5, 0.59, 0.85, 0.86)],
 'e': [(0.64, 0.45, 0.67, 0.61, 0.66),
       (0.66, 0.48, 0.54, 0.7, 0.74),
       (0.63, 0.51, 0.64, 0.72, 0.76),
       (0.62, 0.42, 0.58, 0.79, 0.81)],
 'f': [(0.69, 0.59, 0.77, 0.39, 0.21),
       (0.74, 0.9, 0.57, 0.53, 0.29),
       (0.64, 0.57, 0.7, 0.33, 0.26),
       (0.75, 0.76, 0.83, 0.57, 0.3)],
 'h': [(0.67, 0.81, 0.54, 0.49, 0.23),
       (0.63, 1.0, 0.35, 0.51, 0.49),
       (0.74, 0.74, 0.31, 0.53, 0.52),
       (0.66, 0.71, 0.41, 0.5, 0.35)]}
original_centers = {'a': (0.39, 0.4225, 0.4875, 0.33499999999999996, 0.425),
 'b': (0.5075, 0.53, 0.5975, 0.8150000000000001, 0.8324999999999999),
 'e': (0.6375000000000001,
       0.46499999999999997,
       0.6075,
       0.7050000000000001,
       0.7425),
 'f': (0.705, 0.7050000000000001, 0.7175, 0.45499999999999996, 0.265),
 'h': (0.675,
       0.815,
       0.40249999999999997,
       0.5075000000000001,
       0.39749999999999996)}
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
    print("\n[ENVIRONMENT ERROR] Inform instructor.", e)
