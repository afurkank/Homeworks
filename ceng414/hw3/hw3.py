cluster_samples = {'a': [(0.17, 0.39, 0.53, 0.3, 0.39),
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

from typing import Dict, List, Tuple
import itertools
import math

def calculate_distance(point1: Tuple[float, float], point2: Tuple[float, float]) -> float:
    return math.sqrt((point1[0] - point2[0])**2 + (point1[1] - point2[1])**2)

def single_linkage(cluster_a: List[Tuple[float, float]], cluster_b: List[Tuple[float, float]]) -> float:
    return min(calculate_distance(a, b) for a, b in itertools.product(cluster_a, cluster_b))

def complete_linkage(cluster_a: List[Tuple[float, float]], cluster_b: List[Tuple[float, float]]) -> float:
    return max(calculate_distance(a, b) for a, b in itertools.product(cluster_a, cluster_b))

def average_linkage(cluster_a: List[Tuple[float, float]], cluster_b: List[Tuple[float, float]]) -> float:
    distances = [calculate_distance(a, b) for a, b in itertools.product(cluster_a, cluster_b)]
    return sum(distances) / len(distances)

def cluster_distances(clusters: Dict[str, List[Tuple[float, float]]], metric: str) -> Dict[str, Dict[str, float]]:
    distance_functions = {
        'single': single_linkage,
        'average': average_linkage,
        'complete': complete_linkage
    }

    result = {}

    for cluster_name_a, cluster_a in clusters.items():
        result[cluster_name_a] = {}
        for cluster_name_b, cluster_b in clusters.items():
            if cluster_name_a != cluster_name_b:
                distance = distance_functions[metric](cluster_a, cluster_b)
                result[cluster_name_a][cluster_name_b] = distance
    print(result)
    return result

cluster_distances(cluster_samples, 'single')