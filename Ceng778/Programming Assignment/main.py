import numpy as np
import matplotlib.pyplot as plt
import math

def get_table(file_name):
    with open(file_name, 'r') as f:
        table = []
        for line in f:
            line_values = line.split()
            table.append({"term": line_values[0], "pll": int(line_values[1])})
    return table

def get_query_logs(file_name):
    with open(file_name, 'r', encoding='ISO-8859-1') as f:
        query_logs = []
        for line in f:
            parts = line.split(':')
            query_logs.append(parts[1].strip())
    return query_logs

def binary_search(query_term, node):
    left, right = 0, len(node) - 1
    while left <= right:
        mid = (left + right) // 2
        if node[mid]['term'] == query_term:
            return node[mid]['pll']
        elif node[mid]['term'] < query_term:
            left = mid + 1
        else:
            right = mid - 1
    return 0

def calculate_cost(query, node):
    query_terms = query.split()
    if len(query_terms) == 1:
        return binary_search(query_terms[0], node), 0
    found_plls = [binary_search(term, node) for term in query_terms if binary_search(term, node) > 0]
    if len(found_plls) == 0:
        return 0, 0
    if len(found_plls) == 1:
        return 0, found_plls[0]
    return sum(found_plls), min(found_plls)

def create_nodes(data, partition_count):
    nodes = {i: [] for i in range(partition_count)}
    for i, item in enumerate(data):
        nodes[i % partition_count].append({"term": item['term'], "pll": item['pll']})
    return nodes

def run_simulation(nodes, queries):
    accum_costs, broker_costs = {i: [] for i in range(len(nodes))}, []
    total_cost = 0
    for query in queries:
        node_costs = [calculate_cost(query, node)[0] for node in nodes.values()]
        broker_cost = sum(calculate_cost(query, node)[1] for node in nodes.values())
        max_cost = max(node_costs)
        broker_costs.append(broker_cost)
        total_cost += max_cost + broker_cost
        for i, cost in enumerate(node_costs):
            accum_costs[i].append(cost)
    return total_cost / len(queries), np.sum(broker_costs), [np.sum(costs) for costs in accum_costs.values()]

def create_distributed_node(data, partition_count):
    return [{'term': item['term'], 'pll': math.ceil(item['pll']/partition_count)} for item in data]

def calculate_distributed_cost(query, node):
    return sum(binary_search(term, node) for term in query.split())

def run_distributed_simulation(queries, node):
    total_cost = sum(calculate_distributed_cost(query, node) for query in queries)
    return total_cost / len(queries), total_cost

def plot_costs(costs, partition_count, title, filename):
    x = np.arange(partition_count + 1)
    plt.bar(x, costs)
    plt.title(title)
    plt.xlabel('Nodes')
    plt.ylabel('Cost')
    plt.savefig(filename)
    plt.close()

def main():
    data = get_table('wordlist.txt')
    queries = get_query_logs('10000.topics')
    for partition_count in [4, 32]:
        nodes = create_nodes(data, partition_count)
        avg_cost, broker_cost, node_costs = run_simulation(nodes, queries)
        print(f'Average cost for term-based partitioning for K = {partition_count} is {avg_cost/1e4:.2f} x 1e4')
        plot_costs([broker_cost] + node_costs, partition_count, 'Term-Based Simulation', f'TermBased_{partition_count}.png')
        distributed_node = create_distributed_node(data, partition_count)
        avg_distributed_cost, total_distributed_cost = run_distributed_simulation(queries, distributed_node)
        print(f'Average cost for document-based partitioning for K = {partition_count} is {avg_distributed_cost/1e4:2f} x 1e4')
        plot_costs([0] + [total_distributed_cost] * partition_count, partition_count, 'Document-Based Simulation', f'DocBased_{partition_count}.png')

if __name__ == '__main__':
    main()