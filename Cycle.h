//
// Created by artyom on 31/12/20.
//

#ifndef HELLOWORLD_CYCLE_H
#define HELLOWORLD_CYCLE_H

#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include "Graph.h"

using std::vector;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::endl;

const static int HEAVY_EDGE = 2;

class Cycle {
public:
    explicit Cycle(vector<int> vertexes, const Graph &graph) : connected_edge(std::pair<int, int>(-1, -1)){
        for (int i = 1; i < vertexes.size(); ++i) {
            edges.emplace(vertexes[i - 1], vertexes[i]);
            inverse_edges.emplace(vertexes[i], vertexes[i-1]);
            if (graph.GetEdgeWeight(vertexes[i - 1], vertexes[i]) == HEAVY_EDGE) {
                heavy_edges.emplace(vertexes[i - 1]);
            }
        }
        if (graph.GetEdgeWeight(vertexes[vertexes.size() - 1], vertexes[0]) == HEAVY_EDGE) {
            heavy_edges.emplace(vertexes[vertexes.size() - 1]);
        }
        edges.emplace(vertexes[vertexes.size() - 1], vertexes[0]);
        inverse_edges.emplace(vertexes[0], vertexes[vertexes.size() - 1]);
    }

    bool IsGood() {
        return heavy_edges.empty();
    }

    pair<int, int> GetConnectedEdge() const {
        return connected_edge;
    }
    void SetConnectedEdge(pair<int, int> edge) {
        connected_edge.first = edge.first;
        connected_edge.second = edge.second;
    }

    /**
     * Deletes edge with certain first vertex from cycle
     * @param first_vertex
     * @param second_vertex
     */
    void DeleteEdge(int first_vertex, int second_vertex) {
        edges.erase(first_vertex);
        inverse_edges.erase(second_vertex);
        heavy_edges.erase(first_vertex);
    }

    /**
     * Changes second vertex of edge in cycle
     * @param first - first vertex of the edge, method doesn't change this vertex
     * @param new_second - new second vertex of the edge
     * @param weight_of_new_edge - weight of added edge
     */
    void ChangeEdge(int first, int new_second, int weight_of_new_edge) {
        inverse_edges.erase(edges.find(first)->second);
        edges.find(first)->second = new_second;
        inverse_edges.emplace(new_second, first);
        heavy_edges.erase(first);
        if (weight_of_new_edge == HEAVY_EDGE) {
            heavy_edges.emplace(first);
        }
    }

    /**
     * Add edges of another cycle to this cycle
     * @param cycle - another cycle
     */
    void AddCycle(const Cycle &cycle) {
        for (auto edge: cycle.edges) {
            edges.emplace(edge.first, edge.second);
            inverse_edges.emplace(edge.second, edge.first);
        }
        for (auto heavy_edge: cycle.heavy_edges) {
            heavy_edges.emplace(heavy_edge);
        }
    }

    /**
     * @return edge of maximum weight in the graph
     */
    pair<int, int> GetEdgeOfMaximumWeight() {
        if (heavy_edges.empty()) {
            return std::make_pair(edges.begin()->first, edges.begin()->second);
        } else {
            int first = *heavy_edges.begin();
            int second = GetSecond(first);
            return std::make_pair(first, second);
        }
    }

    /**
     * @param first - first vertex of the edge
     * @return second vertex of the edge
     */
    int GetSecond(int first) {
        return edges.at(first);
    }

    int GetPrev(int first) {
        return inverse_edges.at(first);
    }

    const unordered_set<int> &GetHeavyEdges() const {
        return heavy_edges;
    }

    unordered_map<int, int>& GetEdges() {
        return edges;
    }

    void Print() {
        auto first = edges.begin()->first;
        cout << first << " ";
        auto next = edges.find(first)->second;
        while (next != first) {
            cout << next << " ";
            next = edges.find(next)->second;
        }
    }

private:
    unordered_map<int, int> edges; // key - first vertex, value - second
    unordered_map<int, int> inverse_edges; // key - first vertex, value - second
    pair<int, int> connected_edge; // store edge, which goes from this cycle in bipartite graph,
    // first vertex - in cycle, second vertex - not in cycle
    unordered_set<int> heavy_edges;
    // store of first vertexes of heavy edges
};


#endif //HELLOWORLD_CYCLE_H
