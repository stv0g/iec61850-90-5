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
/*									    */
/* MODULE NAME : dblLnkLst.h						    */
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
#ifndef DBL_LNK_INCLUDED
#define DBL_LNK_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
#define DBL_LNK_SUCCESS 0
#define DBL_LNK_FAILURE 1

/************************************************************************/

typedef struct dbl_lnk
  {
  struct dbl_lnk *next;
  struct dbl_lnk *prev;
  } DBL_LNK;

/************************************************************************/

int	 dblLnkAddFirst (DBL_LNK **listHead, DBL_LNK *newNode);
int	 dblLnkAddLast  (DBL_LNK **listHead, DBL_LNK *newNode);	

DBL_LNK *dblLnkUnlinkFirst (DBL_LNK **listHead);		
DBL_LNK *dblLnkUnlinkLast  (DBL_LNK **listHead);


#ifdef __cplusplus
}
#endif

#endif
