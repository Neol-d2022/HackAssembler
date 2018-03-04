#ifndef AVL_H_LOADED
#define AVL_H_LOADED

#define LH +1
#define EH  0
#define RH -1

//Structure Declarations

//Compare function
typedef int (*_treeCmpFuncT)(void *arg1, void *arg2);

//Data memory release function
typedef void (*_treeDataFreeMemFunc)(void *dataPtr);

//Function for duplicating data
typedef void* (*_treeDataDupFunc)(void *dataPtr);

//Tree node
typedef struct node
{
	void 		*dataPtr;	// Data pointer
	struct node *left;		// Left tree pointer
	struct node *right;		// Right tree pointer
	int 		 bal;		// Balance number, height of left tree - height of right tree
} NODE ;

//Tree head
typedef struct {
	_treeCmpFuncT compare;	//Data comparison function
	_treeDataFreeMemFunc dataFreeMemFunc;	//Data memory release function
	NODE *root;								//Tree root node pointer
	unsigned int count;						//Number of nodes
} AVL_TREE ;


//Prototype Declarations

/*AVL_Create
  Description:
    Create a tree head.
    
  Input:
    int (*compare)(void *arg1, void *arg2)
      [Required] Data comparison function
        It is used when inserting and retrieving data.

    void (*dataFreeMemFunc)(void *dataPtr)
      [Optional] Data memory release function
        Default is NULL
      	It is used when deleting nodes and destroying a tree that contains nodes.
      	If it is NULL, data pointer is not handled when deleting a node.
  
  Output:
    Returns NULL when memory not available.
    Else returns a tree heads.
*/
AVL_TREE* AVL_Create(int (*compare)(void *arg1, void *arg2), void (*dataFreeMemFunc)(void *dataPtr));

/*AVL_Destroy
  Description:
    Destroy a tree head.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head to be destroy
  
  Output:
    Returns NULL.
*/
AVL_TREE* AVL_Destroy(AVL_TREE *tree);

/*AVL_Insert
  Description:
    Insert a new node into a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head of a tree with a new node to be inserted.

    void *dataInPtr
      [Required] A data pointer that is going to be inserted.
  
  Output:
    Returns 0 when a duplicate node has already existed.
    Returns 1 on success
*/
int AVL_Insert(AVL_TREE *tree, void *dataInPtr);

/*AVL_Delete
  Description:
    Delete a node from a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head of a tree with a node to be deleted.

    void *dltKey
      [Required] A data pointer that is going to be deleted.
  
  Output:
    Returns 0 when the provided data node is not found.
    Returns 1 on success
*/
int AVL_Delete(AVL_TREE *tree, void *dltKey);

/*AVL_Retrieve
  Description:
    Retrieve a node from a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head of a tree with a node to be retrieved.

    void *dltKey
      [Required] A data pointer that is going to be retrieved.
  
  Output:
    Returns 0 when the provided data node is not found.
    Returns the data pointer when found.
*/
void* AVL_Retrieve(AVL_TREE *tree, void *keyPtr);

/*AVL_Traverse
  Description:
    Traverse a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head of a tree which is going to be traversed.

    void (*process)(void *dataPtr)
      [Required] A function that process all data pointers in the tree.
  
  Output: No output.
*/
void AVL_Traverse(AVL_TREE *tree, void *paramInOut, void (*process)(void *dataPtr, void *paramInOut));

/*AVL_Count
  Description:
    Returns the number of data nodes in a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head.

  Output:
    Return the number of data nodes in the tree
*/
unsigned int AVL_Count(AVL_TREE *tree);

/*AVL_Empty
  Description:
    Check whether a tree is empty or not.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head.

  Output:
    Return 1 if the tree is empty.
    Return 0 if not.
*/
int AVL_Empty(AVL_TREE *tree);

/*AVL_Full
  Description:
    Check whether there is a room for another data node or not.
    
  Input: No Input

  Output:
    Return 1 if there is no room.
    Return 0 if memory available.
*/
int AVL_Full();

/*AVL_CompareFunc
  Description:
    Returns the comparison function that currently set to a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head.

  Output:
    Returns the comparison function that currently setted to the tree.
*/
_treeCmpFuncT AVL_CompareFunc(AVL_TREE *tree);

/*AVL_MemFreeFunc
  Description:
    Returns the memory release function that currently set to a tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head.

  Output:
    Returns the memory release function that currently setted to the tree.
    Returns NULL when not set.
*/
_treeDataFreeMemFunc AVL_MemFreeFunc(AVL_TREE *tree);

/*AVL_Copy
  Description:
    Copy a AVL tree.
    
  Input:
    AVL_TREE *tree
      [Required] A tree head.
      
    void* (*dup)(void *dataPtr)
      [Required] Function for duplicating data

  Output:
    Returns a copy of the tree.
    Returns NULL when memory overflows.
*/
AVL_TREE* AVL_Copy(AVL_TREE *tree, void* (*dup)(void *dataPtr));

#endif
