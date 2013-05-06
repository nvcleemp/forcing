/* 
 * File:   freqtable.h
 * Author: nvcleemp
 *
 * Created on May 6, 2013, 7:51 PM
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

