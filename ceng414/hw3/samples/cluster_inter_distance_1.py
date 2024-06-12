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

inter_distances = {'a': {'b': 0.698378, 'e': 0.59863, 'f': 0.571924, 'h': 0.57756},
 'b': {'a': 0.698378, 'e': 0.253874, 'f': 0.768546, 'h': 0.69007},
 'e': {'a': 0.59863, 'b': 0.253874, 'f': 0.640213, 'h': 0.602114},
 'f': {'a': 0.571924, 'b': 0.768546, 'e': 0.640213, 'h': 0.433257},
 'h': {'a': 0.57756, 'b': 0.69007, 'e': 0.602114, 'f': 0.433257}}

try:
    student_distances = cluster_inter_distance(cluster_samples=clusters, strategy='average')
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