// Adapted from: https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linkedList.h"


//display the list
//void printList(nodePtr_t head) 
//{
//    nodePtr_t ptr = head;
//    printf("\n[ ");
//
//    //start from the beginning
//    while (ptr != NULL) {
//        printf("(%d,%d) ", ptr->key, ptr->dataPtr);
//        ptr = ptr->next;
//    }
//
//    printf(" ]");
//}

//insert link at the first location
void LL_insertFirst(nodePtr_t* head, uint16_t key, void* dataPtr)
{
    //create a link
    nodePtr_t link = (nodePtr_t)malloc(sizeof(node_t));

    link->key = key;
    link->dataPtr = dataPtr;

    //point it to old first node
    link->next = *head;

    //point head to new link
    *head = link;
}

//delete first item
void LL_removeFirst(nodePtr_t* head)
{
    //Reurn if there is nothing to remove
    if (*head == NULL)
    {
        return;
    }

    //save reference to first link
    nodePtr_t tempLink = *head;

    //Set head to next element in list
    *head = (*head)->next;

    //Free the data 
    free(tempLink->dataPtr);

    //Free the link
    free(tempLink);
}

//delete last item
void LL_removeLast(nodePtr_t* head)
{
    //Reurn if there is nothing to remove
    if (*head == NULL)
    {
        return;
    }
    
    nodePtr_t current = *head;
    nodePtr_t prev = current;

    //Find last item
    while (current->next != NULL)
    {
        prev = current;
        current = current->next;
    }

    //Account for if head is the last item
    if (*head == current)
    {
        *head = NULL;
    }
    else
    {
        //Reset previus links next pointer
        prev->next = NULL;
    }

    //Free the data 
    free(current->dataPtr);

    //Free the link
    free(current);
}

//is list empty
bool LL_isEmpty(nodePtr_t head)
{
    return head == NULL;
}

uint16_t LL_length(nodePtr_t head)
{
    uint16_t length = 0;
    nodePtr_t current;

    for (current = head; current != NULL; current = current->next) {
        length++;
    }

    return length;
}

//find a link with given key
nodePtr_t LL_find(nodePtr_t head, uint16_t key) 
{

    //start from the first link
    nodePtr_t current = head;

    //if list is empty
    if (head == NULL) {
        return NULL;
    }

    //navigate through list
    while (current->key != key) {

        //if it is last node
        if (current->next == NULL) {
            return NULL;
        }
        else {
            //go to next link
            current = current->next;
        }
    }

    //if dataPtr found, return the current Link
    return current;
}

//delete a link with given key
void LL_removeItem(nodePtr_t* head, uint16_t key)
{
    //if list is empty
    if (*head == NULL) 
    {
        return;
    }

    //start from the first link
    nodePtr_t current = *head;
    nodePtr_t previous = NULL;

    //navigate through list
    while (current->key != key) 
    {

        //if it is last node
        if (current->next == NULL) 
        {
            return;
        }
        else 
        {
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }

    //found a match, update the link
    if (current == *head) 
    {
        //change first to point to next link
        *head = (*head)->next;
    }
    else 
    {
        //bypass the current link
        previous->next = current->next;
    }

    //Free resources of link
    free(current->dataPtr);
    free(current);
}

/* void sort(nodePtr_t head) {
   int i, j, k, tempKey, tempData;
   struct node *current;
   struct node *next;

   int size = length();
   k = size ;

   for ( i = 0 ; i < size - 1 ; i++, k-- ) {
      current = head;
      next = head->next;

      for ( j = 1 ; j < k ; j++ ) {
         if ( current->dataPtr > next->dataPtr ) {
            tempData = current->dataPtr;
            current->dataPtr = next->dataPtr;
            next->dataPtr = tempData;
            tempKey = current->key;
            current->key = next->key;
            next->key = tempKey;
         }

         current = current->next;
         next = next->next;
      }
   }
} */

/* void reverse(nodePtr_t head) {
   nodePtr_t prev   = NULL;
   nodePtr_t current = *head_ref;
   nodePtr_t next;

   while (current != NULL) {
      next  = current->next;
      current->next = prev;
      prev = current;
      current = next;
   }

   *head_ref = prev;
} */