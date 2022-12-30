// Adapted from: https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node {
	void* dataPtr;
	int key;
	struct node* next;
} node_t, *nodePtr_t;


////display the list
//void printList(nodePtr_t head);

//insert link at the first location
void LL_insertFirst(nodePtr_t *head, uint16_t key, void* dataPtr);

//delete last item
void LL_removeLast(nodePtr_t* head);

//delete first item
void LL_removeFirst(nodePtr_t* head);

//is list empty
bool LL_isEmpty(nodePtr_t head);

uint16_t LL_length(nodePtr_t head);

//find a link with given key
nodePtr_t LL_find(nodePtr_t head, uint16_t key);

//delete a link with given key
void LL_removeItem(nodePtr_t* head, uint16_t key);

//void sort(nodePtr_t head);

//void reverse(nodePtr_t head);