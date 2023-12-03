import numpy as np
import matplotlib.pyplot as plt
import math
def get_table(file_name):
    with open(file_name, 'r') as f:
        table = []
        for line in f:
            line_values = line.split()
            table.append(
                {
                    "term" : line_values[0],
                    "pll" : int(line_values[1]),
                }
            )
    return table

def get_query_logs(file_name):
    with open(file_name, 'r', encoding='ISO-8859-1') as f:
        query_logs = []
        for line in f:
            parts = line.split(':')
            query_logs.append(parts[1].strip())
    return query_logs

def search_node(q_term, n):
    left, right = 0, len(n) - 1
    while left <= right:
        mid = (left + right) // 2
        if n[mid]['term'] == q_term:
            return n[mid]['pll']
        elif n[mid]['term'] < q_term:
            left = mid + 1
        else:
            right = mid - 1
    return 0
def calculate_term_based_cost(q, n):
    # node = [{'term': term, 'pll': pll}, {...}]
    q_terms = q.split()
    # if query includes only one term, return pll of that term
    if len(q_terms) == 1:
        node_cost = search_node(q_terms[0], n)
        broker_cost = 0
        #print(f"The query term {q_terms[0]} was not found in the node {node_idx}")
        return (node_cost, broker_cost)
    # else, for each term in query, search the node
    else:
        found_plls = []
        for q_term in q_terms:
            pll = search_node(q_term, n)
            # if the term exists in the node, increase the number of terms found in this node
            if pll > 0:
                found_plls.append(pll)
        # otherwise, return the sum of the plls of all the terms that were found in the node
        if len(found_plls) > 1:
            node_cost = np.sum(found_plls)
            broker_cost = np.min(found_plls)
            return (node_cost, broker_cost)
        # if only one term from a multi-term query was found in the node, return 0
        elif len(found_plls) == 1:
            node_cost = 0
            broker_cost = found_plls[0]
            return (node_cost, broker_cost)
        # if no term was found, return 0
        else:
            node_cost = 0
            broker_cost = 0
            return (node_cost, broker_cost)
def create_term_based_nodes(table, K):
    nodes = {i: [] for i in range(K)}
    for i, item in enumerate(table):
        term = item['term']
        pll = item['pll']
        node_idx = i % K
        nodes[node_idx].append({"term" : term, "pll" : pll})
    return nodes
def run_term_based_simulation(nodes, query_logs):
    accum_sum = {i : [] for i in range(len(nodes))}
    broker_costs = []
    average_sum = 0
    for _, query in enumerate(query_logs):
        q_node_costs = []
        q_broker_cost = 0
        for i, node in nodes.items():
            node_cost, broker_cost = calculate_term_based_cost(query, node)
            accum_sum[i].append(node_cost)
            q_node_costs.append(node_cost)
            q_broker_cost += broker_cost
        max_node_cost = np.max(q_node_costs)
        broker_costs.append(q_broker_cost)
        average_sum += max_node_cost + q_broker_cost
    average_sum /= len(query_logs)
    print(f"Average overall query processing cost for term based simulation is {average_sum}")
    total_broker_cost = np.sum(broker_costs)
    total_node_costs = [np.sum(arr) for _, arr in accum_sum.items()]
    total_costs = np.concatenate([np.array([total_broker_cost]), total_node_costs])
    return total_costs
def create_doc_based_node(table, K):
    # create only one node for simplicity
    node = []
    for _, item in enumerate(table):
        term = item['term']
        pll = item['pll']
        distributed_pll = math.ceil(pll/K)
        node.append({'term': term, 'pll': distributed_pll})
    return node
def calculate_doc_based_cost(query, node):
    query_terms = query.split()
    found_plls = []
    for query_term in query_terms:
        found_plls.append(search_node(query_term, node))
    return np.sum(found_plls)
def run_doc_based_simulation(query_logs, node, K):
    node_costs = []
    for query in query_logs:
        query_cost = calculate_doc_based_cost(query, node)
        node_costs.append(query_cost)
    total_cost = np.sum(node_costs)
    avg_cost = total_cost / len(query_logs)
    print(f"Average cost for document-based partitioning for K={K} is {avg_cost}")
    return total_cost
def plot_graph(costs, K, file_name):
    x = np.arange(K+1)
    plt.bar(x, costs)
    plt.title('Cost per Node Graph')
    plt.xlabel('Nodes')
    plt.ylabel('QP Cost')
    plt.show()
    plt.savefig(file_name)
    plt.close()
def main():
    docs_file_name = 'wordlist.txt'
    query_logs_file_name = '10000.topics'
    K_values = [4, 32]
    table = get_table(docs_file_name)
    query_logs = get_query_logs(query_logs_file_name)
    for K in K_values:
        nodes = create_term_based_nodes(table, K=K)
        total_costs = run_term_based_simulation(nodes, query_logs)
        plot_graph(total_costs, K)
    for K in K_values:
        node = create_doc_based_node(table, K=K)
        total_cost = run_doc_based_simulation(query_logs, node, K=K)
        x_data = np.concatenate([np.array([0]), np.array([total_cost]).repeat(K)])
        file_name = 'DocBasedLoadDistributionGraph_k-' + str(K) + '.png'
        plot_graph(x_data, K=K, file_name=file_name)

if __name__ == '__main__':
    main()