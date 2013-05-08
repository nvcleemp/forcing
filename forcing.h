/*
 * Main developer: Nico Van Cleemput
 * In collaboration with: Craig Larson
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef FORCING_H
#define	FORCING_H

#ifndef MAXIMUM_CLIQUE_COUNT
#define MAXIMUM_CLIQUE_COUNT 200
#endif

#include "freqtable.h"
#include <stddef.h>
#include "misc.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Variables which store data during the whole run of the program
int graphCount;

set_t independentSets[MAXIMUM_CLIQUE_COUNT];
//TODO: allow the user to set a value when calling the program
//TODO: calculate an upper bound in some cases
//      (The general upper bound is sharp, but way too big to always be used.)

freqtable forcingNumberCount = NULL;

freqtable independentSetCounts = NULL;

// Flags which are set at the beginning of the program
int onlyGraph = 0;
int detailed = FALSE;
boolean verbose = FALSE;

// Variables which store data for a single graph

set_t core;
set_t anticore;
int independentSetCount;
int alpha;
int forcingNumber;

// reserved variables for some methods

set_t temporarySet; // used by forcingNumberForIndependentSet

// Methods

boolean handleClique(set_t s, graph_t *g, clique_options *opt);

void processGraph(graph_t *g);

int forcingNumberForIndependentSet(int setNr, int upperBound);

set_t getSetComplement(set_t s);

graph_t *getComplement(graph_t *g);

boolean readPlanarCode(FILE *f, graph_t **g);

void help(char *name);

void usage(char *name);

boolean processOptions(int argc, char **argv);

#ifdef	__cplusplus
}
#endif

#endif	/* FORCING_H */

