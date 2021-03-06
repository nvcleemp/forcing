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

// Flags and variables which are set at the beginning of the program
int onlyGraph = 0;
int detailed = FALSE;
boolean verbose = FALSE;
FILE *outfile = NULL;

boolean (*readGraph) (FILE *f, graph_t **g);

boolean toSage = FALSE;
FILE *sageFile = NULL;

boolean toSagenb = FALSE;
FILE *sagenbFile = NULL;

boolean withSelect = FALSE;

boolean selectForcingEqualAlpha = FALSE;

int selectForcingEqual = -1;
int selectForcingGreater = -1;
int selectForcingLess = -1;

int selectAlphaEqual = -1;
int selectAlphaGreater = -1;
int selectAlphaLess = -1;

int selectSizeCoreEqual = -1;
int selectSizeCoreGreater = -1;
int selectSizeCoreLess = -1;

int selectSizeAntiCoreEqual = -1;
int selectSizeAntiCoreGreater = -1;
int selectSizeAntiCoreLess = -1;

int selectedCount = 0;

// Variables which store data for a single graph

set_t core;
set_t anticore;
int independentSetCount;
int alpha;
int forcingNumber;

// reserved variables for some methods

set_t temporarySet; // used by forcingNumberForIndependentSet

// Methods

boolean isCurrentGraphSelected(graph_t *g);

boolean handleClique(set_t s, graph_t *g, clique_options *opt);

void processGraph(graph_t *g, graph_t *originalG);

int forcingNumberForIndependentSet(int setNr, int upperBound, int n);

boolean existsForcingSetOfSize(int iSet, int targetSize, int currentSize, int remainingVertices, int lastVertex, set_t forcingSet);

static set_t set_difference(set_t res,set_t a,set_t b);

set_t getSetComplement(set_t s);

graph_t *getComplement(graph_t *g);

void printGraphAsPythonDict(graph_t *g, FILE *f);

void printCurrentGraphToSageWorksheetFile(graph_t *g, FILE *f);

void printCurrentGraphToSageFile(graph_t *g, FILE *f);

boolean readPlanarCode(FILE *f, graph_t **g);

boolean readMultiCode(FILE *f, graph_t **g);

boolean readGraph6(FILE *f, graph_t **g);

void help(char *name);

void usage(char *name);

void parseSelectString(char *selectString);

boolean processOptions(int argc, char **argv);

#ifdef	__cplusplus
}
#endif

#endif	/* FORCING_H */

