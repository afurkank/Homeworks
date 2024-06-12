import sys
import pprint

from lab3 import simulate_iteration


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

my_new_centroids = {'a': (0.39, 0.4225, 0.4875, 0.33499999999999996, 0.425), 'b': (0.5075, 0.53, 0.5975, 0.8150000000000001, 0.8324999999999999), 'e': (0.6375000000000001, 0.46499999999999997, 0.6075, 0.7050000000000001, 0.7425), 'f': (0.6875, 0.6825, 0.71, 0.445, 0.25), 'h': (0.6925000000000001, 0.8374999999999999, 0.41, 0.5175000000000001, 0.4125)}
my_displacement = 0.06910137480542633
my_is_converged = True
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
