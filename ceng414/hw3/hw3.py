def calculate_distance(p1, p2):

    (x1, y1) = p1[0], p1[1]

    (x2, y2) = p2[0], p2[1]
    
    dist = ( ( x1 - x2 ) **2 +  ( y1 - y2 ) **2 ) ** 0.5

    return dist

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
strategy='average'

cluster_distances = {}

for i, cluster_name in enumerate(cluster_samples):

    inner_cluster_distances = {}

    cluster_points = cluster_samples[cluster_name]

    num_points = len(cluster_points)

    for j, other_cluster_name in enumerate(cluster_samples):
        if i == j:
            continue
        
        other_cluster_points = cluster_samples[other_cluster_name]

        other_num_points = len(other_cluster_points)

        resulting_distance = None

        if strategy == 'single':
            # min distance between any two points inside the clusters

            min_distance = 999

            for point_tuple in cluster_points:
                
                for other_point_tuple in other_cluster_points:
                    
                    distance = calculate_distance(point_tuple, other_point_tuple)

                    if distance < min_distance:
                        min_distance = distance

            resulting_distance = min_distance

        elif strategy == 'complete':
            # max distance between any two points inside the clusters

            max_distance = -1000

            for point_tuple in cluster_points:
                
                for other_point_tuple in other_cluster_points:
                    
                    distance = calculate_distance(point_tuple, other_point_tuple)

                    if distance > max_distance:
                        max_distance = distance
            
            resulting_distance = max_distance
        
        else: # average
            # avg distance between data points of first and second clusters

            sum_distance = 0

            num_distances = 0

            for point_tuple in cluster_points:
                
                for other_point_tuple in other_cluster_points:
                    
                    distance = calculate_distance(point_tuple, other_point_tuple)

                    num_distances += 1
                    sum_distance += distance

            resulting_distance = sum_distance / num_distances
        
        inner_cluster_distances[other_cluster_name] = resulting_distance

    cluster_distances[cluster_name] = inner_cluster_distances

for dict in cluster_distances:
    print(dict + " : ", cluster_distances[dict])