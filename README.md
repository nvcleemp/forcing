forcing
=======

Calculating the forcing number of graphs

Basic information
-----------------

A _forcing set_ F of a maximal independent set I is a subset F of I such that I is the only maximal independent set containing F.
The _forcing number_ of a maximal independent set I is the minimum size of a forcing set in I.
The _forcing number_ of a graph G is the minimum value of the forcing numbers for it maximal independent sets.

Program
-------

The program uses Cliquer. You can download this program from http://users.tkk.fi/pat/cliquer.html

You will need the following files:

* cliquer.c
* cliquer.h
* cliquerconf.h
* graph.c
* graph.h
* misc.h
* reorder.c
* reorder.h
* set.h

You can then use the Makefile to build the program `forcing`.
