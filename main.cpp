#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include "DirectedGraph.h"
#include "TSPApproximation.h"
#include <cstdlib>

using std::vector;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::endl;

/**
 *
 * @param n - length of a permutation
 * @return random permutation of length n
 */
vector<int> RandomPermutation(int n) {
    vector<int> permutation(n);
    for (int i = 0; i < n; ++i) {
        permutation[i] = i;
    }
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % i;
        std::swap(permutation[i], permutation[j]);
    }
    return permutation;
}

/**
 *
 * @param edges - edges of a graph
 * @param cycle - cycle, for which we calc weight
 * @return weight of a cycle in a graph
 */
int CalcWeight(const vector<vector<int>>& edges, vector<int> cycle) {
    int res = 0;
    for (int i = 0; i < cycle.size(); ++i) {
        res += edges[cycle[i]][cycle[(i + 1) % cycle.size()]];
    }
    return res;
}

/**
 *
 * Split 'cycles' on two cycles while cycles.size() les than num_cycles
 * and change weight of new edges added to cycles such that new weight of
 * cycles is equal to previous weight
 *
 * @param cycles - result vector of cycles
 * @param num_cycles - number of cycles
 * @param edges - edges of a graph
 */
void SplitCycles(vector<vector<int>>& cycles, int num_cycles, vector<vector<int>>& edges) {
    while (cycles.size() < num_cycles) {
        int cycle_num = rand() % cycles.size();
        auto cycle = cycles[cycle_num];
        while (cycle.size() < 8) {
            cycle_num = (cycle_num + 1) % cycles.size();
            cycle = cycles[cycle_num];
        }
        if (cycle.size() < 8) {
            cout << "No cycles with length more than 7, can't split any cycle" << endl;
            return;
        }

        int first = rand() % cycle.size();
        int length = 4;
        if (cycle.size() > 8) {
            length += rand() % (cycle.size() - 8);
        }
        int second = (first + length) % cycle.size();

        vector<int> first_cycle;
        vector<int> second_cycle;

        int first_end = -1;
        int second_end = -1;

        if (first < second) {
            first_end = second - 1;
            second_end = first - 1;
            if (second_end < 0) {
                second_end = cycle.size() - 1;
            }
            first_cycle = vector<int>(cycle.begin() + first, cycle.begin() + second);
            second_cycle.reserve( cycle.size() - first_cycle.size());
            second_cycle.insert( second_cycle.end(), cycle.begin() + second, cycle.end());
            if (first > 0) {
                second_cycle.insert( second_cycle.end(), cycle.begin(), cycle.begin() + first);
            }

        } else {
            second_end = first - 1;
            first_end = second - 1;
            if (first_end < 0) {
                first_end = cycle.size() - 1;
            }
            second_cycle = vector<int>(cycle.begin() + second, cycle.begin() + first);
            first_cycle.reserve( cycle.size() - second_cycle.size());
            first_cycle.insert( first_cycle.end(), cycle.begin() + first, cycle.end());
            if (second > 0) {
                first_cycle.insert( first_cycle.end(), cycle.begin(), cycle.begin() + second);
            }
        }

        edges[cycle[first]][cycle[first_end]] = edges[cycle[second_end]][cycle[first]];
        edges[cycle[first_end]][cycle[first]] = edges[cycle[second_end]][cycle[first]];
        edges[cycle[second]][cycle[second_end]] = edges[cycle[first_end]][cycle[second]];
        edges[cycle[second_end]][cycle[second]] = edges[cycle[first_end]][cycle[second]];

        cycles[cycle_num] = first_cycle;
        cycles.push_back(second_cycle);
    }
}

/**
 * Function to test accuracy of approximation
 * creates graph with edges of weight 2
 * choose random permutation of vertexes and
 * set weight of some edges by this permutation to 1
 *
 * calc weight of permutation
 * than calc approximation and its weight
 *
 * calc accuracy as weight_of_permutation/weight_of_approximation
 *
 * @param num_vertexes - number of vertexes in a graph
 * @param num_cycles - number of cycles on which permutation is split to calc approximation
 * @param num_good_edges - number of edges of weight 1 in permutation
 */
void Test(int num_vertexes, int num_cycles, int num_good_edges) {
    assert(num_good_edges <= num_vertexes);
    vector<vector<int>> edges(num_vertexes);
    vector<vector<int>> cycles(1);
    for (auto &edge : edges) {
        edge = vector<int>(num_vertexes, 2);
    }
    auto permutation = RandomPermutation(num_vertexes);
    auto good_edges_indexes = RandomPermutation(num_vertexes);
    for (int i = 0; i < num_good_edges; ++i) {
        int ind_in_permutation = good_edges_indexes[i];
        int left = permutation[ind_in_permutation];
        int right = permutation[(ind_in_permutation + 1) % num_vertexes];
        edges[left][right] = 1;
        edges[right][left] = 1;
    }
    double real_weight = CalcWeight(edges, permutation);
    //cout << "Real weight: " <<  real_weight << endl;

    cycles[0] = permutation;
    SplitCycles(cycles, num_cycles, edges);

    TSPApproximation tspApproximation(edges, cycles);
    vector<int> approximation = tspApproximation.GetApproximation();
    double approximation_weight = CalcWeight(edges, approximation);
    //cout << "Approximation weight: " << approximation_weight << endl;

    cout << approximation_weight / real_weight;
}

int main(int argc, char** argv) {
    int num_vertexes = strtol(argv[1], nullptr, 10);
    int num_cycles = strtol(argv[2], nullptr, 10);
    int num_good_edges = strtol(argv[3], nullptr, 10);
    Test(num_vertexes, num_cycles, num_good_edges);
    //Test(40, 4, 40);
}