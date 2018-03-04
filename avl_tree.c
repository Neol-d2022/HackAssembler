#include "avl_tree.h"
#include <stdlib.h>

static NODE* _insert(AVL_TREE *tree, NODE* root, NODE* newPtr, int* taller, int *success);
static NODE* _delete(AVL_TREE *tree, NODE* root, void *dltKey, int *shorter, int *success, int *freed);
static void* _retrieve(AVL_TREE *tree, void *keyPtr, NODE *root);
static void _traversal(NODE *root, void *paramInOut, void (*process)(void *dataPtr, void *paramInOut));
static void _destroy(NODE *root, void (*dataFreeMemFunc)(void *dataPtr));
static void _destroy_node_only(NODE* root);

static NODE* rotateLeft(NODE *root);
static NODE* rotateRight(NODE *root);
static NODE* insLeftBal(NODE *root, int *taller);
static NODE* insRightBal(NODE *root, int *taller);
static NODE* dltLeftBal(NODE *root, int *shorter);
static NODE* dltRightBal(NODE *root, int *shorter);

static NODE* _copy(NODE *root, void* (*dup)(void *dataPtr) , void (*dlt)(void *dataPtr));

AVL_TREE* AVL_Create(int (*compare)(void *arg1, void *arg2), void (*dataFreeMemFunc)(void *dataPtr)) {
	AVL_TREE *tree = (AVL_TREE*)malloc(sizeof(AVL_TREE));
	if(tree) {
		tree->root = 0;
		tree->count = 0u;
		tree->compare = compare;
		tree->dataFreeMemFunc = dataFreeMemFunc;
	}
	return tree;
}

int AVL_Insert(AVL_TREE *tree, void *dataInPtr) {
	NODE *newPtr = (NODE*)malloc(sizeof(NODE));
	int forTaller = 0;
	int success = 0;
	
	if(!newPtr) return 0;
	
	newPtr->bal = EH;
	newPtr->right = 0;
	newPtr->left = 0;
	newPtr->dataPtr = dataInPtr;
	
	tree->root = _insert(tree, tree->root, newPtr, &forTaller, &success);
	if(success) (tree->count) += 1;
	else {
		if(tree->dataFreeMemFunc) tree->dataFreeMemFunc(newPtr->dataPtr);
		free(newPtr);
	}
	return success;
}

static NODE* _insert(AVL_TREE *tree, NODE* root, NODE* newPtr, int* taller, int *success) {
	if(!root) {
		root = newPtr;
		*taller = 1;
		*success = 1;
		return root;
	}
	
	if(tree->compare(newPtr->dataPtr, root->dataPtr) < 0) {
		root->left = _insert(tree, root->left, newPtr, taller, success);
		if(!(*success)) return root;
		if(*taller) {
			switch(root->bal) {
				case LH:
					root = insLeftBal(root, taller);
					break;
				case EH:
					root->bal = LH;
					break;
				case RH:
					root->bal = EH;
					*taller = 0;
					break;
				default: abort();
			}
		}
	}
	else if(tree->compare(newPtr->dataPtr, root->dataPtr) > 0) {
		root->right = _insert(tree, root->right, newPtr, taller, success);
		if(!(*success)) return root;
		if(*taller) {
			switch(root->bal) {
				case LH:
					root->bal = EH;
					*taller = 0;
					break;
				case EH:
					root->bal = RH;
					break;
				case RH:
					root = insRightBal(root, taller);
					break;
				default: abort();
			}
		}
	}
	else *success = 0;
	return root;
}

static NODE* rotateLeft(NODE *root) {
	NODE *tempPtr = root->right;
	root->right = tempPtr->left;
	tempPtr->left = root;
	return tempPtr;	
}

static NODE* rotateRight(NODE *root) {
	NODE *tempPtr = root->left;
	root->left = tempPtr->right;
	tempPtr->right = root;
	return tempPtr;
}

static NODE* insLeftBal(NODE *root, int *taller) {
	NODE *rightTree;
	NODE *leftTree = root->left;
	
	switch(leftTree->bal) {
		case LH:
			root->bal = EH;
			leftTree->bal = EH;
			
			root = rotateRight(root);
			*taller = 0;
			break;
		case EH: abort();
		case RH:
			rightTree = leftTree->right;
			switch(rightTree->bal) {
				case LH:
					root->bal = RH;
					leftTree->bal = EH;
					break;
				case EH:
					root->bal = EH;
					leftTree->bal = EH;
					break;
				case RH:
					root->bal = EH;
					leftTree->bal = LH;
					break;
				default: abort();
			}
			
			rightTree->bal = EH;
			root->left = rotateLeft(leftTree);
			
			root = rotateRight(root);
			*taller = 0;
			break;
		default: abort();
	}
	
	return root;
}

static NODE* insRightBal(NODE *root, int *taller) {
	NODE *rightTree = root->right;
	NODE *leftTree;
	
	switch(rightTree->bal) {
		case LH:
			leftTree = rightTree->left;
			switch(leftTree->bal) {
				case LH:
					root->bal = EH;
					rightTree->bal = RH;
					break;
				case EH:
					root->bal = EH;
					rightTree->bal = EH;
					break;
				case RH:
					root->bal = LH;
					rightTree->bal = EH;
					break;
				default: abort();
			}
			
			leftTree->bal = EH;
			root->right = rotateRight(rightTree);
			
			root = rotateLeft(root);
			*taller = 0;
			break;
		case EH: abort();
		case RH:
			root->bal = EH;
			rightTree->bal = EH;
			
			root = rotateLeft(root);
			*taller = 0;
			break;
		default: abort();
	}
	
	return root;
}

int AVL_Delete(AVL_TREE *tree, void *dltKey) {
	int shorter = 0;
	int success = 0;
	int freed = 0;
	NODE *newRoot = _delete(tree, tree->root, dltKey, &shorter, &success, &freed);
	
	if(success) {
		tree->root = newRoot;
		(tree->count) -= 1;
	}
	return success;
}

static NODE *_delete(AVL_TREE *tree, NODE *root, void *dltKey, int *shorter, int *success, int *freed) {
	NODE *dltPtr;
	NODE *exchPtr;
	NODE *newRoot;
	
	if(!root) {
		*shorter = 0;
		*success = 0;
		return 0;
	}
	
	if(tree->compare(dltKey, root->dataPtr) < 0) {
		root->left = _delete(tree, root->left, dltKey, shorter, success, freed);
		if(*shorter)
			root = dltRightBal(root, shorter);
	}
	else if(tree->compare(dltKey, root->dataPtr) > 0) {
		root->right = _delete(tree, root->right, dltKey, shorter, success, freed);
		if(*shorter)
			root = dltLeftBal(root, shorter);
	}
	else {
		dltPtr = root;
		if(!root->right) {
			newRoot = root->left;
			*success = 1;
			*shorter = 1;
			if(tree->dataFreeMemFunc) {
				if(!*freed) {
					tree->dataFreeMemFunc(dltPtr->dataPtr);
					*freed = 1;
				}
			}
			free(dltPtr);
			return newRoot;
		}
		else if(!root->left) {
			newRoot = root->right;
			*success = 1;
			*shorter = 1;
			if(tree->dataFreeMemFunc) {
				if(!*freed) {
					tree->dataFreeMemFunc(dltPtr->dataPtr);
					*freed = 1;
				}
			}
			free(dltPtr);
			return newRoot;
		}
		else {
			exchPtr = root->left;
			while(exchPtr->right)
				exchPtr = exchPtr->right;
			if(tree->dataFreeMemFunc) {
				if(!*freed) {
					tree->dataFreeMemFunc(dltPtr->dataPtr);
					*freed = 1;
				}
			}
			root->dataPtr = exchPtr->dataPtr;
			root->left = _delete(tree, root->left, exchPtr->dataPtr, shorter, success, freed);
			if(*shorter)
				root = dltRightBal(root, shorter);
		}
	}
	return root;
}

static NODE* dltLeftBal(NODE *root, int *shorter) {
	NODE *rightTree;
	NODE *leftTree;
	
	switch(root->bal) {
		case LH:
			leftTree = root->left;
			if(leftTree->bal == RH) {
				rightTree = leftTree->right;
				switch(rightTree->bal) {
					case LH:
						leftTree->bal = EH;
						root->bal = RH;
						break;
					case EH:
						leftTree->bal = EH;
						root->bal = EH;
						break;
					case RH:
						leftTree->bal = LH;
						root->bal = EH;
						break;
					default: abort();
				}
				
				rightTree->bal = EH;
				
				root->left = rotateLeft(leftTree);
				root = rotateRight(root);
			}
			else {
				switch(leftTree->bal) {
					case LH:
						root->bal = EH;
						leftTree->bal = EH;
						break;
					case EH:
						root->bal = LH;
						leftTree->bal = RH;
						*shorter = 0;
						break;
					default: abort();
				}
				root = rotateRight(root);
			}
			break;
		case EH:
			root->bal = LH;
			*shorter = 0;
			break;
		case RH:
			root->bal = EH;
			break;
		default: abort();
	}
	
	return root;
}

static NODE* dltRightBal(NODE *root, int *shorter) {
	NODE *rightTree;
	NODE *leftTree;
	
	switch(root->bal) {
		case LH:
			root->bal = EH;
			break;
		case EH:
			root->bal = RH;
			*shorter = 0;
			break;
		case RH:
			rightTree = root->right;
			if(rightTree->bal == LH) {
				leftTree = rightTree->left;
				switch(leftTree->bal) {
					case LH:
						rightTree->bal = RH;
						root->bal = EH;
						break;
					case EH:
						rightTree->bal = EH;
						root->bal = EH;
						break;
					case RH:
						rightTree->bal = EH;
						root->bal = LH;
						break;
					default: abort();
				}
				
				leftTree->bal = EH;
				
				root->right = rotateRight(rightTree);
				root = rotateLeft(root);
			}
			else {
				switch(rightTree->bal) {
					case EH:
						root->bal = RH;
						rightTree->bal = LH;
						*shorter = 0;
						break;
					case RH:
						root->bal = EH;
						rightTree->bal = EH;
						break;
					default: abort();
				}
				root = rotateLeft(root);
			}
			break;
		default: abort();
	}
	
	return root;
}

void* AVL_Retrieve(AVL_TREE *tree, void *keyPtr) {
	if(tree->root)
		return _retrieve(tree, keyPtr, tree->root);
	else return 0;
}

static void* _retrieve(AVL_TREE *tree, void *keyPtr, NODE *root) {
	if(root) {
		if(tree->compare(keyPtr, root->dataPtr) < 0)
			return _retrieve(tree, keyPtr, root->left);
		else if(tree->compare(keyPtr, root->dataPtr) > 0)
			return _retrieve(tree, keyPtr, root->right);
		else return root->dataPtr;
	}
	else return 0;
}

void AVL_Traverse(AVL_TREE *tree, void *paramInOut, void (*process)(void *dataPtr, void *paramInOut)) {
	_traversal(tree->root, paramInOut, process);
	return ;
}

static void _traversal(NODE *root, void *paramInOut, void (*process)(void *dataPtr, void *paramInOut)) {
	if(root) {
		_traversal(root->left, paramInOut, process);
		process(root->dataPtr, paramInOut);
		_traversal(root->right, paramInOut, process);
	}
	return ;
}

int AVL_Empty(AVL_TREE *tree) {
	return (tree->count == 0) ? 1 : 0;
}

int AVL_Full() {
	NODE *newPtr = (NODE*)malloc(sizeof(NODE));
	if(newPtr) {
		free(newPtr);
		return 1;
	}
	else return 0;
}

unsigned int AVL_Count(AVL_TREE *tree) {
	return tree->count;
}

AVL_TREE* AVL_Destroy(AVL_TREE *tree) {
	if(tree) {
		if(tree->dataFreeMemFunc) _destroy(tree->root, tree->dataFreeMemFunc);
		else _destroy_node_only(tree->root);
	}
	
	free(tree);
	return 0;
}

static void _destroy(NODE *root, void (*dataFreeMemFunc)(void *dataPtr)) {
	if(root) {
		_destroy(root->left, dataFreeMemFunc);
		dataFreeMemFunc(root->dataPtr);
		_destroy(root->right, dataFreeMemFunc);
		free(root);
	}
	return ;
}

static void _destroy_node_only(NODE *root) {
	if(root) {
		_destroy_node_only(root->left);
		_destroy_node_only(root->right);
		free(root);
	}
	return ;
}

_treeCmpFuncT AVL_CompareFunc(AVL_TREE *tree) {
	return tree->compare;
}

_treeDataFreeMemFunc AVL_MemFreeFunc(AVL_TREE *tree) {
	return tree->dataFreeMemFunc;
}

static NODE* _copy(NODE *root, void* (*dup)(void *dataPtr) , void (*dlt)(void *dataPtr)) {
	NODE *ret = (NODE*)malloc(sizeof(NODE));
	if(!ret) return 0;
	ret->dataPtr = dup(root->dataPtr);
	if(!(ret->dataPtr)) {
		free(ret);
		return 0;
	}
	ret->left = _copy(root->left, dup, dlt);
	if(!(ret->left)) {
		dlt(ret->dataPtr);
		free(ret);
		return 0;
	}
	ret->right = _copy(root->right, dup, dlt);
	if(!(ret->right)) {
		if(dlt) _destroy(ret->left, dlt);
		else _destroy_node_only(ret->left);
		dlt(ret->dataPtr);
		free(ret);
		return 0;
	}
	ret->bal = root->bal;
	return ret;
}

AVL_TREE* AVL_Copy(AVL_TREE *tree, void* (*dup)(void *dataPtr)) {
	AVL_TREE *ret = (AVL_TREE*)malloc(sizeof(AVL_TREE));
	if(!ret) return 0;
	ret->root = _copy(tree->root, dup, tree->dataFreeMemFunc);
	if(!(ret->root)) {
		free(ret);
		return 0;
	}
	ret->compare = tree->compare;
	ret->count = tree->count;
	ret->dataFreeMemFunc = tree->dataFreeMemFunc;
	return ret;
}
