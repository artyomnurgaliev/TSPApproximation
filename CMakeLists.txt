cmake_minimum_required(VERSION 3.17)
project(helloworld)

set(CMAKE_CXX_STANDARD 17)

add_executable(helloworld main.cpp TSPApproximation.h BipartiteGraph.h Cycle.h DirectedGraph.h Graph.h)