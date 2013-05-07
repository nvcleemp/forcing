/* 
 * File:   forcing.h
 * Author: nvcleemp
 *
 * Created on May 7, 2013, 9:26 AM
 */

#ifndef FORCING_H
#define	FORCING_H

#include "freqtable.h"
#include <stddef.h>
#include "misc.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Variables which store data during the whole run of the program
int graphCount;

freqtable forcingNumberCount = NULL;

freqtable independentSetCounts = NULL;

// Flags which are set at the beginning of the program
int onlyGraph = 0;
int detailed = FALSE;

// Variables which store data for a single graph

set_t core;
set_t anticore;
int independentSetCount;
int alpha;
int forcingNumber;

// Methods

boolean handleClique(set_t s, graph_t *g, clique_options *opt);

void processGraph(graph_t *g);

graph_t *getComplement(graph_t *g);

boolean readPlanarCode(FILE *f, graph_t **g);

void help(char *name);

void usage(char *name);

boolean processOptions(int argc, char **argv);

#ifdef	__cplusplus
}
#endif

#endif	/* FORCING_H */

