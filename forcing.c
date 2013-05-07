/*
 * Main developer: Nico Van Cleemput
 * In collaboration with: Craig Larson
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#include <stdio.h>
#include <getopt.h>
#include "cliquer.h"
#include "forcing.h"

//===================================================================
// Forcing independence methods
//===================================================================


boolean handleClique(set_t s,graph_t *g,clique_options *opt){
    if(detailed){
        set_print(s);
    }
    if(independentSetCount==0){
        //use the first independent set to determine the value of alpha
        alpha = set_size(s);
    }
    if(independentSetCount >= MAXIMUM_CLIQUE_COUNT){
        fprintf(stderr, "Not prepared to handle that many independent sets.\n");
        fprintf(stderr, "Recompile the program with a larger value for MAXIMUM_CLIQUE_COUNT.\n");
        exit(EXIT_FAILURE);
    }
    independentSetCount++;
    
    set_t oldCore = core;
    core = set_intersection(NULL, oldCore, s);
    set_free(oldCore);
    
    set_t oldAnticore = anticore;
    anticore = set_union(NULL, oldAnticore, s);
    set_free(oldAnticore);
    
    return TRUE;
}

void processGraph(graph_t *g){
    clique_options *opts;
    opts=malloc(sizeof(clique_options));
    opts->time_function=NULL;
    opts->output=stderr;
    opts->reorder_function=NULL;
    opts->reorder_map=NULL;
    opts->user_function=handleClique;
    opts->user_data=NULL;
    opts->clique_list=independentSets;
    opts->clique_list_length=MAXIMUM_CLIQUE_COUNT;
    
    core = set_new(g->n);
    anticore = set_new(g->n);
    int i;
    for (i=0; i<g->n; i++){
        SET_ADD_ELEMENT(core, i);
    }
    set_empty(anticore);
    independentSetCount = 0;
    forcingNumber = -1;
    
    clique_find_all(g,0,0,TRUE,opts);
    
    fprintf(stdout, "%d, %d, %d, %d, %d, %d\n", graphCount, g->n, alpha, independentSetCount, set_size(core), g->n - set_size(anticore));
    
    independentSetCounts = increment(independentSetCounts, independentSetCount);
    
    if(independentSetCount == 1){
        forcingNumber = 0;
    } else if(independentSetCount == 2){
        forcingNumber = 1;
    } else {
        int lowerBound = 1;
        int upper1 = independentSetCount - 1;
        int upper2 = alpha - set_size(core);
        int upperBound = upper1 < upper2 ? upper1 : upper2;
        
        temporarySet = set_new(g->n);
        
        for(i=0; i<independentSetCount; i++){
            int f = forcingNumberForIndependentSet(i, upperBound);
            if(f!=-1 && (forcingNumber==-1 || f < forcingNumber)){
                forcingNumber = f;
                upperBound = f; //f is at most upperBound
                if(forcingNumber == lowerBound){
                    break;
                }
            }
        }
        
        set_free(temporarySet);
    }
    
    forcingNumberCount = increment(forcingNumberCount, forcingNumber);
    
    set_free(core);
    set_free(anticore);
}

int forcingNumberForIndependentSet(int setNr, int upperBound){
    //if no forcing set with size smaller than upperBound is found, return -1
    
    //first check to see whether the forcing number is 1
    int i;
    for(i=setNr+1; i<independentSetCount; i++){
        if(set_size(set_intersection(temporarySet, independentSets[setNr], independentSets[i]))>0){
            return 1;
        }
    }
    
    //calculate force number using a brute-force technique
    
    return -1;
}

//===================================================================
// Graph processing methods
//===================================================================

graph_t *getComplement(graph_t *g){
    int i, j;
    graph_t * gComplement = graph_new(g->n);
    for(i = 0; i < g->n-1; i++){
        for(j = i+1; j < g->n; j++){
            if(!GRAPH_IS_EDGE_FAST(g,i,j)){
                GRAPH_ADD_EDGE(gComplement, i, j);
            }
        }
    }
    return gComplement;
}

//===================================================================
// Printing methods
//===================================================================


//===================================================================
// Input methods
//===================================================================
boolean readPlanarCode(FILE *f, graph_t **g){
    static int first = 1;
    unsigned char c;
    char testheader[20];
    int zeroCounter;
    
    if (first) {
        first = 0;

        if (fread(&testheader, sizeof (unsigned char), 15, f) != 15) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[15] = 0;
        if (strcmp(testheader, ">>planar_code<<") != 0) {
            fprintf(stderr, "No planarcode header detected -- exiting!\n");
            exit(1);
        }
    }

    if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
        //nothing left in file
        return (0);
    }

    if (c != 0) /* unsigned chars would be sufficient */ {
        int order = c;
        *g = graph_new(order);
        zeroCounter = 0;
        while (zeroCounter < order) {
            int neighbour = (unsigned short) getc(f);
            if (neighbour == 0) {
                zeroCounter++;
            } else {
                GRAPH_ADD_EDGE(*g, zeroCounter, neighbour-1);
            }
        }
    } else {
        int order = 0;
        int readCount = fread(&order, sizeof (unsigned short), 1, f);
        if(!readCount){
            fprintf(stderr, "Unexpected EOF.\n");
            exit(1);
        }
        *g = graph_new(order);
        int neighbour = 0;
        zeroCounter = 0;
        while (zeroCounter < order) {
            readCount = fread(&neighbour, sizeof (unsigned short), 1, f);
            if(!readCount){
                fprintf(stderr, "Unexpected EOF.\n");
                exit(1);
            }
            if (neighbour == 0) {
                zeroCounter++;
            } else {
                GRAPH_ADD_EDGE(*g, zeroCounter, neighbour-1);
            }
        }
    }

    return (1);
}

//===================================================================
// Usage methods
//===================================================================
void help(char *name){
    fprintf(stderr, "The program %s calculates the forcing number for graphs.\n\n", name);
    fprintf(stderr, "Usage\n=====\n");
    fprintf(stderr, " %s [options]\n", name);
    fprintf(stderr, "       Calculate the forcing number for any graph read through stdin.\n");
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Currently only graphs in planarcode format are supported.");
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "* Various options\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    -f, --filter n\n");
    fprintf(stderr, "       Only do the calculations for the nth graph in the input.\n");
    fprintf(stderr, "    -d, --detailed\n");
    fprintf(stderr, "       Print various details during the calculations.\n");
}

void usage(char *name){
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

/*
 * process any command-line options.
 */
boolean processOptions(int argc, char **argv) {
    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"filter", required_argument, NULL, 'f'},
        {"detailed", no_argument, NULL, 'd'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hf:d", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                //handle long option with no alternative
                switch(option_index) {
                    default:
                        fprintf(stderr, "Illegal option index %d.\n", option_index);
                        usage(name);
                        return EXIT_FAILURE;
                }
                break;
            case 'h':
                help(name);
                return EXIT_SUCCESS;
            case 'f':
                onlyGraph = atoi(optarg);
                break;
            case 'd':
                detailed = TRUE;
                break;
            case '?':
                usage(name);
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Illegal option %c.\n", c);
                usage(name);
                return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    graph_t *g = NULL;
    graph_t *gComplement;
//    set_t s;
    
    if(processOptions(argc, argv)==EXIT_FAILURE) return EXIT_FAILURE;
    
    graphCount = 0;
    
    while (readPlanarCode(stdin, &g)) {
        graphCount++;
        
        //filtering
        if(onlyGraph && graphCount != onlyGraph) continue;
        
        ASSERT(graph_test(g,stderr));
        gComplement = getComplement(g);
        
        if(detailed) graph_print(g);
        
        processGraph(gComplement);
        
        graph_free(g);
        graph_free(gComplement);
    }
    
    char *format = "%5d : %6d\n";
    
    fprintf(stderr, "Count   Graphs\n");
    fprintf(stderr, "--------------\n");
    printFrequencyTable(independentSetCounts, format);
    
    format = "%3d : %6d\n";
    fprintf(stderr, " f    Graphs\n");
    fprintf(stderr, "------------\n");
    printFrequencyTable(forcingNumberCount, format);
    
    fprintf(stderr, "Read %d graphs.\n", graphCount);

    return 0;
}

