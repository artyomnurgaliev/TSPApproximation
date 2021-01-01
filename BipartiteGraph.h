//
// Created by artyom on 31/12/20.
//

#ifndef HELLOWORLD_BIPARTITEGRAPH_H
#define HELLOWORLD_BIPARTITEGRAPH_H

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

class BipartiteGraph {
public:
    BipartiteGraph() = default;

    /**
     * Adds edge to bipartite graph
     * @param first_vertex - number of vertex in first part,
     * @param second_vertex - number of vertex in second part
     * @param info - info about vertexes from first part
     */
    void AddEdge(int first_vertex, int second_vertex, int vertex_info = -1) {
        if (edges.find(first_vertex) == edges.end()) {
            edges.emplace(first_vertex, unordered_set<int>());
        }
        edges.find(first_vertex)->second.emplace(second_vertex);

        if (info.find(first_vertex) == info.end()) {
            info.emplace(first_vertex, unordered_map<int, int>());
        }
        info.find(first_vertex)->second.emplace(second_vertex, vertex_info);
    }

    /**
     * @return unordered_map, which includes optimum matching in bipartite graph
     * key in unordered_map: vertex in second part, value - vertex in first part which is matched with it
     * and info about it
     *
     * Method uses Kuhn algorithm, with finding any matching before main algorithm (optimization)
     */
    unordered_map<int, pair<int, int>> FindOptimalMatching() {
        unordered_set<int> used_in_find_any_matching;
        unordered_set<int> used;
        unordered_map<int, int> matching = FindAnyMatching(used_in_find_any_matching);

        for (const auto &first_part_vertex: edges) {
            if (used_in_find_any_matching.find(first_part_vertex.first) == used_in_find_any_matching.end()) {
                TryFindAugmentingPath(first_part_vertex.first, used, matching);
            }
        }
        unordered_map<int, pair<int, int>> result;
        for (auto pair: matching) {
            int vertex_info = info.at(pair.second).at(pair.first);
            result.emplace(pair.first, std::make_pair(pair.second, vertex_info));
        }

        return result;
    }

private:

    /**
     * @param first_part_vertex - vertex, for which trying to find augmenting path
     * @param used unordered_set with edges from first part of bipartite graph,
     * which are used in matching
     * @param matching - matching
     * @return true if augmenting path is found, false - else
     * If augmenting path is found, rearranges edges to add path to matching
     * Finds augmenting path using ComponentDfs
     */
    bool TryFindAugmentingPath(int first_part_vertex, unordered_set<int> &used, unordered_map<int, int> &matching) {
        if (used.find(first_part_vertex) != used.end()) {
            return false;
        }
        used.emplace(first_part_vertex);
        for (auto vertex: edges.find(first_part_vertex)->second) {
            if (matching.find(vertex) == matching.end()
                || TryFindAugmentingPath(matching.find(vertex)->second, used, matching)) {
                matching.emplace(vertex, first_part_vertex);
                return true;
            }
        }
        return false;
    }

    /**
     * Simple heuristic algorithm to find any matching in bipartite graph
     *
     * @param used: algorithm fills this unordered_set with edges from first part of bipartite graph,
     * which are used in matching, that method returns
     *
     * @return unordered_map, which includes some matching in bipartite graph
     * key in unordered_map: vertex in second part, value - vertex in first part which is matched with it
     */
    unordered_map<int, int> FindAnyMatching(unordered_set<int> &used) {
        unordered_map<int, int> matching;
        for (const auto &first_part_vertex: edges) {
            for (int vertex: first_part_vertex.second) {
                if (matching.find(vertex) == matching.end()) {
                    matching.emplace(vertex, first_part_vertex.first);
                    used.emplace(first_part_vertex.first);
                    break;
                }
            }
        }
        return matching;
    }

    unordered_map<int, unordered_set<int>> edges; // edges from first part to second
    unordered_map<int, unordered_map<int, int>> info; // info about vertexes from first part
};

#endif //HELLOWORLD_BIPARTITEGRAPH_H
