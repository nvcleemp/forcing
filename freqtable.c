
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "freqtable.h"


item* increment(item* head, int key){
    //first check whether the list is empty
    if(head==NULL){
        item *new = (item *)malloc(sizeof(item));
        new->key = key;
        new->value = 1;
        new->next = NULL;
        new->prev = NULL;
        return new;
    }
    
    //find the position where the new value should be added
    item *currentItem = head;
    while(currentItem->key < key && currentItem->next!=NULL){
        currentItem = currentItem->next;
    }
    
    if(currentItem->key == key){
        currentItem->value++;
        return head;
    } else if(currentItem->key < key){
        item *new = (item *)malloc(sizeof(item));
        new->key = key;
        new->value = 1;
        new->next = NULL;
        new->prev = currentItem;
        currentItem->next = new;
        return head;
    } else if(currentItem == head){
        item *new = (item *)malloc(sizeof(item));
        new->key = key;
        new->value = 1;
        new->next = head;
        new->prev = NULL;
        head->prev = new;
        return new;
    } else {
        item *new = (item *)malloc(sizeof(item));
        new->key = key;
        new->value = 1;
        new->next = currentItem;
        new->prev = currentItem->prev;
        currentItem->prev->next = new;
        currentItem->prev = new;
        return head;
    }
}

void printFrequencyTable(item *currentItem, char *format) {
    while(currentItem!=NULL){
        fprintf(stderr, format, currentItem->key, currentItem->value);
        currentItem = currentItem->next;
    }
}