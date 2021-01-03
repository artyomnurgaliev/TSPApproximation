//
// Created by artyom on 31/12/20.
//

#ifndef HELLOWORLD_TSPAPPROXIMATION_H
#define HELLOWORLD_TSPAPPROXIMATION_H

#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include "BipartiteGraph.h"
#include "Cycle.h"
#include "Graph.h"
#include "DirectedGraph.h"

using std::vector;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::endl;

class TSPApproximation {
public:
    TSPApproximation(const vector<vector<int>> &edges, const vector<vector<int>> &cycles) : graph(
            edges.size()) {
        for (int i = 0; i < edges.size(); ++i) {
            for (int j = i + 1; j < edges.size(); ++j) {
                graph.AddEdge(i, j, edges[i][j]);
            }
        }
        for (const auto &cycle: cycles) {
            AddCycle(cycle);
        }

        // join all bad cycles in one
        // after it graph has no more than one bad cycle
        while (bad_cycles.size() > 1) {
            auto it = bad_cycles.begin();
            int c1 = *it;
            it++;
            int c2 = *it;
            JoinTwoCycles(c1, c2);
        }


        // find all edges that goes from good cycle to bad cycle
        // and has weight 1, connected with bad edge in bad cycle
        int bad_cycle_idx = -1;
        vector<int> good_connected_cycles;
        if (bad_cycles.size() == 1) {
            bad_cycle_idx = *bad_cycles.begin();
            auto &c = this->cycles.at(bad_cycle_idx);
            for (const auto &vertex: c.GetHeavyEdges()) {
                for (auto another_vertex: graph.EdgesByVertex(vertex))
                    if (another_vertex.second == 1) {
                        int another_cycle_idx = GetCycle(another_vertex.first);
                        auto &another_cycle = this->cycles.at(another_cycle_idx);
                        if (another_cycle.IsGood()) {
                            good_connected_cycles.push_back(another_cycle_idx);
                        }
                    }
            }

            // join all such good cycles with bad cycle
            for (auto good_cycle_idx: good_connected_cycles) {
                JoinTwoCycles(bad_cycle_idx, good_cycle_idx);
            }
        }

        // creating bipartite graph
        // first part - good cycles
        // second part - all vertexes
        BipartiteGraph bipartite_graph;
        for (const auto &vertex_edges: graph.GetEdges()) {
            for (const auto &edge: vertex_edges.second) {
                if (edge.second == 1 && (GetCycle(vertex_edges.first) != GetCycle(edge.first))
                    && (GetCycle(vertex_edges.first) != bad_cycle_idx)) {
                    // info: vertex_edges.first - index of a vertex in a cycle
                    bipartite_graph.AddEdge(GetCycle(vertex_edges.first), edge.first, vertex_edges.first);
                }
            }
        }

        // find optimal matching in a bipartite graph
        auto matching = bipartite_graph.FindOptimalMatching();
        for (auto edge: matching) {
            // edge.second.first - index of a cycle
            // edge.second.second - info: index of a vertex in a cycle
            // edge.first - index of a vertex not in a cycle
            this->cycles.at(GetCycle(edge.first)).SetConnectedEdge(std::make_pair(edge.first, edge.second.second));

            // Creating directed graph of cycles
            // add inverse edges, not as in text
            directed_graph.AddEdge(GetCycle(edge.first), edge.second.first);
        }

        SplitDirectedGraph();


        int bad = -1;
        if (!bad_cycles.empty()) {
            bad = *bad_cycles.begin();
        } else {
            bad = this->cycles.begin()->first;
        }

        while (this->cycles.size() > 1) {
            int second = -1;
            for (const auto& cycle: this->cycles) {
                if (cycle.first != bad) {
                    second = cycle.first;
                }
            }
            TwoCycles twoCycles(bad, second);
            twoCycles.JoinCycles(this);
        }

        auto cycle = this->cycles.at(bad);
        approximation = cycle.GetCycle();
    }

    vector<int> GetApproximation() {
        return approximation;
    }

private:
    void SplitDirectedGraph() {
        auto start_vertexes = directed_graph.FindComponents();
        for (auto start_vertex: start_vertexes) {
            SplitComponent(start_vertex);
        }
    }

    /**
     * Split component on subtrees with max depth 1, paths of length 1 and no more than one path of length 2
     * @param start_vertex - vertex, from which we can call CycleDfs and go over all vertex in component
     */
    void SplitComponent(int start_vertex) {
        auto cycle = directed_graph.FindCycle(start_vertex);

        if (cycle.size() == 1) {
            SplitDfs(start_vertex);
            return;
        }

        // key - index of vertex in cycle
        // value - distance to closest vertex in cycle that has subtree
        unordered_map<int, int> has_subtree;
        unordered_set<int> cycle_vertexes;
        for (auto v: cycle) {
            cycle_vertexes.emplace(v);
            if (directed_graph.GetEdges().at(v).size() > 1) {
                has_subtree.emplace(v, 0);
            }
        }

        // calc distance to closest vertex from cycle which has subtree
        int prev = -1;
        int first = -1;
        for (int i = 0; i < cycle.size(); ++i) {
            if (has_subtree.find(cycle[i]) != has_subtree.end()) {
                if (first != -1) {
                    has_subtree.find(cycle[prev])->second = i - prev - 1;
                } else {
                    first = i;
                }
                prev = i;
            }
        }

        int last_part = cycle.size() - 1 - prev;
        if (!has_subtree.empty()) {
            has_subtree.find(cycle[prev])->second = last_part + first;
        }
        unordered_set<int> used;

        // find subtrees with max depth 1 and join them
        for (auto v: has_subtree) {
            if (used.find(v.first) == used.end()) {
                unordered_set<int> leaves;
                int cycle_leaf = -1;
                for (auto u: directed_graph.GetEdges().at(v.first)) {
                    if (cycle_vertexes.find(u) == cycle_vertexes.end()) {
                        if (SplitDfs(u)) {
                            leaves.emplace(u);
                        }
                    } else {
                        cycle_leaf = u;
                    }
                }
                if (!leaves.empty()) {
                    used.emplace(v.first);
                    assert(cycle_leaf != -1);
                    if (has_subtree.at(v.first) % 2 == 0) {
                        SubTree subTree(v.first, leaves);
                        subTree.JoinCycles(this);
                    } else {
                        leaves.emplace(cycle_leaf);
                        used.emplace(cycle_leaf);
                        SubTree subTree(v.first, leaves);
                        subTree.JoinCycles(this);
                    }
                }
            }
        }

        // split rest of the cycle on paths of length 1 and no more than one path of length 2
        int start = 1;
        int end = 0;
        if (!used.empty()) {
            for (int i = 0; i < cycle.size(); ++i) {
                if (used.find(cycle[i]) != used.end()) {
                    start = (i + 1) % cycle.size();
                    end = i;
                }
            }
        }

        for (size_t i = start; i != end; i = (i + 1) % cycle.size()) {
            if (used.find(cycle[i]) == used.end()) {
                size_t next = (i + 1) % cycle.size();
                size_t nextnext = (next + 1) % cycle.size();
                if (used.find(cycle[next]) == used.end()) {
                    if (nextnext != end) {
                        SubTree subTree(cycle[i], {cycle[next]});
                        subTree.JoinCycles(this);
                        i = next;
                        if (i == end) return;
                    } else {
                        ThreeCycles threeCycles(cycle[i], cycle[next], cycle[nextnext]);
                        threeCycles.JoinCycles(this);
                        return;
                    }
                }
            }
        }
    }

    /**
     *
     * @param v - vertex from vhich dfs is called
     * @return - true if this node should be leaf, false - if root
     */
    bool SplitDfs(int v) {
        if (directed_graph.GetEdges().at(v).empty()) {
            return true;
        }
        unordered_set<int> leaves;
        for (auto u: directed_graph.GetEdges().at(v)) {
            if (SplitDfs(u)) {
                leaves.emplace(u);
            }
        }
        if (!leaves.empty()) {
            SubTree subTree(v, leaves);
            subTree.JoinCycles(this);
            return false;
        } else {
            return true;
        }
    }

    class SmallGraph {
        /**
         *
         * @param travellingSalesmanProblemApproximation
         * @return index of joined cycle
         */
        virtual int JoinCycles(TSPApproximation *travellingSalesmanProblemApproximation) = 0;
    };

    class TwoCycles : public SmallGraph {
    public:
        TwoCycles(int first_cycle, int second_cycle) : first_cycle(first_cycle), second_cycle(second_cycle) {}

        int JoinCycles(TSPApproximation *travellingSalesmanProblemApproximation) override {
            travellingSalesmanProblemApproximation->JoinTwoCycles(first_cycle, second_cycle);
            return first_cycle;
        }

    private:
        int first_cycle;
        int second_cycle;
    };

    class ThreeCycles : public SmallGraph {
    public:
        ThreeCycles(int first_cycle, int second_cycle, int third_cycle) : first_cycle(first_cycle),
                                                                          second_cycle(second_cycle),
                                                                          third_cycle(third_cycle) {}

        int JoinCycles(TSPApproximation *travellingSalesmanProblemApproximation) override {
            travellingSalesmanProblemApproximation->JoinThreeCycles(first_cycle, second_cycle, third_cycle);
            return first_cycle;
        }

    private:
        int first_cycle;
        int second_cycle;
        int third_cycle;
    };

    class SubTree : public SmallGraph {
    public:
        SubTree(int root_cycle, const unordered_set<int> &cycles) : root_cycle(root_cycle), cycles(cycles) {}

        int JoinCycles(TSPApproximation *tspApproximation) override {
            auto &root = tspApproximation->cycles.at(root_cycle);

            // key - index of vertex in root cycle
            // value - index of non root cycle which is connected by edge with root cycle
            unordered_map<int, int> connected_edges;

            for (auto cycle: cycles) {
                auto &c = tspApproximation->cycles.at(cycle);
                auto edge = c.GetConnectedEdge();
                connected_edges.emplace(edge.second, cycle);
            }

            auto start = root.GetEdgeOfMaximumWeight();
            auto first = start.first;
            auto second = start.second;
            while (second != start.first) {
                if (connected_edges.find(first) == connected_edges.end()) {
                    break;
                }
                first = second;
                second = root.GetSecond(first);
            }

            auto last = first;
             while (second != last) {
                if (connected_edges.find(first) != connected_edges.end()) {
                    if (connected_edges.find(second) != connected_edges.end()) {
                        tspApproximation->JoinThreeCyclesWithRoot(root_cycle,
                                                                 connected_edges.find(first)->second,
                                                                 connected_edges.find(second)->second);
                    } else {
                        tspApproximation->JoinTwoCyclesWithRoot(root_cycle, connected_edges.find(first)->second);
                    }
                }

                first = second;
                second = root.GetSecond(first);
            }

            if (connected_edges.find(first) != connected_edges.end()) {
                tspApproximation->JoinTwoCyclesWithRoot(root_cycle, connected_edges.find(first)->second);
            }

            return root_cycle;
        }

    private:
        int root_cycle;
        unordered_set<int> cycles;
    };

    void JoinThreeCyclesWithRoot(int root_idx, int left_child_idx, int right_child_idx) {
        auto &root = cycles.at(root_idx);
        auto &c1 = cycles.at(left_child_idx);
        auto &c2 = cycles.at(right_child_idx);

        auto e1 = c1.GetConnectedEdge();
        auto e2 = c2.GetConnectedEdge();

        assert(root.GetSecond(e1.second) == e2.second);
        root.ChangeEdge(e1.second, e1.first, 1);

        int new_1 = c1.GetPrev(e1.first);
        int new_2 = c2.GetSecond(e2.first);

        c1.ChangeEdge(new_1, new_2, graph.GetEdgeWeight(new_1, new_2));
        c2.ChangeEdge(e2.first, e2.second, 1);
        assert(graph.GetEdgeWeight(e2.first, e2.second) == 1);
        root.AddCycle(c1);
        root.AddCycle(c2);
        for (auto v: root.GetEdges()) {
            SetCycle(v.first, root_idx);
        }
        if (!root.IsGood() && (bad_cycles.find(root_idx) == bad_cycles.end())) {
            bad_cycles.emplace(root_idx);
        }
        cycles.erase(left_child_idx);
        cycles.erase(right_child_idx);
        bad_cycles.erase(left_child_idx);
        bad_cycles.erase(right_child_idx);
    }

    void JoinThreeCycles(int idx1, int idx2, int idx3) {
        auto &c1 = cycles.at(idx1);
        auto &c2 = cycles.at(idx2);
        auto &c3 = cycles.at(idx3);

        auto e1 = c1.GetConnectedEdge();
        auto e2 = c2.GetConnectedEdge();

        c1.ChangeEdge(e1.first, e1.second, 1);
        assert(graph.GetEdgeWeight(e1.first, e1.second) == 1);

        c2.ChangeEdge(c2.GetPrev(e1.second), c1.GetSecond(e1.first),
                      graph.GetEdgeWeight(c2.GetPrev(e1.second), c1.GetSecond(e1.first)));

        c2.ChangeEdge(e2.first, e2.second, 1);
        assert(graph.GetEdgeWeight(e2.first, e2.second) == 1);

        c2.ChangeEdge(c3.GetPrev(e2.second), c2.GetSecond(e2.first),
                      graph.GetEdgeWeight(c3.GetPrev(e2.second), c2.GetSecond(e2.first)));

        c1.AddCycle(c2);
        c1.AddCycle(c3);
        for (auto v: c1.GetEdges()) {
            SetCycle(v.first, idx1);
        }
        if (!c1.IsGood() && (bad_cycles.find(idx1) == bad_cycles.end())) {
            bad_cycles.emplace(idx1);
        }
        cycles.erase(idx2);
        cycles.erase(idx3);
        bad_cycles.erase(idx2);
        bad_cycles.erase(idx3);
    }

    void JoinTwoCyclesWithRoot(int root_idx, int child_idx) {
        auto &root = cycles.at(root_idx);
        auto &c1 = cycles.at(child_idx);

        auto e1 = c1.GetConnectedEdge();
        auto new_1 = root.GetSecond(e1.second);
        root.ChangeEdge(e1.second, e1.first, 1);
        assert(graph.GetEdgeWeight(e1.first, e1.second) == 1);
        c1.ChangeEdge(c1.GetPrev(e1.first), new_1, graph.GetEdgeWeight(c1.GetPrev(e1.first), new_1));
        root.AddCycle(c1);
        for (auto v: root.GetEdges()) {
            SetCycle(v.first, root_idx);
        }
        if (!root.IsGood() && (bad_cycles.find(root_idx) == bad_cycles.end())) {
            bad_cycles.emplace(root_idx);
        }
        cycles.erase(child_idx);
        bad_cycles.erase(child_idx);
    }

    void JoinTwoCycles(int c1_idx, int c2_idx) {
        auto &c1 = cycles.at(c1_idx);
        auto &c2 = cycles.at(c2_idx);
        auto c1_delete_edge = c1.GetEdgeOfMaximumWeight();
        auto c2_delete_edge = c2.GetEdgeOfMaximumWeight();
        c1.ChangeEdge(c1_delete_edge.first,
                      c2_delete_edge.second,
                      graph.GetEdgeWeight(c1_delete_edge.first, c2_delete_edge.second));
        c2.ChangeEdge(c2_delete_edge.first,
                      c1_delete_edge.second,
                      graph.GetEdgeWeight(c2_delete_edge.first, c1_delete_edge.second));
        c1.AddCycle(c2);
        for (auto v: c1.GetEdges()) {
            SetCycle(v.first, c1_idx);
        }
        if (!c1.IsGood() && (bad_cycles.find(c1_idx) == bad_cycles.end())) {
            bad_cycles.emplace(c1_idx);
        }
        cycles.erase(c2_idx);
        bad_cycles.erase(c2_idx);
    }

    int GetCycle(int vertex) {
        return vertexes.find(vertex)->second;
    }

    void SetCycle(int vertex, int cycle_num) {
        if (vertexes.find(vertex) == vertexes.end()) {
            vertexes.emplace(vertex, cycle_num);
        } else {
            vertexes.find(vertex)->second = cycle_num;
        }
    }

    void AddCycle(const vector<int> &cycle) {
        // set index of cycle for each vertex
        for (auto vertex: cycle) {
            SetCycle(vertex, cycles.size());
        }
        Cycle c(cycle, graph);
        if (!c.IsGood()) {
            bad_cycles.emplace(cycles.size());
        }
        cycles.emplace(cycles.size(), c);
    }

    unordered_set<int> bad_cycles; // storage of cycles which has heavy edges
    unordered_map<int, int> vertexes; // value - index of cycle, in which vertex is
    Graph graph;
    unordered_map<int, Cycle> cycles;
    vector<int> approximation{};
    DirectedGraph directed_graph;
};


#endif //HELLOWORLD_TSPAPPROXIMATION_H
