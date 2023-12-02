import numpy as np

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
    """For the given query term, search the docs dictionary using binary search"""
    # node = [{'term': term, 'pll': pll}, {...}]
    # returns the pll of the query term
    # if the query term is not in the node, returns 0
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

def QP_node(q, n):
    
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
                #print(f"Query term: '{q_term}' with pll={pll} at node {node_idx}")
                found_plls.append(pll)
        # otherwise, return the sum of the plls of all the terms that were found in the node
        if len(found_plls) > 1:
            node_cost = np.sum(found_plls)
            broker_cost = np.min(found_plls)
            #print(f"More than one term in a node, plls are: {found_plls}, node cost is the sum: {node_cost}, broker cost: {broker_cost}")
            return (node_cost, broker_cost)
        # if only one term from a multi-term query was found in the node, return 0
        elif len(found_plls) == 1:
            #print(f"Only one term found at node {node_idx}, node cost: 0, broker cost is its pll {found_plls[0]}")
            node_cost = 0
            broker_cost = found_plls[0]
            return (node_cost, broker_cost)
        
        # if no term was found, return 0
        else:
            #print(f"No term was found at node {node_idx}, node cost is: 0, broker cost is: 0")
            node_cost = 0
            broker_cost = 0
            return (node_cost, broker_cost)

def create_and_get_nodes(table, K):
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
    for _, query in enumerate(query_logs):
        q_node_costs = []
        q_broker_cost = 0
        for i, node in nodes.items():
            
            node_cost, broker_cost = QP_node(query, node)
            accum_sum[i].append(node_cost)
            q_node_costs.append(node_cost)
            q_broker_cost += broker_cost
        max_node_cost = np.max(q_node_costs)
        broker_costs.append(q_broker_cost)
        print(f"Node costs are: {q_node_costs}")
        print(f"Max node cost is: {max_node_cost}")
        print(f"Broker cost for the query '{query}' is: {q_broker_cost}")
        print(f"Total query processing cost for the query '{query}' is: {max_node_cost+q_broker_cost}")
    total_broker_cost = np.sum(broker_costs)
    total_node_costs = [np.sum(arr) for _, arr in accum_sum.items()]
    total_costs = np.concatenate([np.array([total_broker_cost]), total_node_costs])

def main():
    docs_file_name = 'wordlist.txt'
    query_logs_file_name = '2.topics'
    table = get_table(docs_file_name)
    query_logs = get_query_logs(query_logs_file_name)

    nodes = create_and_get_nodes(table, K=2)

    run_term_based_simulation(nodes, query_logs)

if __name__ == '__main__':
    main()