import json
from strgraphcpp import eval_graph, Graph

# Example usage
graph = Graph()
node1 = graph.new_constant("Hello")
node2 = graph.new_constant("World")
node3 = graph.new_case_conversion(node2, "upper")
node4 = graph.new_concatenation(node1, node3)
node5 = graph.new_replacement(node4, graph.new_constant("llo"), node4)

output = graph.eval(node5)
print(output) # HeHelloWORLDWORLD

# DEMO: Shuffling node indices does not alter computation
import random
N = len(graph.nodes)
perm = list(range(N))
random.shuffle(perm)
invperm = list(range(N))
for i in range(N):
    invperm[perm[i]] = i
newgraph = Graph()
for i in range(N):
    node = graph.nodes[invperm[i]]
    if 'nodes' in node.args:
        node.args['nodes'] = [perm[i] for i in node.args['nodes']]
    newgraph.add_node(node)
print(newgraph.serialize())
output = newgraph.eval(newgraph.nodes[perm[5]])
print(output) # HeHelloWORLDWORLD