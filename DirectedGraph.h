//
// Created by artyom on 31/12/20.
//

#ifndef HELLOWORLD_DIRECTEDGRAPH_H
#define HELLOWORLD_DIRECTEDGRAPH_H

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

enum Colors {
    GREY, BLACK
};

class DirectedGraph {
public:
    DirectedGraph() = default;

    void AddEdge(int first_vertex, int second_vertex) {
        if (edges.find(first_vertex) == edges.end()) {
            edges.emplace(first_vertex, unordered_set<int>());
        }
        if (edges.find(second_vertex) == edges.end()) {
            edges.emplace(second_vertex, unordered_set<int>());
        }
        edges.find(first_vertex)->second.emplace(second_vertex);
    }

    /**
     * @return set of a start vertexes, starting from which you can go to any vertex in component
     * (if component has a cycle, it will be vertex from cycle)
     */
    unordered_set<int> FindComponents() {
        unordered_set<int> start_vertexes;
        unordered_map<int, int> colors;
        for (const auto &v: edges) {
            if (colors.find(v.first) == colors.end()) {
                start_vertexes.emplace(v.first);
                ComponentDfs(v, colors, start_vertexes);
            }
        }
        return start_vertexes;
    }

    /**
     * Finds cycle in a component which can be bypassed from start_vertex
     * @param start_vertex - vertex, from which we can call CycleDfs and go over all vertex in component
     * @return cycle, if it exists (there can be no more than one cycle), or start_vertex, if there no cycle
     */
    vector<int> FindCycle(int start_vertex) {
        unordered_map<int, int> colors;
        unordered_map<int, int> p;
        vector<int> cycle;
        CycleDfs(*edges.find(start_vertex), colors, p, cycle);
        if (cycle.empty()) {
            cycle.push_back(start_vertex);
        }
        return cycle;
    }

    unordered_map<int, unordered_set<int>>& GetEdges() {
        return edges;
    }

private:
    void ComponentDfs(const pair<int, unordered_set<int>> &v,
                      unordered_map<int, int> &colors,
                      unordered_set<int> &start_vertexes) {
        colors.emplace(v.first, GREY);
        for (auto u: v.second) {
            if (colors.find(u) == colors.end()) {
                ComponentDfs(*edges.find(u), colors, start_vertexes);
            } else if (colors.find(u)->second != GREY) {
                start_vertexes.erase(u);
            }
        }
        colors.find(v.first)->second = BLACK;
    }

    void CycleDfs(const pair<int, unordered_set<int>> &v, unordered_map<int, int> &colors,
                  unordered_map<int, int>& p, vector<int>& cycle) {
        colors.emplace(v.first, GREY);
        for (auto u: v.second) {
            if (colors.find(u) == colors.end()) {
                p.emplace(u, v.first);
                CycleDfs(*edges.find(u), colors, p, cycle);
            } else if (colors.find(u)->second == GREY) {
                cycle.push_back(u);
                int next = v.first;
                while (next != u) {
                    cycle.push_back(next);
                    next = p.find(next)->second;
                }
            }
        }
        colors.find(v.first)->second = BLACK;
    }

    unordered_map<int, unordered_set<int>> edges;
};


#endif //HELLOWORLD_DIRECTEDGRAPH_H
