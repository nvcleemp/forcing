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
#include "set.h"

//===================================================================
// Miscellaneous methods
//===================================================================

boolean isCurrentGraphSelected(graph_t *g){
    if (selectForcingEqualAlpha && forcingNumber != alpha) return FALSE;
    
    if (selectForcingEqual >= 0 && selectForcingEqual != forcingNumber) return FALSE;
    if (selectForcingLess >= 0 && selectForcingLess <= forcingNumber) return FALSE;
    if (selectForcingGreater >= 0 && selectForcingGreater >= forcingNumber) return FALSE;
    
    if (selectAlphaEqual >= 0 && selectAlphaEqual != alpha) return FALSE;
    if (selectAlphaLess >= 0 && selectAlphaLess <= alpha) return FALSE;
    if (selectAlphaGreater >= 0 && selectAlphaGreater >= alpha) return FALSE;
    
    if (selectSizeCoreEqual >= 0 && selectSizeCoreEqual != set_size(core)) return FALSE;
    if (selectSizeCoreLess >= 0 && selectSizeCoreLess <= set_size(core)) return FALSE;
    if (selectSizeCoreGreater >= 0 && selectSizeCoreGreater >= set_size(core)) return FALSE;
    
    if (selectSizeAntiCoreEqual >= 0 && selectSizeAntiCoreEqual != g->n - set_size(anticore)) return FALSE;
    if (selectSizeAntiCoreLess >= 0 && selectSizeAntiCoreLess <= g->n - set_size(anticore)) return FALSE;
    if (selectSizeAntiCoreGreater >= 0 && selectSizeAntiCoreGreater >= g->n - set_size(anticore)) return FALSE;
    
    selectedCount++;
    
    return TRUE;
}

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

void processGraph(graph_t *g, graph_t *originalG){
    clique_options opts = {reorder_by_unweighted_greedy_coloring, NULL, NULL, stderr, handleClique, NULL, independentSets, MAXIMUM_CLIQUE_COUNT};
    
    core = set_new(g->n);
    anticore = set_new(g->n);
    int i;
    for (i=0; i<g->n; i++){
        SET_ADD_ELEMENT(core, i);
    }
    set_empty(anticore);
    independentSetCount = 0;
    forcingNumber = -1;
    
    clique_find_all(g,0,0,TRUE,&opts);
    
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
            int f = forcingNumberForIndependentSet(i, upperBound, g->n);
            if(f!=-1 && (forcingNumber==-1 || f < forcingNumber)){
                forcingNumber = f;
                
                upperBound = f-1;
                /* We have already found a forcing set of size f,
                 * so we only need to see if we can find one that is smaller.
                 */
                
                if(forcingNumber == lowerBound){
                    break;
                }
            }
        }
        
        set_free(temporarySet);
    }
    
    if(outfile!=NULL){
        fprintf(outfile, "%d, %d, %d, %d, %d, %d, %d\n", graphCount, g->n, alpha, independentSetCount, set_size(core), g->n - set_size(anticore), forcingNumber);
    }
    
    forcingNumberCount = increment(forcingNumberCount, forcingNumber);
    
    if (isCurrentGraphSelected(g)) {
        if(toSage)
            printCurrentGraphToSageFile(originalG, sageFile);
    }
    
    set_free(core);
    set_free(anticore);
}

int forcingNumberForIndependentSet(int setNr, int upperBound, int n){
    //if no forcing set with size smaller than upperBound is found, return -1
    
    //first check to see whether the forcing number is 1
    int i;
    set_t setAntiCore = set_duplicate(independentSets[setNr]);
    for(i=0; i<independentSetCount; i++){
        if(i==setNr) continue;
        
        set_t oldSetAntiCore = setAntiCore;
        set_t complement = getSetComplement(independentSets[i]);
        setAntiCore = set_intersection(NULL, oldSetAntiCore, complement);
        set_free(oldSetAntiCore);
        set_free(complement);
    }
    if(set_size(setAntiCore)>0){
        set_free(setAntiCore);
        return 1;
    }
    set_free(setAntiCore);
    
    //calculate force number using a brute-force technique
    set_t forcingSet = set_new(n);
    for (i = 2; i <= upperBound; i++) {
        if (existsForcingSetOfSize(setNr, i, 0, alpha, -1, forcingSet)) {
            set_free(forcingSet);
            return i;
        }
    }
    set_free(forcingSet);
    
    return -1;
}

boolean existsForcingSetOfSize(int iSet, int targetSize, int currentSize, int remainingVertices, int lastVertex, set_t forcingSet){
    if (targetSize == currentSize) {
        //check forcing
        set_t diff;
        int i;
        for (i=0; i<independentSetCount; i++){
            if (iSet!=i) {
                diff = set_difference(NULL, forcingSet, independentSets[i]);
                if (set_size(diff)==0){
                    set_free(diff);
                    return FALSE;
                }
                set_free(diff);
            }
        }
        return TRUE;
    } else if (remainingVertices + currentSize < targetSize) {
        return FALSE;
    } else {
        //try extending the set
        while (remainingVertices + currentSize >= targetSize) {
            int next = set_return_next(independentSets[iSet], lastVertex);
            
            SET_ADD_ELEMENT((forcingSet), next);
            remainingVertices--;
            if(existsForcingSetOfSize(iSet, targetSize, currentSize+1, remainingVertices, next, forcingSet)){
                return TRUE;
            }
            
            SET_DEL_ELEMENT((forcingSet), next);
            lastVertex = next;
        }
        return FALSE;
    }
}


/*
 * set_difference()
 *
 * Store the difference of sets a and b into res.  If res is NULL,
 * a new set is created and the result is written to it.  If res is
 * smaller than the larger one of a and b, it is freed and a new set
 * is created and the result is returned.
 *
 * Returns either res or a new set that has been allocated in its stead.
 *
 * Note:  res may not be a or b.
 */
INLINE
static set_t set_difference(set_t res,set_t a,set_t b) {
	int i,max;

	if (res==NULL) {
		res = set_new(MAX(SET_MAX_SIZE(a),SET_MAX_SIZE(b)));
	} else if (SET_MAX_SIZE(res) < MAX(SET_MAX_SIZE(a),SET_MAX_SIZE(b))) {
		set_free(res);
		res = set_new(MAX(SET_MAX_SIZE(a),SET_MAX_SIZE(b)));
	} else {
		set_empty(res);
	}

	max=MIN(SET_ARRAY_LENGTH(a),SET_ARRAY_LENGTH(b));
	for (i=0; i<max; i++) {
		res[i]=SET_ELEMENT_DIFFERENCE(a[i],b[i]);
	}

	return res;
}

set_t getSetComplement(set_t s){
    int i;
    set_t sComplement = set_new(SET_MAX_SIZE(s));
    for(i = 0; i < SET_MAX_SIZE(s); i++){
        if(!SET_CONTAINS_FAST(s, i)){
            SET_ADD_ELEMENT(sComplement, i);
        }
    }
    return sComplement;
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
// Output methods
//===================================================================

void printGraphAsPythonDict(graph_t *g, FILE *f){
    //for use in Sage
    
    int i, j;
    
    fprintf(f, "d = {");
    
    for (i = 0; i < g->n; i++) {
        fprintf(f, "%d: [", i);
        boolean first = TRUE;
        for (j = 0; j < g->n; j++) {
            if(GRAPH_IS_EDGE(g, i, j)){
                if (first) {
                    fprintf(f, "%d", j);
                    first = FALSE;
                } else {
                    fprintf(f, ", %d", j);
                }
            }
        }
        fprintf(f, "]");
        if (i < g->n-1) {
            fprintf(f, ", ");
        }
    }
    fprintf(f, "}\n");
}

void printCurrentGraphToSageFile(graph_t *g, FILE *f){
    int i;
    
    printGraphAsPythonDict(g, f);
    fprintf(f, "g = Graph(d)\n");
    
    fprintf(f, "sets = '''");
    for (i = 0; i < independentSetCount; i++) {
        int vertex = set_return_next(independentSets[i],-1);
        fprintf(f, "{\\\\small\\\\{%d", vertex);
        while ((vertex=set_return_next(independentSets[i],vertex))>=0) {
            fprintf(f, ", %d", vertex);
        }
        fprintf(f, "\\\\}\\\\\\\\}\n");
    }
    fprintf(f, "'''\n");
    
    fprintf(f, "core = '");
    if (set_size(core)) {
        int vertex = set_return_next(core,-1);
        fprintf(f, "{\\\\small\\\\{%d", vertex);
        while ((vertex=set_return_next(core,vertex))>=0) {
            fprintf(f, ", %d", vertex);
        }
        fprintf(f, "\\\\}}");
    } else {
        fprintf(f, "$\\\\emptyset$");
    }
    fprintf(f, "'\n");
    
    set_t realAnticore = getSetComplement(anticore);
    fprintf(f, "anticore = '");
    if (set_size(realAnticore)) {
        int vertex = set_return_next(realAnticore,-1);
        fprintf(f, "{\\\\small\\\\{%d", vertex);
        while ((vertex=set_return_next(realAnticore,vertex))>=0) {
            fprintf(f, ", %d", vertex);
        }
        fprintf(f, "\\\\}}");
    } else {
        fprintf(f, "$\\\\emptyset$");
    }
    fprintf(f, "'\n");
    set_free(realAnticore);
    
    fprintf(f, "snippets.append(graphToLaTeXSnippet(g, %d, %d, sets, core, anticore))\n\n",
            alpha, forcingNumber);
}

void printHeadSageFile(FILE *f){
    fprintf(f, "def buildLaTeXFile(mainPart):\n");
    fprintf(f, "    start = '''\\\\documentclass[10pt]{article}\n\n");
    fprintf(f, "\\\\usepackage{tikz}\n\\\\usepackage{tkz-berge}\n\n");
    fprintf(f, "\\\\begin{document}\n");
    fprintf(f, "'''\n");
    fprintf(f, "    end = '''\\\\end{document}\\n'''\n");
    fprintf(f, "    return start + mainPart + end\n\n");
    fprintf(f, "def graphToLaTeXSnippet(g, alpha, f, sets, core, anticore):\n");
    fprintf(f, "    from string import Template\n");
    fprintf(f, "    template = Template('''\\\\section*{$name}\n");
    fprintf(f, "$picture\n\\\\bigskip\n\n");
    fprintf(f, "\\\\noindent$$\\\\alpha$$: $alpha\\\\\\\\\n");
    fprintf(f, "Forcing number: $f\\\\\\\\\n");
    fprintf(f, "Independent sets:\\\\\\\\\n");
    fprintf(f, "$sets\\\\\\\\\n");
    fprintf(f, "Core: $core\\\\\\\\\n");
    fprintf(f, "Anti-core: $anticore\\\\\\\\\n''')\n");
    fprintf(f, "    content = template.substitute(name=latexEscape(g.graph6_string()),\n");
    fprintf(f, "                                  picture=latex(g), alpha=alpha, f=f,\n");
    fprintf(f, "                                  sets=sets, core=core, anticore=anticore)\n");
    fprintf(f, "    return content\n\n");
    fprintf(f, "def latexEscape(s):\n");
    fprintf(f, "    s = s.replace('\\\\', r'\\textbackslash ')\n");
    fprintf(f, "    escape_chars = {'#': r'\\#','$': r'\\$','%%': r'\\%%','&': r'\\&',\n");
    fprintf(f, "                    '_': r'\\_','{': r'\\{','}': r'\\}','^': r'\\textasciicircum ',\n");
    fprintf(f, "                    '~': r'\\textasciitilde '}\n");
    fprintf(f, "    for old, new in escape_chars.iteritems(): s = s.replace(old, new)\n");
    fprintf(f, "    s = s.replace('`', r'\\`{}')\n");
    fprintf(f, "    return s\n\n");
    fprintf(f, "snippets = []\n\n");
}

void printTailSageFile(FILE *f){
    fprintf(f, "content = buildLaTeXFile('\\\\clearpage\\n'.join(snippets))\n");
    fprintf(f, "try:\n");
    fprintf(f, "    latex_file = open('output/forcing.tex', 'w')\n");
    fprintf(f, "    latex_file.write(content)\n");
    fprintf(f, "    latex_file.close()\n");
    fprintf(f, "except:\n");
    fprintf(f, "    print 'Creating LaTeX failed. \n");
}
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

        if (fread(&testheader, sizeof (unsigned char), 13, f) != 13) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[13] = 0;
        if (strcmp(testheader, ">>planar_code") != 0) {
            fprintf(stderr, "No planarcode header detected -- exiting!\n");
            exit(1);
        }
        //read reminder of header (either empty or le/be specification)
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            return FALSE;
        }
        while (c!='<'){
            if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
                return FALSE;
            }
        }
        //read one more character
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            return FALSE;
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

boolean readMultiCode(FILE *f, graph_t **g){
    static int first = 1;
    unsigned char c;
    char testheader[20];
    int zeroCounter;
    
    if (first) {
        first = 0;

        if (fread(&testheader, sizeof (unsigned char), 12, f) != 12) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[12] = 0;
        if (strcmp(testheader, ">>multi_code") != 0) {
            fprintf(stderr, "No multicode header detected -- exiting!\n");
            exit(1);
        }
        //read reminder of header (either empty or le/be specification)
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            fprintf(stderr, "Unexpected EOF.\n");
            exit(1);
        }
        while (c!='<'){
            if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
                fprintf(stderr, "Unexpected EOF.\n");
                exit(1);
            }
        }
        //read one more character
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            fprintf(stderr, "Unexpected EOF.\n");
            exit(1);
        }
    }

    if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
        //nothing left in file
        return FALSE;
    }

    if (c != 0) /* unsigned chars would be sufficient */ {
        int order = c;
        *g = graph_new(order);
        zeroCounter = 0;
        while (zeroCounter < order - 1) {
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

    return TRUE;
}

boolean readGraph6(FILE *f, graph_t **g){
    unsigned char c;
    char testheader[10];
    int order;
    
    c = getc(f);
    
    if (c == (unsigned char)EOF) {
        //reached end of file
        return FALSE;
    }
    
    if (c == '>') {
        //there must be a header
        if (fread(&testheader, sizeof (unsigned char), 9, f) != 9) {
            fprintf(stderr, "can't read header ((1)file too small)-- exiting\n");
            exit(1);
        }
        testheader[9] = 0;
        if (strcmp(testheader, ">graph6<<") != 0) {
            fprintf(stderr, "No graph6 header detected -- exiting!\n");
            fprintf(stderr, "Header found was >%s\n", testheader);
            exit(1);
        }
    
        if (fread(&c, sizeof (unsigned char), 1, f) == 0) {
            //nothing left in file
            return FALSE;
        }

        if (c == '\n' && fread(&c, sizeof (unsigned char), 1, f) == 0) {
            //skipped line feed after header and found nothing there :-(
            return FALSE;
        }
    }
    
    if (c - 63 < 63) {
        order = c - 63;
    } else {
        unsigned char c1, c2, c3;
        c1 = getc(f) - 63;
        c2 = getc(f) - 63;
        c3 = getc(f) - 63;
        order = (c1<<12) + (c2<<6) + c3;
    }
    
    *g = graph_new(order);
    
    int i = 0, j = 1;
    while(TRUE){
        c = getc(f) - 63;
        int d;
        for (d = 5; d >= 0; d--) {
            if ((c >> d) & 1) {
                GRAPH_ADD_EDGE(*g, i, j);
            }
            //next position in adjacency matrix
            i++;
            if (i == j){
                j++;
                i = 0;
            }
            if (j == order) break; //reached end of adjacency matrix
        }
        if (j == order) break; //reached end of adjacency matrix
    }
    
    //read end of line
    if (fread(&c, sizeof (unsigned char), 1, f) == 1) {
        //we didn't reach EOF
        if (c != '\n') {
            fprintf(stderr, "Unexpected character-- exiting\n");
            fprintf(stderr, "Expected \\n, but found %c\n", c);
            exit(1);
        }
    }

    return TRUE;
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
    fprintf(stderr, "Valid options\n=============\n");
    fprintf(stderr, "* Various options\n");
    fprintf(stderr, "    -h, --help\n");
    fprintf(stderr, "       Print this help and return.\n");
    fprintf(stderr, "    -f, --filter n\n");
    fprintf(stderr, "       Only do the calculations for the nth graph in the input.\n");
    fprintf(stderr, "    -d, --detailed\n");
    fprintf(stderr, "       Print various details during the calculations.\n");
    fprintf(stderr, "    -v, --verbose\n");
    fprintf(stderr, "       Make the program more verbose.\n");
    fprintf(stderr, "    -o, --output\n");
    fprintf(stderr, "       Write output to stdout. The output consists of comma-separated lists.\n");
    fprintf(stderr, "       This list contains the following items:\n");
    fprintf(stderr, "           * number of the graph\n");
    fprintf(stderr, "           * number of vertices in the graph\n");
    fprintf(stderr, "           * independence number of the graph\n");
    fprintf(stderr, "           * number of maximum independent sets in the graph\n");
    fprintf(stderr, "           * the size of the core of the graph\n");
    fprintf(stderr, "           * the size of the anti-core of the graph\n");
    fprintf(stderr, "           * the forcing number of the graph\n");
    fprintf(stderr, "    -F, --format <format>\n");
    fprintf(stderr, "       Specify the format of the input file. The possible values are:\n");
    fprintf(stderr, "           multi     multi_code\n");
    fprintf(stderr, "           planar    planar_code\n");
    fprintf(stderr, "           g6        graph6\n");
    fprintf(stderr, "       In case this is not specified, the program assumes planar_code.\n");
}

void usage(char *name){
    fprintf(stderr, "Usage: %s [options]\n", name);
    fprintf(stderr, "For more information type: %s -h \n\n", name);
}

void parseSelectString(char *selectString) {
    if (strlen(selectString)<=2){
        //all select specifiers are at least 3 characters long
        fprintf(stderr, "select specifier to short (%s) -- ignoring\n", selectString);
        return;
    } else if (strcmp(selectString, "f=a") == 0 || strcmp(selectString, "a=f") == 0) {
        fprintf(stderr, "Selecting graphs that have forcing number = alpha.\n");
        selectForcingEqualAlpha = TRUE;
    } else if (selectString[0] == 'f') {
        //forcing number
        if (selectString[1] == '=') {
            selectForcingEqual = atoi(selectString+2);
            fprintf(stderr, "Selecting graphs that have forcing number = %d.\n", selectForcingEqual);
        } else if (selectString[1] == '<') {
            selectForcingLess = atoi(selectString+2);
            fprintf(stderr, "Selecting graphs that have forcing number < %d.\n", selectForcingLess);
        } else if (selectString[1] == '>') {
            selectForcingGreater = atoi(selectString+2);
            fprintf(stderr, "Selecting graphs that have forcing number > %d.\n", selectForcingGreater);
        } else {
            fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
            return;
        }
    } else if (selectString[0] == 'a') {
        //alpha
        if (selectString[1] == '=') {
            selectAlphaEqual = atoi(selectString+2);
            fprintf(stderr, "Selecting graphs that have alpha = %d.\n", selectAlphaEqual);
        } else if (selectString[1] == '<') {
            selectAlphaLess = atoi(selectString+2);
            fprintf(stderr, "Selecting graphs that have alpha < %d.\n", selectAlphaLess);
        } else if (selectString[1] == '>') {
            selectAlphaGreater = atoi(selectString+2);
            fprintf(stderr, "Selecting graphs that have alpha > %d.\n", selectAlphaGreater);
        } else {
            fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
            return;
        }
    } else if (strlen(selectString)==3){
        //all following select specifiers are at least 4 characters long
        fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
        return;
    } else if (selectString[0] == 's' && selectString[1] == 'c') {
        //size of core
        if (selectString[2] == '=') {
            selectSizeCoreEqual = atoi(selectString+3);
            fprintf(stderr, "Selecting graphs that have a core with %d element%s.\n",
                    selectSizeCoreEqual, selectSizeCoreEqual == 1 ? "" : "s");
        } else if (selectString[2] == '<') {
            selectSizeCoreLess = atoi(selectString+3);
            fprintf(stderr, "Selecting graphs that have a core with less than %d element%s.\n",
                    selectSizeCoreLess, selectSizeCoreLess == 1 ? "" : "s");
        } else if (selectString[2] == '>') {
            selectSizeCoreGreater = atoi(selectString+3);
            fprintf(stderr, "Selecting graphs that have a core with more than %d element%s.\n",
                    selectSizeCoreGreater, selectSizeCoreGreater == 1 ? "" : "s");
        } else {
            fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
            return;
        }
    } else if (strlen(selectString)==4){
        //all following select specifiers are at least 5 characters long
        fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
        return;
    } else if (selectString[0] == 's' && selectString[1] == 'a' && selectString[2] == 'c') {
        //size of anti-core
        if (selectString[3] == '=') {
            selectSizeAntiCoreEqual = atoi(selectString+4);
            fprintf(stderr, "Selecting graphs that have a anti-core with %d element%s.\n",
                    selectSizeAntiCoreEqual, selectSizeAntiCoreEqual == 1 ? "" : "s");
        } else if (selectString[3] == '<') {
            selectSizeAntiCoreLess = atoi(selectString+4);
            fprintf(stderr, "Selecting graphs that have a anti-core with less than %d element%s.\n",
                    selectSizeAntiCoreLess, selectSizeAntiCoreLess == 1 ? "" : "s");
        } else if (selectString[3] == '>') {
            selectSizeAntiCoreGreater = atoi(selectString+4);
            fprintf(stderr, "Selecting graphs that have a anti-core with more than %d element%s.\n",
                    selectSizeAntiCoreGreater, selectSizeAntiCoreGreater == 1 ? "" : "s");
        } else {
            fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
            return;
        }
    } else {
        fprintf(stderr, "invalid select specifier (%s) -- ignoring\n", selectString);
        return;
    }
    withSelect = TRUE;
}

/*
 * process any command-line options.
 */
int processOptions(int argc, char **argv) {
    int c;
    char *name = argv[0];
    static struct option long_options[] = {
        {"sage", required_argument, NULL, 0},
        {"select", required_argument, NULL, 0},
        {"help", no_argument, NULL, 'h'},
        {"filter", required_argument, NULL, 'f'},
        {"detailed", no_argument, NULL, 'd'},
        {"verbose", no_argument, NULL, 'v'},
        {"output", no_argument, NULL, 'o'},
        {"format", required_argument, NULL, 'F'}
    };
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "hf:dvoF:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                //handle long option with no alternative
                switch(option_index) {
                    case 0:
                        toSage = TRUE;
                        sageFile = fopen(optarg, "w");
                        break;
                    case 1:
                        parseSelectString(optarg);
                        break;
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
            case 'v':
                verbose = TRUE;
                break;
            case 'o':
                outfile = stdout;
                break;
            case 'F':
                if (strcmp(optarg, "multi") == 0) {
                    readGraph = readMultiCode;
                } else if (strcmp(optarg, "planar") == 0) {
                    readGraph = readPlanarCode;
                } else if (strcmp(optarg, "g6") == 0) {
                    readGraph = readGraph6;
                } else {
                    fprintf(stderr, "Unknown file format: %s.\n", optarg);
                    return EXIT_FAILURE;
                }
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
    return -1;
}

int main(int argc, char *argv[]) {
    graph_t *g = NULL;
    graph_t *gComplement;
//    set_t s;
    
    readGraph = readPlanarCode;
    
    int po = processOptions(argc, argv);
    if(po != -1) return po;
    
    graphCount = 0;
    
    if (toSage) {
        printHeadSageFile(sageFile);
    }
    
    while (readGraph(stdin, &g)) {
        graphCount++;
        
        //filtering
        if(onlyGraph && graphCount != onlyGraph) continue;
        
        ASSERT(graph_test(g,(verbose ? stderr : NULL)));
        gComplement = getComplement(g);
        
        if(detailed) graph_print(g);
        
        processGraph(gComplement, g);
        
        graph_free(g);
        graph_free(gComplement);
    }
    
    if (toSage) {
        printTailSageFile(sageFile);
        fclose(sageFile);
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
    
    if (withSelect) {
        fprintf(stderr, "Selected %d graph%s.\n",
                selectedCount, selectedCount == 1 ? "" : "s");
    }

    return 0;
}

