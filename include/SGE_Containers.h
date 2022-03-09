#ifndef __SGE_CONTAINERS_H__
#define __SGE_CONTAINERS_H__

/**
 * \brief A single node of an SGE_LinkedList.
 * 
 */
typedef struct SGE_LLNode
{
    void *data;
    struct SGE_LLNode *next;
} SGE_LLNode;

/**
 * \brief A linked list of general purpose data.
 * 
 * Stores void pointers in each node.
 * Can call a custom callback function for each node when the linked list is destroyed.
 * 
 */
typedef struct
{
    SGE_LLNode *head; /**< The first node in the list. */
    unsigned int count; /**< The current number of nodes in the list. */
    void (*deallocator)(void *data); /**< Callback function to call for each node when destroying the list. */
} SGE_LinkedList;

/**
 * \brief Create a new linked list of void pointers.
 * 
 * \param deallocator Callback function to call for each node when destroying the list.
 *                    The callback function will receive the data pointer stored in the current node
 *                    which can be freed if needed.
 * \return Pointer to a new SGE_LinkedList.
 */
SGE_LinkedList *SGE_LLCreate(void (*deallocator)(void *data));

/**
 * \brief Destroys a linked list. And calls the list's deallocator callback for each node.
 * 
 * \param list The list to destroy.
 * \return NULL
 */
SGE_LinkedList *SGE_LLDestroy(SGE_LinkedList *list);

/**
 * \brief Removes all nodes from a list calling the list's deallocator callback for each node.
 * 
 * \param list The list to clear.
 */
void SGE_LLClear(SGE_LinkedList *list);

/**
 * \brief Adds a new data pointer at the end of a linked list.
 * 
 * \param list The list to add to.
 * \param data The pointer to add to the list.
 */
void SGE_LLPush(SGE_LinkedList *list, void *data);

/**
 * \brief Removes the last node from a linked list, calling the list's deallocator callback for that node.
 * 
 * \param list The list whose last node should be removed.
 */
void SGE_LLPop(SGE_LinkedList *list);

/**
 * \brief Removes the first node from a list, calling the list's deallocator callback for that node.
 * 
 * \param list The list whose first node should be removed.
 */
void SGE_LLPopFront(SGE_LinkedList *list);

/**
 * \brief Removes a given node from a linked list.
 * 
 * \param list The list to remove the node from.
 * \param node The address of the node to remove.
 */
void SGE_LLRemove(SGE_LinkedList *list, SGE_LLNode *node);

/**
 * \brief Returns the last element of a list.
 * 
 * \param list The list whose last element should be returned.
 * \return The data pointer stored in the last node of the list.
 */
void *SGE_LLGetLast(SGE_LinkedList *list);

/**
 * \brief Calls a callback function for each node in a linked list.
 * 
 * \param list The list to iterate over.
 * \param processFunc The callback function to call for each node.
 *                    It should return NULL if the iteration should continue for the next node
 *                    or a non NULL value to stop processing furthur.
 * \param data The data to pass to the callback function.
 * \return NULL if the entire list was processed or
 *         the return value of processFunc() if processFunc() returns non NULL for any node.
 */
void *SGE_LLProcess(SGE_LinkedList *list, void *(*processFunc)(SGE_LinkedList *list, SGE_LLNode* currentNode, void *processData), void *data);

void SGE_LLInsert(SGE_LinkedList *list, SGE_LLNode *insertAfter, void *data); // To be implemented
void SGE_LLPushFront(SGE_LinkedList *list, void *data); // To be implemented

#endif