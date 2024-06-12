import sys
import pprint

from lab3 import simulate_iteration


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
my_new_centroids = {'a': (0.36800000000000005, 0.45600000000000007, 0.258, 0.362), 'b': (0.502, 0.5700000000000001, 0.7699999999999999, 0.798), 'e': (0.4033333333333333, 0.61, 0.7066666666666667, 0.7400000000000001), 'f': (0.78, 0.7424999999999999, 0.48, 0.3524999999999999), 'h': (0.6466666666666666, 0.49666666666666665, 0.4766666666666666, 0.41)}
my_displacement = 0.14144409006231712
my_is_converged = False
try:
    ret = simulate_iteration(cluster_samples=clusters, displacement_threshold=.1)
    if not ret or not isinstance(ret, tuple) or len(ret) != 3:
        print("\n[FAIL] You should return 3 variables: new centroids, displacement, is converged.")
        sys.exit(0)
    
    new_centroids = ret[0]
    displacement = ret[1]
    is_converged = ret[2]
except Exception as e:
    print("\n[RUNTIME ERROR]", e)
    sys.exit(0)
success = True
try:
    if not isinstance(new_centroids, dict) or not (isinstance(displacement, float) or isinstance(displacement, int)) or not isinstance(is_converged, bool):
        print("\n[FAIL] Check your return values.")
        sys.exit(0)
        
    if not len(new_centroids.keys()):
        success = False
    for cluster, s_centers in new_centroids.items():
        for i, each in enumerate(s_centers):
            if not ((each > 0 and each*0.98 < my_new_centroids[cluster][i] < each*1.02) or (each < 0 and each*0.98 > my_new_centroids[cluster][i] > each*1.02)):
                success = False
                break
    
    if not success:
        print(f"\n\nEXPECTED OUTPUT:")
        print_centroids(my_new_centroids)
        print(f"\nGIVEN OUTPUT:")
        print_centroids(new_centroids)
    
    elif not my_displacement*0.98 < displacement < my_displacement*1.02:
        success = False
        print(f"\n\nEXPECTED OUTPUT:")
        print(f"Displacement: {my_displacement:.3f}")
        print(f"\nGIVEN OUTPUT:")
        print(f"Displacement: {displacement:.3f}")
        
    elif is_converged != my_is_converged:
        success = False
        print(f"\n\nEXPECTED OUTPUT:")
        print(f"Is Converged: {my_is_converged}")
        print(f"\nGIVEN OUTPUT:")
        print(f"Is Converged: {is_converged}")
    
    if success:
        print("\n[SUCCESSFUL]")
        
except Exception as e:
    print("\n[ENVIRONMENT ERROR] Inform instructor.", e)
