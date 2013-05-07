/*
 * Main developer: Nico Van Cleemput
 * In collaboration with: Craig Larson
 * 
 * Copyright (C) 2013 Ghent University.
 * Licensed under the GNU GPL, read the file LICENSE.txt for details.
 */

#ifndef FREQTABLE_H
#define	FREQTABLE_H

#ifdef	__cplusplus
extern "C" {
#endif

struct list_el {
    int key;
    int value;
    struct list_el * next;
    struct list_el * prev;
};

typedef struct list_el item;

typedef item* freqtable;

item* increment(item* head, int key);

void printFrequencyTable(item *currentItem, char *format);


#ifdef	__cplusplus
}
#endif

#endif	/* FREQTABLE_H */

