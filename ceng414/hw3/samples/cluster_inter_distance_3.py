import pprint
import sys

from lab3 import cluster_inter_distance


def print_distances(inter_distances):
    printing = {}
    for ref_cluster, distances in inter_distances.items():
        for d_cluster, distance in distances.items():
            if d_cluster not in printing:
                printing[d_cluster] = {}
            printing[d_cluster][ref_cluster] = round(distance, 3)
    pprint.pprint(printing)
        

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

inter_distances = {'a': {'b': 0.859942, 'e': 0.839643, 'f': 0.689057, 'h': 0.469574},
 'b': {'a': 0.859942, 'e': 0.294958, 'f': 0.739594, 'h': 0.676092},
 'e': {'a': 0.839643, 'b': 0.294958, 'f': 0.757034, 'h': 0.661287},
 'f': {'a': 0.689057, 'b': 0.739594, 'e': 0.757034, 'h': 0.563383},
 'h': {'a': 0.469574, 'b': 0.676092, 'e': 0.661287, 'f': 0.563383}}
try:
    student_distances = cluster_inter_distance(cluster_samples=clusters, strategy='complete')
except Exception as e:
    print("\n[RUNTIME ERROR]", e)
    sys.exit(0)
success = True
try:
    if not isinstance(student_distances, dict):
        print("\n[FAIL] Check your return values.")
        sys.exit(0)
    
    if not len(student_distances.keys()):
        success = False
    for ref_cluster, distances in inter_distances.items():
        if not len(distances):
            success = False
            break
        for d_cluster, distance in distances.items():
            if not student_distances.get(ref_cluster, {}).get(d_cluster) or student_distances.get(ref_cluster, {}).get(d_cluster) < distance*.98 or student_distances.get(ref_cluster, {}).get(d_cluster) > distance*1.02:
                success = False
                break
    
    if success:
        print("\n[SUCCESSFUL]")
    else:
        print(f"\n\nEXPECTED OUTPUT:")
        print_distances(inter_distances)
        print(f"\nGIVEN OUTPUT:")
        print_distances(student_distances)
except Exception as e:
    print("\n[ENVIRONMENT ERROR] Inform instructor.", e)
