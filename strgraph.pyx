# distutils: language = c++

import json
from libcpp.string cimport string
from libcpp.vector cimport vector

# RRR distutils: sources = strgraphcpp.cpp

# Declare the C++ function
cdef extern from "strgraphcpp.cpp":
    string eval_graph_cpp(const string &json_str)

# Create a Python wrapper function
def eval_graph(json_obj):
    return eval_graph_cpp(json.dumps(json_obj).encode("utf-8"))
