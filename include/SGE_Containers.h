#ifndef __SGE_CONTAINERS_H__
#define __SGE_CONTAINERS_H__

/*
 * General purpose linked list
 */

typedef struct SGE_LLNode
{
    void *data;
    struct SGE_LLNode *next;
} SGE_LLNode;

typedef struct
{
    SGE_LLNode *head;
    unsigned int size;
    void (*deallocator)(void *data);
} SGE_LinkedList;

SGE_LinkedList *SGE_LLCreate(void (*deallocator)(void *data));
SGE_LinkedList *SGE_LLDestroy(SGE_LinkedList *list);
void SGE_LLClear(SGE_LinkedList *list);
void SGE_LLPush(SGE_LinkedList *list, void *data);
void SGE_LLPop(SGE_LinkedList *list);
void SGE_LLRemove(SGE_LinkedList *list, SGE_LLNode *node);
void SGE_LLPrint(SGE_LinkedList *list, void (*printFunc)(void *data));
void *SGE_LLProcess(SGE_LinkedList *list, void *(*processFunc)(SGE_LinkedList *list, SGE_LLNode* currentNode, void *processData), void *data);
void *SGE_LLGetLast(SGE_LinkedList *list);

void SGE_LLInsert(SGE_LinkedList *list, SGE_LLNode *insertAfter, void *data); // To be implemented
void SGE_LLPushFront(SGE_LinkedList *list, void *data); // To be implemented
void SGE_LLPopFront(SGE_LinkedList *list);

void SGE_LLPrintInt(void *data);
void SGE_LLPrintStr(void *data);
void *SGE_LLSearchStr(SGE_LinkedList *list, SGE_LLNode *currentNode, void *processData);
void *SGE_LLSearchRemoveStr(SGE_LinkedList *list,  SGE_LLNode *currentNode, void *processData);

#endif