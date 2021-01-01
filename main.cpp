#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include "DirectedGraph.h"
#include "TSPApproximation.h"

using std::vector;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::cout;
using std::endl;

int main() {
    int N = 21;
    vector<vector<int>> edges(N);
    vector<vector<int>> cycles(7);
    for (int i = 0; i < edges.size(); ++i) {
        edges[i] = vector<int>(N, 2);
    }
    edges[0][1] = 1;
    edges[1][2] = 1;
    edges[1][4] = 1;
    edges[0][2] = 1;
    edges[3][4] = 1;
    edges[4][5] = 1;
    edges[3][5] = 1;
    edges[6][7] = 1;
    edges[7][8] = 1;
    edges[6][8] = 1;
    edges[9][10] = 1;
    edges[9][11] = 1;
    edges[12][13] = 1;
    edges[12][14] = 1;
    edges[7][10] = 1;
    edges[15][16] = 1;
    edges[16][17] = 1;
    edges[15][17] = 1;
    edges[18][19] = 1;
    edges[18][20] = 1;
    edges[19][20] = 1;
    edges[1][17] = 1;
    edges[4][20] = 1;


    cycles[0] = {0, 1, 2};
    cycles[1] = {3, 4, 5};
    cycles[2] = {6, 7, 8};
    cycles[3] = {9, 10, 11};
    cycles[4] = {12, 13, 14};
    cycles[5] = {15, 16, 17};
    cycles[6] = {18, 19, 20};

    TSPApproximation tspApproximation(edges, cycles);
}