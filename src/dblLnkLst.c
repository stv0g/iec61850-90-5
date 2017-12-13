/****************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *********************************************/
/****************************************************************************/
/* (c) Copyright 2012 Systems Integration Specialists Company, Inc. (SISCO) */
/* 6605 19 1/2 Mile Road, Sterling Heights, Michigan, 48314 USA		    */
/* Tel: +1-586-254-0020   http://www.sisconet.com			    */
/*									    */
/* This Work consists of voluntary contributions made by SISCO and 	    */
/* individuals on behalf of SISCO and then contributed to the 		    */
/* community in support of furthering market acceptance of 		    */
/* IEC TR 61850-90-5. Please support the community by submitting 	    */
/* your modifications and contributions at:				    */
/*									    */
/* http://iec61850.ucaiug.org/90-5/default.aspx				    */
/* 									    */
/* Licensed under the Apache License, Version 2.0 (the "License"); you may  */
/* not use this Work except in compliance with the License.		    */
/*									    */
/* You may obtain a copy of the License at				    */
/*									    */
/* http://www.apache.org/licenses/LICENSE-2.0				    */
/*									    */
/* A copy of the License is included in the distribution of this Work	    */
/* in the file named “90-5_License.txt”.				    */
/*									    */
/* Unless required by applicable law or agreed to in writing, software 	    */
/* distributed under the License is distributed on an "AS IS" BASIS,	    */
/* WITHOUT, WARRANTIES OR CONDITIONS OF ANY KIND, either express or 	    */
/* implied. See the License for the specific language governing  	    */
/* permissions andlimitations under the License.			    */
/*									    */
/* MODULE NAME : dblLnkLst.c						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Functions to manipulate Double Linked Lists			    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who	   Comments					    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
/****************************************************************************/

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
/*				dblLnkUnlink				*/
/*									*/
/* Removes a member from the double linked list				*/
/************************************************************************/

static int dblLnkUnlink (DBL_LNK **list_head_ptr, DBL_LNK *node_ptr)
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


