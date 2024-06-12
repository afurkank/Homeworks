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

inter_distances = {'a': {'b': 0.09, 'e': 0.161245, 'f': 0.265518, 'h': 0.130384},
 'b': {'a': 0.09, 'e': 0.12, 'f': 0.207364, 'h': 0.152643},
 'e': {'a': 0.161245, 'b': 0.12, 'f': 0.256125, 'h': 0.254755},
 'f': {'a': 0.265518, 'b': 0.207364, 'e': 0.256125, 'h': 0.073485},
 'h': {'a': 0.130384, 'b': 0.152643, 'e': 0.254755, 'f': 0.073485}}
try:
    student_distances = cluster_inter_distance(cluster_samples=clusters, strategy='single')
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
