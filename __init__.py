from .cppext import eval_graph
from .cppext import *

class Node:
    def __init__(self, node_type, args):
        self.node_type = node_type
        self.args = args
        self.index = None  # Will be set when the node is added to the graph

class Graph:
    def __init__(self):
        self.nodes = []
        self.node_counter = 0

    def add_node(self, node: Node):
        node.index = self.node_counter
        self.nodes.append(node)
        self.node_counter += 1
        return node

    def new_constant(self, value: str):
        return self.add_node(Node('constant', {'value': value}))

    def new_concatenation(self, *nodes: Node):
        return self.add_node(Node('concatenation', {'nodes': [node.index for node in nodes]}))

    def new_substring(self, node: Node, start_index: int, end_index: int):
        return self.add_node(Node('substring', {'nodes': [node.index], 'start': start_index, 'end': end_index}))

    def new_replacement(self, node: Node, old: Node, new: Node):
        return self.add_node(Node('replacement', {'nodes': [node.index, old.index, new.index]}))

    def new_pattern_matching(self, node: Node, pattern: Node):
        return self.add_node(Node('pattern_matching', {'nodes': [node.index, pattern.index]}))

    def new_case_conversion(self, node: Node, conversion_type: str):
        assert conversion_type in {"upper", "lower"}, "Must be either upper or lower"
        return self.add_node(Node('case_conversion', {'nodes': [node.index], 'type': conversion_type}))

    def new_length_calculation(self, node: Node):
        return self.add_node(Node('length_calculation', {'nodes': [node.index]}))

    # Additional methods for other types of nodes can be added here

    def serialize(self):
        return {'nodes': [{**node.args, 'index': node.index, 'node_type': node.node_type} for node in self.nodes]}

    def eval_serialize(self, output_node: Node):
        return {
            'graph': self.serialize(),
            'output_node_index': output_node.index
        }
    
    def eval(self, output_node: Node):
        return eval_graph(self.eval_serialize(output_node))