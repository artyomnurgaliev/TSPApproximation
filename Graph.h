//
// Created by artyom on 31/12/20.
//

#ifndef HELLOWORLD_GRAPH_H
#define HELLOWORLD_GRAPH_H

#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cassert>

using std::vector;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::endl;

class Graph {
public:
    explicit Graph(int n) {
        for (int i = 0; i < n; ++i) {
            edges.emplace(i, unordered_map<int, int>());
        }
    }

    void AddEdge(int first_vertex, int second_vertex, int weight) {
        edges.find(first_vertex)->second.emplace(second_vertex, weight);
        edges.find(second_vertex)->second.emplace(first_vertex, weight);
    }

    int GetEdgeWeight(int first_vertex, int second_vertex) const {
        return edges.find(first_vertex)->second.find(second_vertex)->second;
    }

    const unordered_map<int, int> &EdgesByVertex(int vertex) {
        return edges.at(vertex);
    }

    const unordered_map<int, unordered_map<int, int>> &GetEdges() const {
        return edges;
    }
private:
    unordered_map<int, unordered_map<int, int>> edges;
};


#endif //HELLOWORLD_GRAPH_H
