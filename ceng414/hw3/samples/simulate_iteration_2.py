import sys
import pprint

from lab3 import simulate_iteration


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
my_new_centroids = {'a': (0.384, 0.398, 0.528), 'h': (0.63, 0.6866666666666666, 0.39999999999999997), 'e': (0.7579999999999999, 0.476, 0.5199999999999999), 'b': (0.66, 0.53, 0.59), 'f': (0.7266666666666667, 0.7233333333333333, 0.86)}
my_displacement = 0.3629745531193368
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
