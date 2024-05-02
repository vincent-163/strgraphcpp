# StrGraphCpp

## Overview
StrGraphCpp is a project aimed at developing a graph computation system where nodes
represent string values and computations are custom defined by the user. The system models
data dependencies and computations as a Directed Acyclic Graph (DAG), similar to PyTorch but
specifically tailored for string manipulation and operations. The graph definition and
manipulation will be done in Python, while the computationally intensive backend is
implemented in C++ for efficiency.

## Objectives

To implement a flexible and efficient node-driven computation system for string values.
Enable custom-defined computations at each node, involving the node's input strings and
additional constants.

Design the system to be user-friendly for defining graphs in Python, with the heavy lifting done
by the C++ backend.

## Key Features

* Python Interface for Graph Definition: Users can define the graph structure, node dependencies,
and computations in Python.
* C++ Backend for Execution: The core computation engine is implemented in C++ for
performance optimization.
* Custom Computation Logic: Users can define their own computation logic for each node, based
on the input node values and constant strings.
* DAG-Based Execution: The system ensures computations are executed in a correct order,
respecting data dependencies within the DAG.

## Installation

Dependencies: cython

Install via `python setup.py install` or `pip install -e .` (recommended)

## Project architecture

This project is divided into two parts: a Python interface and a C++ backend.

The Python interface is defined in `__init__.py`, which can be used to construct a graph.
The graph is then serialized to JSON and passed to the C++ backend via Cython, which then
computes the string for the output node.

The API is designed in such a way that only graphs without cycles can be constructed.
Nevertheless, the backend is robust to permutation of node indices. Non-DAG graphs can't
be handled at the moment, though.

## API Usage

### `eval_graph` function

This function takes a serialized Graph and converts it to JSON and calls the C++ backend. The C++ backend computes the result and returns it to Python.

### `Node` object

This object is created by `new_...()` methods on `Graph`. It should only be used within the Graph it is created by. It represents a node in the DAG to be computed.

### `Graph` object

* `__init__(self)`: Creates an empty graph.

* `new_constant(self, value: str)`: Creates a constant node, which always evaluates to `value`.

* `new_concatenation(self, *nodes: Node)`: Creates a concatenation node, whose value is created by concatenating all nodes specified.

* `new_substring(self, node: Node, start_index: int, end_index: int)`: Creates a substring node, which extracts a substring from a specified constant start index to end index.

* `new_replacement(self, node: Node, old: Node, new: Node)`: Creates a replacement node, which replaces all occurences of `old` to `new` in the original string `node`.

* `new_pattern_matching(self, node: Node, pattern: Node)`: Creates a pattern matching node, which evaluates to the string "true" if `pattern` occurs within `node`, and "false" if otherwise.

* `new_case_conversion(self, node: Node, conversion_type: str)`: Creates a case conversion node, which evaluates to `node` with all letters in its uppercase form if `conversion_type` is "upper", or lowercase if `conversion_type` is "lower".

* `new_length_calculation(self, node: Node)`: Creates a length calculation node, which evaluates to the length of `node` in decimal form.

* `eval_serialize(self, output_node: Node)`: Generates an object representing the graph and an output node. This object can be passed to `eval_graph` function, which will evaluate the output node in C++.

* `eval(self, output_node: Node)`: Combines `eval_serialize` and `eval_graph` and returns the evaluated result directly.

## Custom node types

The node types defined above allow for a variety of string manipulations and evaluations within the graph, making the system flexible and powerful for processing complex string operations dynamically.

However, in cae other operations are needed and you want to implement a custom node type, the following modifications are necessary:

* In `__init__.py`, define a node creation op in the `Graph` class
* In `strgraphcpp.cpp` (which is the C++ backend), define a new node class inheriting from `Node` class, and implement deserialization in `Graph::deserialize`

## Example usage

See `examples/demo.py`. It generates a graph using Python API and uses the C++ backend
to evaluate the result. It then shuffles the node indices and passes the graph to the C++
backend to get the same result.