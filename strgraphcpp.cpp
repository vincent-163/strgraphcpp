#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <unordered_set>
#include <algorithm>

using json = nlohmann::json;

class Graph; // Forward declaration

class Node {
    std::optional<std::string> cached_result;

protected:
    virtual std::string eval_inner(Graph& graph) = 0;

public:
    virtual std::vector<size_t> getDependencies() const = 0;

    std::string eval(Graph& graph) {
        if (!cached_result.has_value()) {
            cached_result = eval_inner(graph);
        }
        return cached_result.value();
    }

    bool has_cache() {
        return cached_result.has_value();
    }

    void invalidateCache() {
        cached_result.reset();
    }

    virtual ~Node() = default;
};

class ConstantNode : public Node {
    std::string value;
public:
    void setValue(const std::string &val) {
        value = val;
        invalidateCache();
    }

protected:
    std::string eval_inner(Graph& graph) override {
        return value;
    }

public:
    ConstantNode(const std::string& val) : value(val) {}

    std::vector<size_t> getDependencies() const override {
        return {}; // Constant nodes do not depend on any other nodes.
    }
};

class ConcatenationNode : public Node {
    std::vector<size_t> childrenIndices;

protected:
    std::string eval_inner(Graph& graph) override;

public:
    void addChild(size_t childIndex) {
        childrenIndices.push_back(childIndex);
    }

    std::vector<size_t> getDependencies() const override {
        return childrenIndices;
    }
};

class SubstringNode : public Node {
    size_t inputNodeIndex;
    size_t startIndex;
    size_t endIndex;

protected:
    std::string eval_inner(Graph& graph) override;

public:
    SubstringNode(size_t inputIndex, size_t start, size_t end)
        : inputNodeIndex(inputIndex), startIndex(start), endIndex(end) {}

    std::vector<size_t> getDependencies() const override {
        return {inputNodeIndex};
    }
};

class ReplacementNode : public Node {
    size_t inputNodeIndex, oldNodeIndex, newNodeIndex;

protected:
    std::string eval_inner(Graph& graph) override;

public:
    ReplacementNode(size_t inputIndex, size_t oldIndex, size_t newIndex)
        : inputNodeIndex(inputIndex), oldNodeIndex(oldIndex), newNodeIndex(newIndex) {}

    std::vector<size_t> getDependencies() const override {
        return {inputNodeIndex, oldNodeIndex, newNodeIndex};
    }
};

class PatternMatchingNode : public Node {
    size_t inputNodeIndex, patternNodeIndex;

protected:
    std::string eval_inner(Graph& graph) override;

public:
    PatternMatchingNode(size_t inputIndex, size_t patternIndex)
        : inputNodeIndex(inputIndex), patternNodeIndex(patternIndex) {}

    std::vector<size_t> getDependencies() const override {
        return {inputNodeIndex, patternNodeIndex};
    }
};

class CaseConversionNode : public Node {
    size_t inputNodeIndex;
    std::string conversionType; // "upper" or "lower"

protected:
    std::string eval_inner(Graph& graph) override;

public:
    CaseConversionNode(size_t inputIndex, const std::string& type)
        : inputNodeIndex(inputIndex), conversionType(type) {}

    std::vector<size_t> getDependencies() const override {
        return {inputNodeIndex};
    }
};

class LengthCalculationNode : public Node {
    size_t inputNodeIndex;

protected:
    std::string eval_inner(Graph& graph) override;

public:
    LengthCalculationNode(size_t inputIndex)
        : inputNodeIndex(inputIndex) {}

    std::vector<size_t> getDependencies() const override {
        return {inputNodeIndex};
    }
};

class Graph {
    std::vector<std::shared_ptr<Node>> nodes;

public:
    void deserialize(const json& j);
    std::shared_ptr<Node> getNode(size_t index) {
        return nodes.at(index);
    }
    void checkInvalidateCache();
};

// Definitions of eval_inner functions that require Graph reference
std::string ConcatenationNode::eval_inner(Graph& graph) {
    std::string result;
    for (auto index : childrenIndices) {
        result += graph.getNode(index)->eval(graph);
    }
    return result;
}

std::string SubstringNode::eval_inner(Graph& graph) {
    std::string input = graph.getNode(inputNodeIndex)->eval(graph);
    if (startIndex >= input.size()) {
        return ""; // Start index is out of bounds
    }
    if (endIndex > input.size()) {
        endIndex = input.size(); // Adjust end index if it is out of bounds
    }
    return input.substr(startIndex, endIndex - startIndex);
}

std::string ReplacementNode::eval_inner(Graph& graph) {
    std::string input = graph.getNode(inputNodeIndex)->eval(graph);
    std::string oldString = graph.getNode(oldNodeIndex)->eval(graph);
    std::string newString = graph.getNode(newNodeIndex)->eval(graph);
    std::string result = input;
    size_t pos = 0;
    while ((pos = result.find(oldString, pos)) != std::string::npos) {
        result.replace(pos, oldString.length(), newString);
        pos += newString.length();
    }
    return result;
}

std::string PatternMatchingNode::eval_inner(Graph& graph) {
    std::string input = graph.getNode(inputNodeIndex)->eval(graph);
    std::string pattern = graph.getNode(patternNodeIndex)->eval(graph);
    return (input.find(pattern) != std::string::npos) ? "true" : "false";
}

std::string CaseConversionNode::eval_inner(Graph& graph) {
    std::string input = graph.getNode(inputNodeIndex)->eval(graph);
    if (conversionType == "upper") {
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
    } else if (conversionType == "lower") {
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    }
    return input;
}

std::string LengthCalculationNode::eval_inner(Graph& graph) {
    std::string input = graph.getNode(inputNodeIndex)->eval(graph);
    size_t length = input.length();
    return std::to_string(length);
}

// Implementation of Graph methods
void Graph::deserialize(const json& j) {
    std::unordered_map<int, std::shared_ptr<Node>> nodeIndex;

    for (const auto& node : j["nodes"]) {
        if (node["node_type"] == "constant") {
            auto newNode = std::make_shared<ConstantNode>(node["value"]);
            nodes.push_back(newNode);
            nodeIndex[node["index"]] = newNode;
        } else if (node["node_type"] == "concatenation") {
            auto newNode = std::make_shared<ConcatenationNode>();
            for (int childIndex : node["nodes"]) {
                newNode->addChild(childIndex);
            }
            nodes.push_back(newNode);
            nodeIndex[node["index"]] = newNode;
        } else if (node["node_type"] == "substring") {
            auto inputNodeIndex = node["nodes"][0];
            size_t start = node["start"];
            size_t end = node["end"];
            auto newNode = std::make_shared<SubstringNode>(inputNodeIndex, start, end);
            nodes.push_back(newNode);
            nodeIndex[node["index"]] = newNode;
        }
        else if (node["node_type"] == "replacement") {
            auto inputNodeIndex = node["nodes"][0];
            auto oldNodeIndex = node["nodes"][1];
            auto newNodeIndex = node["nodes"][2];
            auto myNode = std::make_shared<ReplacementNode>(inputNodeIndex, oldNodeIndex, newNodeIndex);
            nodes.push_back(myNode);
            nodeIndex[node["index"]] = myNode;
        }
        else if (node["node_type"] == "pattern_matching") {
            auto inputNodeIndex = node["nodes"][0];
            auto patternNodeIndex = node["nodes"][1];
            auto newNode = std::make_shared<PatternMatchingNode>(inputNodeIndex, patternNodeIndex);
            nodes.push_back(newNode);
            nodeIndex[node["index"]] = newNode;
        }
        else if (node["node_type"] == "case_conversion") {
            auto inputNodeIndex = node["nodes"][0];
            std::string type = node["type"];
            auto newNode = std::make_shared<CaseConversionNode>(inputNodeIndex, type);
            nodes.push_back(newNode);
            nodeIndex[node["index"]] = newNode;
        }
        else if (node["node_type"] == "length_calculation") {
            auto inputNodeIndex = node["nodes"][0];
            auto newNode = std::make_shared<LengthCalculationNode>(inputNodeIndex);
            nodes.push_back(newNode);
            nodeIndex[node["index"]] = newNode;
        }

    }
}

void Graph::checkInvalidateCache() {
    bool visited = true;
    while(visited) {
        visited = false;
        for (auto& n : nodes) if(n->has_cache()) {
            auto dependencies = n->getDependencies();
            for (auto& depIndex : dependencies) {
                if(!getNode(depIndex)->has_cache()) {
                    n->invalidateCache();
                    break;
                }
            }
        }
    }
}

class Computation {
    Graph graph;
    size_t outputIndex;

public:
    void deserialize(const json& j) {
        graph.deserialize(j["graph"]);
        outputIndex = j["output_node_index"];
    }

    std::string eval() {
        return graph.getNode(outputIndex)->eval(graph);
    }

    Graph& getGraph() {
        return graph;
    }
};

std::string eval_graph_cpp(const std::string &json_string) {
    json j = json::parse(json_string);
    Computation computation;
    computation.deserialize(j);
    std::string result = computation.eval();
    return result;
}