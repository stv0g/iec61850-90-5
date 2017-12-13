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
/* MODULE NAME : iec_90_5_kdc_int.c					    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Provides indirection functions for KDC and key			    */
/*	management.							    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who	   Comments					    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision		  		    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"

//just used for testing until the real interface to a KDC is provided
static ST_UCHAR dummy_test_key1[] = {
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88};

static ST_UCHAR dummy_test_key2[] = {
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x02,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88};

static ST_ULONG dummy_key_id = 0;

static ST_UINT32 refNum=0;	      //used to generate KDC pair reference numbers (just incremented)

typedef struct kdc_info{
  ST_BOOLEAN inUse;		      //flag to indicate that the entry is valid (if TRUE)
  ST_UINT32   kdcRefNum;	      //an internally generated reference number
  char *pUserReference;		      //pointer to a user assigned reference name that can be used in a lookup
  IEC_COMM_ADDRESS *pPrimary;	      //addressing information for the primary KDC
  KDC_CREDENTIALS *pPrimaryCredentials;   //pointer to the primary KDCs credentials that need to be used in phase 1 of GDOI 
  IEC_COMM_ADDRESS  *pSecondary;	      //addressing information for the secondary KDC
  KDC_CREDENTIALS *pSecondaryCredentials;   //pointer to the secondary KDCs credentials that need to be used in phase 1 of GDOI  
}KDC_INFO;

static KDC_CREDENTIALS localApplicationCredentials;
static int numAllowedKDCPairs=0;
static KDC_INFO *pKDCInfoStorage;			    //pointer to an allocated array that will be used to store the KDC information
static ST_UINT32 KDCPairRefNum=1;


/************************************************************************/
/* This function is a dummy function so that implementations can	*/
/* occur prior to the real KDC being integrated				*/
/************************************************************************/
static int dummy_kdc_key_function(IEC905_MSG_CNTRL * pMsgCntrl)
{
ST_ULONG keyID;
IEC905_KEY_INFO *retVal;
ST_UINT32 detectedKeyID;



    if(pMsgCntrl->pCurrentKey==NULL)
    {
      keyID=1;
      if((retVal = iec905_add_current_key(pMsgCntrl, KEY_TYPE_AES_128, sizeof(dummy_test_key1), dummy_test_key1,  (ST_UCHAR *)&keyID, 120))==NULL)
        return(IEC905_ErrorHandler(UNABLE_TO_ADD_KEY,__FILE__,__LINE__));

      keyID=2;
      if((retVal = iec905_add_next_key(pMsgCntrl, KEY_TYPE_AES_128, sizeof(dummy_test_key2), dummy_test_key2,  (ST_UCHAR *)&keyID, 240))==NULL)
        return(IEC905_ErrorHandler(UNABLE_TO_ADD_KEY,__FILE__,__LINE__));
    }
   else
    {
      detectedKeyID = *(ST_UINT32 *)pMsgCntrl->pCurrentKey->key_id;
      if(detectedKeyID==2)
       {
       keyID=1;
       if((retVal = iec905_add_next_key(pMsgCntrl, KEY_TYPE_AES_128, sizeof(dummy_test_key1), dummy_test_key1,  (ST_UCHAR *)&keyID, 240))==NULL)
        return(IEC905_ErrorHandler(UNABLE_TO_ADD_KEY,__FILE__,__LINE__));
       }
     else
       {
      keyID=2;
      if((retVal = iec905_add_next_key(pMsgCntrl, KEY_TYPE_AES_128, sizeof(dummy_test_key2), dummy_test_key2,  (ST_UCHAR *)&keyID, 240))==NULL)
        return(IEC905_ErrorHandler(UNABLE_TO_ADD_KEY,__FILE__,__LINE__));
      }
    } 

    pMsgCntrl->requestedUpdate = FALSE;
    return(SUCCESS_IEC905);

}

/************************************************************************/
/*				iec905_get_kdc_tx_keys			*/
/* this function is a placeholder and allows special API code to	*/
/* to be added if the KDC for the application is local			*/
/* (e.g. no communication may be required				*/
/************************************************************************/
int iec905_get_kdc_tx_keys(IEC905_MSG_CNTRL * pMsgCntrl)	//pointer the the information that contains the dest & src address and DataSet info needed for the KDC
{
    return(dummy_kdc_key_function(pMsgCntrl));
}


/************************************************************************/
/*				iec905_get_kdc_rx_keys			*/
/* this function is a placeholder and will need to be updated once	*/
/* tthe KDC code is integrated				  		*/
/************************************************************************/
int get_kdc_rx_keys(IEC905_MSG_CNTRL * pMsgCntrl )	    //pointer the the information that contains the dest & src address and DataSet info needed for the KDC
		       
{

  return(dummy_kdc_key_function(pMsgCntrl));
}


/************************************************************************/
/* 		iec905_init_kdc_interface				*/
/*									*/
/*  function initializes information needed for the interaction		*/
/*  with the KDC to get tx oriented keys				*/
/*									*/
/*  Inputs:								*/
/*    pLocalCredential - pointer to local credential information for	*/
/*			 the application				*/
/*									*/
/* Return: SUCCES or ErroCode.						*/
/************************************************************************/
int iec905_init_kdc_interface(KDC_CREDENTIALS *pLocalCredential	, int maxNumOfKDCPairs)
 {
      
      if(pLocalCredential==NULL)
	return(IEC905_ErrorHandler( INVALID_INPUT_PARAMETER,__FILE__,__LINE__));

      //may need to perform validation on the credentials here, once the actual credentials are known

      //store away the credentials that were passed in so the original can be destroyed.
      memcpy(&localApplicationCredentials,pLocalCredential,sizeof(KDC_CREDENTIALS));

      pKDCInfoStorage = (KDC_INFO *)calloc(sizeof(KDC_INFO), maxNumOfKDCPairs);
      numAllowedKDCPairs = maxNumOfKDCPairs;
      return(SUCCESS_IEC905);
  }




/************************************************************************/
/* 		iec905_destroy_kdc_credential				*/
/*									*/
/*  function destroys previously created KDC credential information	*/
/*									*/
/*  Inputs:								*/
/*	Credential to be destroyed					*/
/*    									*/
/* Return: SUCCESS or an Error code			  		*/
/************************************************************************/
int iec905_destroy_kdc_credential(KDC_CREDENTIALS *pCredentialToDestroy)
{
      //may need to do more eventually

      free(pCredentialToDestroy);
      return(SUCCESS_IEC905);
}

/************************************************************************/
/* 		iec905_create_kdc_pair					*/
/*									*/
/*  function allocates internal information regarding primary and	*/
/*  secondary KDCs that would be utilized to obtain keys		*/
/*  Inputs:								*/
/*	UserReference: a pointer to a NULL terminated string		*/
/*	Primary and Secondary Credential Information			*/
/*	Primary and Secondary Communication Addressing Information	*/
/*		  							*/
/*    									*/
/* Return: NULL or a pointer to a Referenc Structure			*/
/************************************************************************/
KDC_REF *iec905_create_KDC_pair(char *pUserRef, 
			    IEC_COMM_ADDRESS *pPrimary,	      //addressing information for the primary KDC
			    KDC_CREDENTIALS *pPrimaryCredentials,   //pointer to the primary KDCs credentials that need to be used in phase 1 of GDOI 
			    IEC_COMM_ADDRESS  *pSecondary,	      //addressing information for the secondary KDC
			     KDC_CREDENTIALS *pSecondaryCredentials   //pointer to the secondary KDCs credentials that need to be used in phase 1 of GDOI
			     )
{
int i;
KDC_INFO *pMyKDCInfo=NULL;
KDC_REF *pRetVal;

    if((pPrimary==NULL) || (pPrimaryCredentials==NULL))		//if no primary is specified, this is an error
    {
      IEC905_ErrorHandler(INVALID_INPUT_PARAMETER,__FILE__,__LINE__);
      return(NULL);
    }

    //now need to find an empty slot in the storage array
    for(i=0;i<numAllowedKDCPairs;++i)
    {
      pMyKDCInfo = &pKDCInfoStorage[i];
      if(pMyKDCInfo->inUse==FALSE)
	break;
    }

    if(pMyKDCInfo==NULL)
    {
      IEC905_ErrorHandler(INVALID_INPUT_PARAMETER,__FILE__,__LINE__);
      return(NULL);
    }
    
    if(i==numAllowedKDCPairs)	    //didn't find a slot
    {
      IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
      return(NULL);
    }

    //now can fill in stuff
    pMyKDCInfo->inUse = TRUE;
    
    ++KDCPairRefNum;
    pMyKDCInfo->kdcRefNum = KDCPairRefNum;
    
    if(pUserRef)
    {
      if((pMyKDCInfo->pUserReference = calloc(1,(strlen(pUserRef)+1)))!=NULL)
	strcpy(pMyKDCInfo->pUserReference,pUserRef);
      else
       {
        IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
        return(NULL);
       }
    }
    else
    {
      pMyKDCInfo->pUserReference=NULL;
    }

    //now fill in the primary information
     if((pMyKDCInfo->pPrimaryCredentials = calloc (1, sizeof(KDC_CREDENTIALS)))!=NULL)
      {
       if((pMyKDCInfo->pPrimary = calloc(1,sizeof(IEC_COMM_ADDRESS)))!=NULL)
	{
         memcpy(pMyKDCInfo->pPrimaryCredentials,pPrimaryCredentials,sizeof(KDC_CREDENTIALS));
         memcpy(pMyKDCInfo->pPrimary,pPrimary,sizeof(IEC_COMM_ADDRESS));
	}
      else
        {
	IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
	free(pMyKDCInfo->pPrimaryCredentials);
	pMyKDCInfo->pPrimaryCredentials=NULL;
	}
      }
    else
      {
      IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
      }

    if(pMyKDCInfo->pPrimaryCredentials==NULL)
      return(NULL);

    //now see if a secondary was specified
    if((pSecondary!=NULL) && (pSecondaryCredentials!=NULL))
    {
     if((pMyKDCInfo->pSecondaryCredentials = calloc (1, sizeof(KDC_CREDENTIALS)))!=NULL)
      {
       if((pMyKDCInfo->pSecondary = calloc(1,sizeof(IEC_COMM_ADDRESS)))!=NULL)
         {
	 memcpy(pMyKDCInfo->pSecondaryCredentials,pPrimaryCredentials,sizeof(KDC_CREDENTIALS));
         memcpy(pMyKDCInfo->pSecondary,pPrimary,sizeof(IEC_COMM_ADDRESS));
	 }
       else
	 {
	 IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
	 free(pMyKDCInfo->pSecondaryCredentials);
	 pMyKDCInfo->pSecondaryCredentials =NULL;
	 }
      }
    else
      {
      IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
      }
    }
    else
    {
      pMyKDCInfo->pSecondary=NULL;
      pMyKDCInfo->pSecondaryCredentials=NULL;
    }



    if((pRetVal = calloc(sizeof(KDC_REF),1))!=NULL)
      {
      pRetVal->pReserved = (void *)pMyKDCInfo;
      pRetVal-> refNum= pMyKDCInfo->kdcRefNum;
      pRetVal->pUserRef= pMyKDCInfo->pUserReference;
      }
    else
      {
      IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
      if(pMyKDCInfo->pPrimary!=NULL)
	free(pMyKDCInfo->pPrimary);

      free(pMyKDCInfo->pPrimaryCredentials);

      if(pMyKDCInfo->pSecondaryCredentials!=NULL)
	{
	if( pMyKDCInfo->pSecondary!=NULL)
	  free( pMyKDCInfo->pSecondary);
        free(pMyKDCInfo->pSecondaryCredentials);
	}
      }
   
    return(pRetVal);
}

/************************************************************************/
/* 		iec905_destroy_KDC_pair					*/
/*									*/
/*  function allocates and initializes the credential information	*/
/*  Inputs:	 None							*/
/*									*/
/* Return: NULL or a pointer to a credential structure			*/
/************************************************************************/
int iec905_destroy_KDC_pair(ST_UINT32 refNum)
{
  int i;
  KDC_INFO *pMyKDCInfo;

  for(i=0;i<numAllowedKDCPairs;++i)
  {
    pMyKDCInfo = &pKDCInfoStorage[i];
    if(pMyKDCInfo->kdcRefNum==refNum) 
    {
      //then we have something to free
      pMyKDCInfo->inUse=FALSE;
      pMyKDCInfo->kdcRefNum = 0;
      free(pMyKDCInfo->pPrimary);
      pMyKDCInfo->pPrimary=NULL;
      free(pMyKDCInfo->pPrimaryCredentials);
      pMyKDCInfo->pPrimaryCredentials=NULL;
      if(pMyKDCInfo->pSecondary)
      {
	free(pMyKDCInfo->pSecondary);
	pMyKDCInfo->pSecondary=NULL;
      }

      if(pMyKDCInfo->pSecondaryCredentials)
      {
	free(pMyKDCInfo->pSecondaryCredentials);
	pMyKDCInfo->pSecondaryCredentials=NULL;
      }
 
      if(pMyKDCInfo->pUserReference)
      {
	free(pMyKDCInfo->pUserReference);
	pMyKDCInfo->pUserReference=NULL;
      }
    }
  }
  return(SUCCESS_IEC905);
}


/************************************************************************/
/* 		iec905_kdc_bind	    					*/
/*									*/
/*  function copies/initializes relevant KDC information into	  	*/
/*  a MSG_CNTRL structure so that the structure can be used to		*/
/*  interact with the appropriate KDC					*/
/*									*/
/*  Inputs:	 pointer to MSG_CTRL structure				*/							
/*		 pointer to appropriate KDC Pair information	  	*/
/* Return: Success or an error code		  			*/
/************************************************************************/
int iec905_kdc_bind (IEC905_MSG_CNTRL *pMsgCntrl, KDC_REF *pKDCRef)
{
KDC_INFO *pmyKDCInfo;

    //check that the KDC_REF is still valid
    pmyKDCInfo = (KDC_INFO *)pKDCRef->pReserved;
    if(((pmyKDCInfo->inUse==FALSE) || (pmyKDCInfo->kdcRefNum!=pKDCRef->refNum))||(pMsgCntrl==NULL))
    {
      return(IEC905_ErrorHandler(INVALID_INPUT_PARAMETER,__FILE__,__LINE__));
    }


    pMsgCntrl->kdcRefNum = pKDCRef->refNum;
    pMsgCntrl->pKDCReserved = pKDCRef->pReserved;
    return(SUCCESS_IEC905);
}