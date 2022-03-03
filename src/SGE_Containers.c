#include "SGE_Containers.h"
#include "SGE_Logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SGE_LinkedList *SGE_LLCreate(void (*deallocator)(void *data))
{
    SGE_LinkedList *list = (SGE_LinkedList*)malloc(sizeof(SGE_LinkedList));
    list->head = NULL;
    list->size = 0;
    list->deallocator = deallocator;
    return list;
}

SGE_LinkedList *SGE_LLDestroy(SGE_LinkedList *list)
{
    if(list->head != NULL)
    {
        SGE_LLClear(list);
    }
    free(list);
    return NULL;
}

void SGE_LLClear(SGE_LinkedList *list)
{
    int count = 0;
    SGE_LLNode *current = list->head;
    while (current != NULL)
    {
        SGE_LLNode *temp = current->next;
        if(list->deallocator != NULL)
        {
            list->deallocator(current->data);
        }
        free(current);
        current = temp;
        count++;
        list->head = current;
        list->size--;
    }
    SGE_LogPrintLine(SGE_LOG_DEBUG, "Freed linked list with %d nodes!", count);
    list->head = NULL;
    list->size = 0;
}

void SGE_LLPush(SGE_LinkedList *list, void *data)
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to push on NULL linked list!");
        return;
    }

    SGE_LLNode *newNode = (SGE_LLNode*)malloc(sizeof(SGE_LLNode));
    newNode->data = data;
    newNode->next = NULL;

    if(list->head == NULL)
    {
        list->head = newNode;
    }
    else
    {
        SGE_LLNode *current = list->head;
        while(current->next != NULL)
        {
            current = current->next;
        }
        current->next = newNode;
    }
    list->size++;
}

void *SGE_LLGetLast(SGE_LinkedList *list)
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to access NULL linked list!");
        return NULL;
    }
    
    SGE_LLNode *current = list->head;
    if(current == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to access empty linked list!");
        return NULL;
    }
    
    while(current->next != NULL)
    {
        current = current->next;
    }
    return current->data;
}

/*
 *  This can be made faster by adding a tail pointer to the Linked List
 */
void SGE_LLPop(SGE_LinkedList *list)
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to pop on NULL linked list!");
        return;
    }
    
    if(list->head == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to pop on empty linked list!");
        return;
    }

    if(list->head->next == NULL)
    {
        free(list->head);
        list->head = NULL;
        return;
    }

    SGE_LLNode *current = list->head;
    SGE_LLNode *temp = NULL;
    while(current->next != NULL)
    {
        temp = current;
        current = current->next;
    }
    if(list->deallocator != NULL)
    {
        list->deallocator(current->data);
    }
    free(current);
    temp->next = NULL;
}

void SGE_LLPopFront(SGE_LinkedList *list)
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to pop on NULL linked list!");
        return;
    }
    
    if(list->head == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to pop on empty linked list!");
        return;
    }

    SGE_LLNode *temp = list->head;
    list->head = list->head->next;
    if(list->deallocator != NULL)
    {
        list->deallocator(temp->data);
    }
    free(temp);
    return;
}

void SGE_LLRemove(SGE_LinkedList *list, SGE_LLNode *node)
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to remove on NULL linked list!");
        return;
    }

    if(node == list->head)
    {
        SGE_LLPopFront(list);
        return;
    }

    SGE_LLNode *current = list->head;
    SGE_LLNode *temp = NULL;
    while(current != node)
    {
        temp = current;
        current = current->next;
    }
    temp->next = current->next;
    if(list->deallocator != NULL)
    {
        list->deallocator(current->data);
    }
    free(current);
}

void *SGE_LLProcess(SGE_LinkedList *list, void *(*processFunc)(SGE_LinkedList *list, SGE_LLNode* currentNode, void *processData), void *data)
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to process NULL linked list!");
        return NULL;
    }

    SGE_LLNode *current = list->head;
    while(current != NULL)
    {
        SGE_LLNode *temp = current->next;
        void *done = processFunc(list, current, data);
        if(done)
        {
            return done;
        }
        current = temp; // This ensures safety if processFunc() happens to free the current node
    }

    return NULL;
}

void SGE_LLPrint(SGE_LinkedList *list, void (*printFunc)(void *data))
{
    if(list == NULL)
    {
        SGE_LogPrintLine(SGE_LOG_WARNING, "Attempt to print NULL linked list!");
        return;
    }

    SGE_printf(SGE_LOG_DEBUG, "{");
    SGE_LLNode *current = list->head;
    while(current != NULL)
    {
        if(current != list->head)
        {
            SGE_printf(SGE_LOG_DEBUG, ", ");
        }
        printFunc(current->data);
        current = current->next;
    }
    SGE_printf(SGE_LOG_DEBUG, "}\n");
}

void SGE_LLPrintInt(void *data)
{
    SGE_printf(SGE_LOG_DEBUG, "%d", (int*)data);
}

void SGE_LLPrintStr(void *data)
{
    SGE_printf(SGE_LOG_DEBUG, "\"%s\"", (char*)data);
}

void *SGE_LLSearchStr(SGE_LinkedList *list, SGE_LLNode *currentNode, void *processData)
{
    char *currentStr = currentNode->data;
    char *searchStr = processData;
    
    if(!strcmp(currentStr, searchStr))
    {
        SGE_LogPrintLine(SGE_LOG_DEBUG, "String \"%s\" found!", searchStr);
    }

    return NULL;
}

void *SGE_LLSearchRemoveStr(SGE_LinkedList *list,  SGE_LLNode *currentNode, void *processData)
{
    if(processData == NULL)
    {
        return NULL;
    }

    char *currentStr = currentNode->data;
    char *searchStr = processData;
    if(!strcmp(currentStr, searchStr))
    {
        SGE_LLRemove(list, currentNode);
    }

    return NULL;
}