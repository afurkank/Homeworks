import sys
import pprint

from lab3 import cluster_centers


def print_centroids(centroids):
    for key, value in centroids.items():
        print(f"##### Cluster: {key} #####")
        print("[" + ", ".join([f"{val:.3f}" for val in value]) + "]")
        

clusters = {'a': [(0.29, 0.28, 0.44),
       (0.51, 0.46, 0.58),
       (0.4, 0.46, 0.52),
       (0.65, 0.55, 0.34)],
 'b': [(0.43, 0.4, 0.58),
       (0.89, 0.55, 0.51),
       (0.46, 0.61, 0.48),
       (0.66, 0.53, 0.59)],
 'e': [(0.78, 0.44, 0.45),
       (0.72, 0.42, 0.65),
       (0.63, 0.42, 0.48),
       (0.77, 0.55, 0.51)],
 'f': [(0.68, 0.76, 0.84),
       (0.77, 0.57, 0.88),
       (0.61, 0.75, 0.51),
       (0.73, 0.84, 0.86)],
 'h': [(0.63, 0.8, 0.46),
       (0.69, 0.67, 0.3),
       (0.74, 0.74, 0.31),
       (0.29, 0.39, 0.52)]}
original_centers = {'a': (0.4625, 0.4375, 0.47000000000000003),
 'b': (0.61, 0.5225, 0.5399999999999999),
 'e': (0.725, 0.4575, 0.5225),
 'f': (0.6975, 0.73, 0.7725),
 'h': (0.5874999999999999, 0.65, 0.3975)}
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
