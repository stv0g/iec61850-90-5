/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : dblLnkLst.c						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	Functions to manipulate Double Linked Lists			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*				udp_tx					*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who	   Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/************************************************************************/
/************************************************************************/

#include <windows.h>
#include <stdio.h>
#include "dblLnkLst.h"

/************************************************************************/

static CRITICAL_SECTION  _dblLnkCriticalSection;	//mutex

//*********  Example of using Mutex *************************
/*void x ()
  {
  InitializeCriticalSection(&_dblLnkCriticalSection);
  EnterCriticalSection(&_dblLnkCriticalSection);
  LeaveCriticalSection(&_dblLnkCriticalSection);
  }
*/

/************************************************************************/
/*				dblLnkAddFirst				*/
/*									*/
/* Adds a member as the first member in the double linked list		*/
/************************************************************************/

int dblLnkAddFirst (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
  {
DBL_LNK *list_tail_ptr;

  if (node_ptr == NULL)
    return (DBL_LNK_FAILURE);

 /* Will this be the only mode? */
  if (*list_head_ptr == NULL)
    {
    node_ptr->next = node_ptr;
    node_ptr->prev = node_ptr;
    }
  else
    {
    list_tail_ptr = (*list_head_ptr)->prev;
    node_ptr->next = *list_head_ptr;
    node_ptr->prev = list_tail_ptr;
    list_tail_ptr->next = node_ptr;
    (*list_head_ptr)->prev = node_ptr;
    }

/* assign the new head of list	*/
  *list_head_ptr = node_ptr;
  return (DBL_LNK_SUCCESS);
  }

/************************************************************************/
/*				dblLnkAddLast				*/
/*									*/
/* Adds a member as the last member in the double linked list		*/
/************************************************************************/

int dblLnkAddLast (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
  {
DBL_LNK *list_tail_ptr;

  if (node_ptr == NULL)
    return (DBL_LNK_FAILURE);

 /* Will this be the only mode? */
  if (*list_head_ptr == NULL)
    {
    node_ptr->next = node_ptr;
    node_ptr->prev = node_ptr;
    *list_head_ptr = node_ptr;
    }
  else
    {
    list_tail_ptr = (*list_head_ptr)->prev;
    list_tail_ptr->next = node_ptr;
    node_ptr->prev = list_tail_ptr;
    node_ptr->next = *list_head_ptr;
    (*list_head_ptr)->prev = node_ptr;
    }
  return (DBL_LNK_SUCCESS);
  }


/************************************************************************/
/*				dblLnkUnlink				*/
/*									*/
/* Removes a member from the double linked list				*/
/************************************************************************/

int dblLnkUnlink (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
  {
  if (*list_head_ptr == NULL) 
    return (DBL_LNK_FAILURE);

  if (node_ptr == NULL)
    return (DBL_LNK_FAILURE);

/* See if only node in list */
  if ((node_ptr->next == *list_head_ptr) &&  (node_ptr == *list_head_ptr))
    *list_head_ptr = NULL;
  else 
    {					       
  /* If first node in list need to change list_head */    
    if (node_ptr == *list_head_ptr)    	        
      *list_head_ptr = node_ptr->next;

    (node_ptr->next)->prev = node_ptr->prev;	/* link next to prev */
    (node_ptr->prev)->next = node_ptr->next;	/* link prev to next */
    }
  return (DBL_LNK_SUCCESS);
  }
   

/************************************************************************/
/*				dblLnkUnlinkFirst			*/
/*									*/
/* Removes the first member in the double linked list			*/
/************************************************************************/

DBL_LNK *dblLnkUnlinkFirst (DBL_LNK **list_head_ptr)
  {
DBL_LNK *node_ptr;

  if (*list_head_ptr == NULL)
    return (NULL);

  node_ptr = *list_head_ptr;
  dblLnkUnlink (list_head_ptr, node_ptr);
  return (node_ptr);
  }

/************************************************************************/
/*				dblLnkUnlinkLast			*/
/*									*/
/* Removes the last member in the double linked list			*/
/************************************************************************/

DBL_LNK *dblLnkUnlinkLast (DBL_LNK **list_head_ptr)
  {
DBL_LNK *node_ptr;

  if (*list_head_ptr == NULL)
    return (NULL);

  node_ptr = *list_head_ptr;
  node_ptr = node_ptr->prev;
  dblLnkUnlink (list_head_ptr, node_ptr);
  return (node_ptr);
  }

#if 0
/************************************************************************/
/*		           dblLnkFind					*/
/************************************************************************/

int dblLnkFind (DBL_LNK *list_head_ptr, DBL_LNK *node_ptr)
  {
DBL_LNK	  *temp_ptr;
DBL_LNK	  *list_tail_ptr;
int	ret_code;
     
  if ((list_head_ptr == NULL) || (node_ptr == NULL))
    return (DBL_LNK_FAILURE);

  temp_ptr = list_head_ptr;
  list_tail_ptr = list_head_ptr->prev;

  while ((temp_ptr != list_tail_ptr) && (temp_ptr != node_ptr))
    temp_ptr = temp_ptr->next;

  if (temp_ptr == node_ptr)
    ret_code = DBL_LNK_SUCCESS;
  else
    ret_code = DBL_LNK_FAILURE;

  return (ret_code);
  }

/************************************************************************/
/*   	           dblLnkAddAfter					*/
/*									*/
/* Removes the first member in the double linked list			*/
/************************************************************************/

int dblLnkAddAfter (DBL_LNK *cur_node, DBL_LNK *new_node)
  {
DBL_LNK	*next_node;
int	ret_val;

  ret_val = DBL_LNK_FAILURE;
  if ((new_node == NULL) || (cur_node == NULL))
    return (DBL_LNK_FAILURE);

  next_node = cur_node -> next;
  new_node -> next = next_node;
  new_node -> prev = cur_node;
  next_node -> prev = new_node;
  cur_node -> next = new_node;
  return (DBL_LNK_SUCCESS);
  }    

/************************************************************************/
/*		           dblLnkFindNext				*/
/************************************************************************/

DBL_LNK *dblLnkFindNext (DBL_LNK *list_head_ptr, DBL_LNK *cur_node)
  {
DBL_LNK *next_node = NULL;

  if ((list_head_ptr == NULL) || (cur_node == NULL))
    return (NULL);

  if (cur_node->next != list_head_ptr)
    next_node = cur_node->next;

  return (next_node);
  }

/************************************************************************/
/*		           dblLnkSize				*/
/************************************************************************/

int dblLnkSize (DBL_LNK *list_head_ptr)
  {
int count;
DBL_LNK *cur_node;

  count = 0;
  cur_node = list_head_ptr;

  while (cur_node != NULL) 
    {
    count ++;
    cur_node = (DBL_LNK *) dblLnkFindNext (list_head_ptr, cur_node);
    }

  return (count);
  }

/************************************************************************/
/*		           dblLnkFindPrev				*/
/************************************************************************/

DBL_LNK *dblLnkFindPrev (DBL_LNK *list_head_ptr, DBL_LNK *cur_node)
  {
DBL_LNK *prev_node = NULL;

  if ((list_head_ptr == NULL) || (cur_node == NULL))
    return (NULL);

  if (cur_node != list_head_ptr)
    prev_node = cur_node->prev;

  return (prev_node);
  }

/************************************************************************/
/*				dblLnkFindLast				*/
/************************************************************************/

DBL_LNK *dblLnkFindLast (DBL_LNK *list_head_ptr)
  {
DBL_LNK *last_node;

  if (list_head_ptr)
    last_node = list_head_ptr->prev;
  else
    last_node = NULL;

  return (last_node);
  }
#endif

