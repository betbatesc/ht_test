This repo contains a very simple hash table implementation in C++.

A. Introduction

This hash table is populated using data generated by the TPC-H dbgen tool.

Tested using SF-1.

Preliminar C++ implementation.

B. Compilation

The repo includes the hash table implementation and the data generator, along some
utilities for columnizing the read relations (indicated in the main app).

Such utilities where taken and customized from the Vitis Database library 2021.2 version.

The code has been tested using standard GNU tools and make tool, thus compilation
should be as simple as:

    $make all

C. Usage

    $./main -w . -sf 1

where the scale factor argument can be changed (1, 10, 100).
