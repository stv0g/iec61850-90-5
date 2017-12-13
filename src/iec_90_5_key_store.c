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
/* MODULE NAME : iec_90_5_key_store.c					    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Key rotation and lookup module					    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "time.h"
#include "iec_90_5.h"

/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

typedef struct iec_chain_info{
  IEC905_KEY_INFO *pStartKeyInfo;			      //linked list of key info structures so that KEY_IDs can be found easily
  IEC905_MSG_CNTRL *pIpV4Storage;			      //linked list of IpV4 key stores so that look-up based upon addressing can occur easily
  IEC905_MSG_CNTRL *pIpV6Storage;			      //linked list of IpV6 key stores so that look-up based upon addressing can occur easily
//  IEC905_MSG_CNTRL *pEthernetMAC;			      //linked list of EthernetMac key stores so that look-up based upon addressing can occur easily (not used currently )- for 62351-6
}IEC_CHAIN_INFO;

typedef struct iec_90_5_key_chains{
  IEC_CHAIN_INFO GooseChain;
  IEC_CHAIN_INFO SVChain;
  IEC_CHAIN_INFO TunnelChain;
}IEC_90_5_KEY_CHAINS;

static IEC_90_5_KEY_CHAINS keyChains;		  /*stores the memory resident keys in chains	*/
static int intialized=FALSE;

/************************************************************************************/
/*					init_key_chain				    */
/* must be called upon power-up							    */
/************************************************************************************/
static void init_key_chain (IEC_CHAIN_INFO *chain)
{
  chain->pIpV4Storage = NULL;
  chain->pIpV6Storage = NULL;
  chain->pStartKeyInfo= NULL;
}



/******************************************************************************************************/
/******************************************************************************************************/
/*	      The following section are internal helper functions				      */
/******************************************************************************************************/
/******************************************************************************************************/

/************************************************************************************/
/*					find_chain_head				    */
/* returns the head of a Key Storage chain based upon the usage (GOOSE, SV, Tunnel) */
/* internal helper function only						    */
/************************************************************************************/

static IEC_CHAIN_INFO *find_chain_head(ST_UINT8 usageType)
{
  IEC_CHAIN_INFO *pChainTemp=NULL;

  switch(usageType)
  {
    case IEC_KEY_USAGE_TYPE_SV:
      pChainTemp= &keyChains.SVChain;
      break;

    case IEC_KEY_USAGE_TYPE_GOOSE:
      pChainTemp = &keyChains.GooseChain;
      break;

    case IEC_KEY_USAGE_TYPE_TUNNEL:
      pChainTemp = &keyChains.TunnelChain;
      break;

    default:
      break;
  }
  
  return(pChainTemp);
}


/************************************************************************************/
/*					find_key_storage_head			    */
/* returns the head of a Key Storage head based upon the address type		    */
/* internal helper function only						    */
/************************************************************************************/
static IEC905_MSG_CNTRL **find_key_storage_head(IEC_CHAIN_INFO *keyChain, ST_UINT8 addressType)
{

IEC905_MSG_CNTRL **pTempStore=NULL;

  switch(addressType)
  {
    case IP_V4_ADDRESS_TYPE:
	pTempStore = &keyChain->pIpV4Storage;
	break;

    case IP_V6_ADDRESS_TYPE:
	pTempStore = &keyChain->pIpV6Storage;

    case ETHERNET_MAC_ADDRESS_TYPE:
    default:
	break;
  }

  return(pTempStore);
}

/************************************************************************************/
/*					find_key_storage			    */
/* returns the head of a Key Storage head based upon the address type		    */
/* internal helper function only						    */
/************************************************************************************/
static IEC905_MSG_CNTRL *find_key_storage(IEC_CHAIN_INFO *keyChain,  IEC_COMM_ADDRESS *pAddress, char *pDatSetRef)
{

IEC905_MSG_CNTRL **pTempStoreHead=NULL;
IEC905_MSG_CNTRL *pNext;

  
  pTempStoreHead = find_key_storage_head(keyChain, pAddress->typeOfAddress);
  if(pTempStoreHead == NULL)
   return(NULL);

  if (*pTempStoreHead == NULL)
    return(NULL);

  //now need to search the chain for the appropriate Storage Information
  pNext = *pTempStoreHead;
  while(pNext!=NULL)
    {
    if( (strcmp(pNext->pDataSetRef,pDatSetRef)) ||
		(pNext->keyAddress.typeOfAddress!=pAddress->typeOfAddress) ||
		(pNext->keyAddress.lenOfAddress!=pAddress->lenOfAddress) ||
        (memcmp(pNext->keyAddress.pAddress,pAddress->pAddress, pNext->keyAddress.lenOfAddress)!=0))	   
        	{
			  //will need to check if something is manipulating the list (mutex or semaphore check here for the StoreHead)
              pNext = (IEC905_MSG_CNTRL *)pNext->pNext;
			}
    else
      break;
    }


  return(pNext);
}

/************************************************************************************/
/*					find_key_in_chain			    */
/* searches for key information based upon CHAIN		  		    */
/* internal helper function only						    */
/************************************************************************************/
static IEC905_KEY_INFO *find_key_in_chain(IEC_CHAIN_INFO *keyChain, 
	ST_UCHAR *pKeyID, 
	IEC905_MSG_CNTRL *pKeyStorage)	//used to check for Src and destination IP Address
{
IEC905_KEY_INFO *pNext=NULL;

    if((keyChain==NULL) || (pKeyID==NULL))
      return(NULL);

    if((pNext = keyChain->pStartKeyInfo)==NULL)
      return(NULL);

    while(pNext!=NULL) 
	{
	  if(!memcmp (pNext->key_id,pKeyID,SIZE_OF_KEY_ID))
	    {
	    //have an ID that matches, now need to check the IP addresses that it is associated with
	      if( (pNext->pUsedBy->keyAddress.typeOfAddress==pKeyStorage->keyAddress.typeOfAddress) &&
		  (!memcmp(pNext->pUsedBy->keyAddress.pAddress,pKeyStorage->keyAddress.pAddress,pNext->pUsedBy->keyAddress.lenOfAddress)))
		  {
		    //now need to check the Src address if there is one
		    if(pNext->pUsedBy->srcKeyAddress.lenOfAddress==pKeyStorage->keyAddress.lenOfAddress)
		      {
			if( (pNext->pUsedBy->srcKeyAddress.lenOfAddress!=0) &&
			    ( !memcmp(pNext->pUsedBy->srcKeyAddress.pAddress,pKeyStorage->keyAddress.pAddress,pNext->pUsedBy->srcKeyAddress.lenOfAddress) ))
			    break;	//Have a match and need to return it
		      }
		    else
		      {
		      //will need to check if something is manipulating the list (mutex or semaphore check here for the StoreHead)
		      pNext= pNext->pNext;
		      }
		      
		  }
	      else
		{
	         //will need to check if something is manipulating the list (mutex or semaphore check here for the StoreHead)
		pNext= pNext->pNext;
		}
	    }
	  else
	    {
		 //will need to check if something is manipulating the list (mutex or semaphore check here for the StoreHead)
		pNext= pNext->pNext;
	    }
	}

    return(pNext);
}



/************************************************************************************/
/*				iec905_destroy_key_info				    */
/* free of allocated key information				  		    */
/************************************************************************************/

static ST_BOOLEAN iec905_destroy_key_info(IEC905_KEY_INFO *pKeyInfo)
{
IEC_CHAIN_INFO *pContext;
IEC905_KEY_INFO *pFound, *pPayloadKeyInfo;
IEC905_MSG_CNTRL *pPayloadID;

  if(!pKeyInfo)
    return(FALSE);

  pContext = pKeyInfo->pChain;


  if((pFound = pContext->pStartKeyInfo)==NULL)
      return(FALSE);
 
   
  //make sure this thing is in the list before we do anything
   while ((pFound!=NULL) && (pFound!=pKeyInfo))
    pFound = pFound->pNext;

//  if((pFound = find_key_in_chain(pContext, pKeyInfo->key_id, pKeyInfo->pUsedBy))==NULL)	//it is not in the chain, not ours
//    return(FALSE);

  if(pFound==NULL)
    return(FALSE);

  if(pKeyInfo->pPrev)
    {
      pKeyInfo->pPrev->pNext = pKeyInfo->pNext;
      if(pKeyInfo->pNext)
        pKeyInfo->pNext->pPrev = pKeyInfo->pPrev;
#if 0
      if(pKeyInfo->pNext)
	  {
	  pKeyInfo->pNext->pPrev = pKeyInfo->pPrev;
	  pKeyInfo->pPrev->pNext = pKeyInfo->pNext;
	  }
      else
	  pKeyInfo->pPrev->pNext = NULL;
#endif

    }
  else
    {
    // this means that the payload is the first in the list and have to find the root node 
      pKeyInfo->pChain->pStartKeyInfo=pKeyInfo->pNext;
      if( pKeyInfo->pNext)
	 pKeyInfo->pNext->pPrev = NULL;
     }

  pPayloadID = pKeyInfo->pUsedBy;
  pKeyInfo->pUsedBy = NULL;
  pKeyInfo->pPrev=pKeyInfo->pNext=NULL;

  //now determine if it is the current or next key
  if((pPayloadKeyInfo = pPayloadID->pCurrentKey)==pFound)
     pPayloadID->pCurrentKey = NULL;
  else if((pPayloadKeyInfo = pPayloadID->pNextKey)==pFound)
     pPayloadID->pNextKey = NULL;

  
  if(pKeyInfo->pKey)
    free(pKeyInfo->pKey);

  //now need to unlink from chain and then free it
/*unlink from the appropriate chain			*/

 //will need to lock if  (mutex or semaphore check here for the StoreHead)



 //will need to unlock if  (mutex or semaphore check here for the StoreHead)


  free(pKeyInfo);
  return(TRUE);
}

/************************************************************************************/
/*				add_key						    */
/* adds a key to the internal key chain	and Message Control	  		    */
/************************************************************************************/
static IEC905_KEY_INFO *add_key(IEC905_MSG_CNTRL *pKeyStorage, IEC905_KEY_INFO **pKeyInfo, ST_UINT8 typeOfKey,  ST_UINT16 key_len, ST_UCHAR *pKey,  ST_UCHAR *pKeyID, ST_UINT32 time_remaining )
{
IEC_CHAIN_INFO *pContext = find_chain_head(pKeyStorage->keyUsageType);
IEC905_KEY_INFO *pFound;

  if(pContext==NULL)
    return (NULL);

  if((pFound = find_key_in_chain(pContext,pKeyID, pKeyStorage))!=NULL)    //then the KeyID is already present, can't have 2 of the same KeyID.
	  return(NULL);


   if((pFound = calloc(1, sizeof(IEC905_KEY_INFO)))==NULL)
      {
	IEC905_ErrorHandler (MEMORY_ALLOCATION_ERROR, __FILE__, __LINE__);
	return(NULL);
      }
   pFound->pUsedBy = pKeyStorage;

  
  //set the time remaining and expiration time, both are in seconds
  if(time_remaining == 0)			//then will default to the maximum
	pFound->elapsedTimeUntilExpiration = MAX_ALLOWED_KEY_EXPIRATION_SECONDS;
  else
    pFound->elapsedTimeUntilExpiration = time_remaining;

   pFound->timeOfexpiration = time(NULL) + pFound->elapsedTimeUntilExpiration;  //value can just now be compared 

   pFound->timeOfInitialUse = time(NULL);
   pFound->pKey = (ST_UCHAR *)calloc(1,key_len);
   memcpy( pFound->pKey, pKey, key_len);
  
   pFound->keyLen = key_len;

   memcpy( pFound->key_id,pKeyID,SIZE_OF_KEY_ID);
   pFound-> typeOfKey = typeOfKey;

  //now link it to the list

 //will need to lock if  (mutex or semaphore check here for the StoreHead)
   pFound->pNext = pContext->pStartKeyInfo;
  if(pContext->pStartKeyInfo!=NULL)
    pContext->pStartKeyInfo->pPrev = pFound;
  pContext->pStartKeyInfo = pFound;
  *pKeyInfo = pFound;
//will need to unlock if  (mutex or semaphore check here for the StoreHead)

  //Now need to set the used by pointer



  pFound->pChain = pContext;
  return(pFound);
}

/************************************************************************************/
/*				iec905_add_next_key				    */
/* adds a next to usekey to the internal key chain and Message Control		    */
/* although global, this function should not be used directly by the user	    */
/************************************************************************************/
IEC905_KEY_INFO * iec905_add_next_key(IEC905_MSG_CNTRL *pKeyStorage, ST_UINT8 typeOfKey, ST_UINT16 key_len, ST_UCHAR *pKey,  ST_UCHAR *pKeyID, ST_UINT32 time_remaining)
{
  if(pKeyStorage->pNextKey)	  //if there is already a current key, destroy it.
    iec905_destroy_key_info(pKeyStorage->pNextKey);
  return(add_key(pKeyStorage, &pKeyStorage->pNextKey, typeOfKey, key_len, pKey, pKeyID, time_remaining));
}
/************************************************************************************/
/*				iec905_next_next_key				    */
/* adds a next to use key to the internal key chain and Message Control		    */
/* although global, this function should not be used directly by the user	    */
/************************************************************************************/
IEC905_KEY_INFO * iec905_add_current_key( IEC905_MSG_CNTRL *pKeyStorage, ST_UINT8 typeOfKey, ST_UINT16 key_len, ST_UCHAR *pKey, ST_UCHAR *pKeyID, ST_UINT32 time_remaining)
{
  if(pKeyStorage->pNextKey)    //if there is already a next key, destroy it.
    iec905_destroy_key_info(pKeyStorage->pNextKey);
  return(add_key(pKeyStorage,&pKeyStorage->pCurrentKey, typeOfKey, key_len, pKey, pKeyID, time_remaining));
}




/******************************************************************************************************/
/******************************************************************************************************/
/*	      The following section are gloabal PayloadID functions				      */
/******************************************************************************************************/
/******************************************************************************************************/
//this function needs to be called prior to each transmission (e.g. to get a key rotation
//a return value of TRUE indicates that the application needs to 
//return values for manage_tx_key
//#define KEYS_NOK 0			//indicates that neither the primary nor next key are valid
//#define KEYS_OK   1			//indicates that the Primary key is ok
//#define PRIME_KEY_OK_NEXT_KEY_NOK 1		//indicates that the primary key is OK, but there is no next key
int iec905_manage_keys(IEC905_MSG_CNTRL *pKeyRoot)
{
 time_t seconds;

 if(pKeyRoot==NULL)
    return(FALSE);
 seconds = time(NULL);			/*get the number of seconds */
 if ((pKeyRoot->pCurrentKey) && (seconds >= pKeyRoot->pCurrentKey->timeOfexpiration))			/*then the primary has expire  */
   {
	iec905_destroy_key_info(pKeyRoot->pCurrentKey);
	pKeyRoot->pCurrentKey = pKeyRoot->pNextKey;
	pKeyRoot->pNextKey = NULL;


//place to spawn a thread to go back to GDOI to refresh the next key
	if(pKeyRoot->requestedUpdate==FALSE)
	 {
	 pKeyRoot->requestedUpdate=TRUE;
	 usr_notify_of_key_updated_needed(pKeyRoot);
	 }

	if(pKeyRoot->pCurrentKey)
	  {
	  if(pKeyRoot->pCurrentKey->pKey!=NULL)
	    return(PRIME_KEY_OK_NEXT_KEY_NOK);
	  else
	    return(KEYS_NOK);
	  }
	 else
	  return(KEYS_NOK);
   }

  if(pKeyRoot->pNextKey)
    {
    if(pKeyRoot->pNextKey->pKey==NULL)
      return(PRIME_KEY_OK_NEXT_KEY_NOK);
    }

 return(SUCCESS_IEC905);

}


/************************************************************************************/
/*					iec905_destroy_msg_cntrl		    */
/*  function frees storage for  key payload and associated information	 	    */
/*  including key information that were allocated				    */
/*										    */
/*  Return:  BOOLEAN (TRUE)							    */
/* 										    */
/************************************************************************************/
ST_BOOLEAN iec905_destroy_msg_cntrl (IEC905_MSG_CNTRL *pKeyRoot)
{
   IEC905_MSG_CNTRL **pHeadofStore;
   IEC905_MSG_CNTRL *pRet;
   IEC_CHAIN_INFO *pTempChain=find_chain_head(pKeyRoot->keyUsageType);	//get the chain
 
   //make sure that the Payload is in the list
   if(pTempChain==NULL)
     return(FALSE);

   if((pRet = find_key_storage(pTempChain, &pKeyRoot->keyAddress, pKeyRoot->pDataSetRef))!=pKeyRoot)  //something is wrong don't touch as we didn't manage it
     return(FALSE);

 /*unlink from the appropriate chain			*/

 //will need to lock if  (mutex or semaphore check here for the StoreHead)

  if(pKeyRoot->pPrev)
    {
	  pKeyRoot->pPrev->pNext = pKeyRoot->pNext;
	  if(pKeyRoot->pNext)
	    pKeyRoot->pNext->pPrev = pKeyRoot->pPrev;

#if 0
      if(pKeyRoot->pNext)
	  {
	  pKeyRoot->pNext->pPrev = pKeyRoot->pPrev;
	  pKeyRoot->pPrev->pPrev = pKeyRoot->pNext;
	  }
      else
	  pKeyRoot->pPrev->pNext = NULL;
#endif
    }
  else
    {
    // this means that the payload is the first in the list and have to find the root node 
      if((pHeadofStore= find_key_storage_head(pTempChain, pKeyRoot->keyAddress.typeOfAddress))!=NULL)
	 {
	      *pHeadofStore = pKeyRoot->pNext;
	       if(pKeyRoot->pNext)
		 pKeyRoot->pNext->pPrev = NULL;
	 }

     }

   iec905_destroy_key_info(pKeyRoot->pCurrentKey);
   iec905_destroy_key_info(pKeyRoot->pNextKey);
   if((pKeyRoot->pNextKey!=NULL) || (pKeyRoot->pCurrentKey!=NULL))
      printf("MAJOR ERROR destroy\n");

 /* now need to free up what was previously allocated	*/
  if(pKeyRoot->keyAddress.pAddress)
    free(pKeyRoot->keyAddress.pAddress);

  if(pKeyRoot->srcKeyAddress.pAddress)
    free(pKeyRoot->srcKeyAddress.pAddress);

  if(pKeyRoot->pDataSetRef)
    free(pKeyRoot->pDataSetRef);



 //will need to unlock if  (mutex or semaphore check here for the StoreHead)

 /* free up the actual storage				*/
     free(pKeyRoot);

  return(TRUE);

}

/************************************************************************************/
/*					create_msg_cntrl  			    */
/*  function allocates storage for new key storage and places it into the 	    */
/*  the appropriate key chain for efficient lookup				    */
/*  all inputs to the function, if allocated by the application can be freed	    */
/*  upon return.								    */
/*  The function does not interact with the GDOI KDC, that is under control	    */
/*  of the application initially.						    */
/*										    */
/*  Return:  Pointer to the allocated storage, if NULL there was a problem	    */
/* 										    */
/************************************************************************************/

static IEC905_MSG_CNTRL *create_msg_cntrl( ST_UINT8 usageType, IEC_COMM_ADDRESS *pAddress, IEC_COMM_ADDRESS *pSrcAddress, char *pDatSetRef)	//used to allocate key storage and put it into the appropriate chain
{
   IEC905_MSG_CNTRL **pHeadofStore;
   IEC905_MSG_CNTRL *pNextStore;
   IEC905_MSG_CNTRL *pTempStore;
   IEC_CHAIN_INFO *pTempChain;	
						//pointer to allocated information	
   	  
  if(( usageType> MAX_IEC_KEY_USAGE_TYPE)||(pAddress->typeOfAddress >IP_V6_ADDRESS_TYPE)
	|| (pAddress->lenOfAddress ==0) || (pAddress->pAddress==NULL))		//we can't do anything since there is no chain for it
    return(NULL);

   if((pTempChain =find_chain_head(usageType))==NULL)	    //get the chain that will need to be used
    return(NULL);

   if((pHeadofStore= find_key_storage_head(pTempChain, pAddress->typeOfAddress))==NULL)
    return(NULL);

  if((pTempStore = find_key_storage(pTempChain, pAddress, pDatSetRef))==NULL)		//see if it is already in the chain
    {
    if((pTempStore = (IEC905_MSG_CNTRL *) calloc(1, sizeof(IEC905_MSG_CNTRL)))!=NULL)
      pTempStore->keyUsageType = usageType;
    else
      {
	IEC905_ErrorHandler (MEMORY_ALLOCATION_ERROR, __FILE__, __LINE__);
	return(NULL);
      }


    if(pTempStore!=NULL)
    {

    /* we have stuff to fill in	      */
      pTempStore->keyAddress.typeOfAddress = pAddress->typeOfAddress;
      pTempStore->keyAddress.lenOfAddress = pAddress->lenOfAddress;  
      if((pTempStore->keyAddress.pAddress = calloc(1,pTempStore->keyAddress.lenOfAddress))==NULL)
      {
	IEC905_ErrorHandler (MEMORY_ALLOCATION_ERROR, __FILE__, __LINE__);
	free(pTempStore);
	return(NULL);
      }
       memcpy(pTempStore->keyAddress.pAddress,pAddress->pAddress, pTempStore->keyAddress.lenOfAddress);

      pTempStore->dataSetRefLen = (ST_UINT8)strlen(pDatSetRef);
      if((pTempStore->pDataSetRef = calloc(1,(pTempStore->dataSetRefLen+1)))==NULL)
      {
	IEC905_ErrorHandler (MEMORY_ALLOCATION_ERROR, __FILE__, __LINE__);
	free(pTempStore->keyAddress.pAddress);
	pTempStore->keyAddress.pAddress=NULL;
	free(pTempStore);
	return(NULL);
      }
      strcpy(pTempStore->pDataSetRef,pDatSetRef);

      if(pSrcAddress==NULL)
	{
        pTempStore->srcKeyAddress.lenOfAddress=0;
        pTempStore->srcKeyAddress.typeOfAddress=NO_ADDRESS_TYPE;
        pTempStore->srcKeyAddress.pAddress=NULL;
	}
      else
	{
	pTempStore->srcKeyAddress.lenOfAddress= pSrcAddress->lenOfAddress;
	pTempStore->srcKeyAddress.typeOfAddress = pSrcAddress->typeOfAddress;
	pTempStore->srcKeyAddress.pAddress = calloc(1,pSrcAddress->lenOfAddress);
	memcpy(pTempStore->srcKeyAddress.pAddress,pSrcAddress->pAddress,pSrcAddress->lenOfAddress);
	}

   }
   else
    return(NULL);

   //now that we have it filled in, time to link the storage into the appropriate chain
 //will need to lock if  (mutex or semaphore check here for the StoreHead)
    
     pNextStore= *pHeadofStore;
     if(pNextStore!=NULL)
       pNextStore->pPrev = (struct iec905_msg_cntrl *) pTempStore;

     pTempStore->pNext= (struct iec905_msg_cntrl *) pNextStore;
     pTempStore->pPrev = NULL;
     *pHeadofStore = pTempStore;
 //will need to unlock if  (mutex or semaphore check here for the StoreHead)

  }
   
  return(pTempStore);

}
/************************************************************************************/
/*				iec905_create_msg_cntrl_rx  			    */
/*  creates a MSG_CNTRL structure for use for receiving information		    */
/************************************************************************************/
IEC905_MSG_CNTRL *iec905_create_msg_cntrl_rx( ST_UINT8 usageType, IEC_COMM_ADDRESS *pAddress, IEC_COMM_ADDRESS *pSrcAddress, char *pDatSetRef)
{
//allow subscriptions to not specify the SrcAddress, however this is not advised $$$need to discuss
    return(create_msg_cntrl( usageType, pAddress, pSrcAddress, pDatSetRef));
}

/************************************************************************************/
/*				iec905_create_msg_cntrl_tx  			    */
/*  creates a MSG_CNTRL structure for use fortransmitting information		    */
/************************************************************************************/
IEC905_MSG_CNTRL *iec905_create_msg_cntrl_tx( ST_UINT8 usageType, IEC_COMM_ADDRESS *pAddress, IEC_COMM_ADDRESS *pSrcAddress, char *pDatSetRef)
{
    return(create_msg_cntrl( usageType, pAddress, pSrcAddress, pDatSetRef));
}


/**************************************************************************************/
/*			iec905_init_key_storage					      */
/*										      */
/*  Function initializes the internal storage required for keys and payload	      */
/*  definitions.								      */
/*										      */
/*  No inputs or outputs							      */
/*							  			      */
/**************************************************************************************/
void iec905_init_key_storage()
{
  if(intialized==FALSE)	      //protection from multiple initialization calls
    {
    init_key_chain(&keyChains.GooseChain);
    init_key_chain(&keyChains.SVChain);
    init_key_chain(&keyChains.TunnelChain);
    intialized = TRUE;
    }
 
}


/**************************************************************************************/
/*			iec905_find_rxd_msg_cntrl				      */
/*										      */
/*  Function looks for a registered security message control structure		      */
/*  based upon the Session PDU type and the received destination address	      */
/*										      */
/*  Returns: Pointer to the found structure or NULL if not found		      */
/*  Although global, not to be used by user directly				      */
/**************************************************************************************/
IEC905_MSG_CNTRL *iec905_find_rxd_msg_cntrl(ST_UCHAR pduSI,  IEC_COMM_ADDRESS *pDestAddress, ST_UCHAR *pKeyID)
{
IEC905_MSG_CNTRL **pTempStoreHead=NULL;
IEC905_MSG_CNTRL *pNext;
IEC_CHAIN_INFO *pSelectedChain;

   switch(pduSI)
    {
    	case IEC_90_5_SI_TYPE_GOOSE:
	  pSelectedChain = &keyChains.GooseChain;
	  break;

	case IEC_90_5_SI_TYPE_SV:
	  pSelectedChain = &keyChains.SVChain;
	  break;

	case IEC_90_5_SI_TYPE_TUNNEL:
	 pSelectedChain = &keyChains.TunnelChain;
	  break;

	default:
	  pSelectedChain =NULL;
	  break;
    }
  
  if(pSelectedChain ==NULL)
    return(NULL);

  if(pDestAddress->typeOfAddress==IP_V4_ADDRESS_TYPE)
      pTempStoreHead = &pSelectedChain->pIpV4Storage;

  if(pDestAddress->typeOfAddress==IP_V6_ADDRESS_TYPE)
      pTempStoreHead = &pSelectedChain->pIpV4Storage; 

  if((pTempStoreHead == NULL)|| (*pTempStoreHead == NULL))
    return(NULL);



  //now need to search the chain for the appropriate Storage Information
  //need to match on Source address, in Linux, may be able to couple with destination address (not available in Windows
  pNext = *pTempStoreHead;
  while(pNext!=NULL)
    {
    if((pNext->keyAddress.pAddress!=NULL) && (pNext->pCurrentKey!=NULL))
      {
      if( (pNext->keyAddress.typeOfAddress==pDestAddress->typeOfAddress) ||
	  (pNext->keyAddress.lenOfAddress==pDestAddress->lenOfAddress) ||
          (memcmp(pNext->srcKeyAddress.pAddress,pDestAddress->pAddress, pNext->keyAddress.lenOfAddress)==0))
	  {
	    //now need to check if the current key matches
	    if((memcmp(pNext->pCurrentKey->key_id,pKeyID,4)==0))
		break;
	    else if((memcmp(pNext->pNextKey->key_id,pKeyID,4)==0))	//if the current key doesn't match, check the next key (could have rotated
		{							//in the sender, if so, need to make the next key the current key
		  iec905_destroy_key_info(pNext->pCurrentKey);
		  pNext->pCurrentKey = pNext->pNextKey;
		  pNext->pNextKey = NULL;
		  if(pNext->requestedUpdate==FALSE)
		    {
		    pNext->requestedUpdate=TRUE;
	            usr_notify_of_key_updated_needed(pNext);
		    }
		  break;
		}
	    else
              pNext = (IEC905_MSG_CNTRL *)pNext->pNext;		      //didn't match the current or next key, so need to check the next control
	  }
	else
	  pNext = (IEC905_MSG_CNTRL *)pNext->pNext;	
      }
     else
      pNext = (IEC905_MSG_CNTRL *)pNext->pNext;
    }

  return(pNext);
}

/**************************************************************************************/
/*			calc_stats						      */
/* function looks at the statistics in a specific control structure and adds them     */
/* to the total.  If resetStats = TRUE, it will writen the control stats to zero      */
/**************************************************************************************/
static void calc_stats(IEC905_MSG_CNTRL *pInfo, IEC905_STATISTICS *pRetStatistics, int resetStats)
{
    while (pInfo!=NULL)
    {
       pRetStatistics->TotalRxPktCnt += pInfo->stats.TotalRxPktCnt;
       pRetStatistics->TotalRxMissingPktCnt +=pInfo->stats.TotalRxMissingPktCnt;
       pRetStatistics->TotalRxPktWithBadHMAC +=pInfo->stats.TotalRxPktWithBadHMAC;
       pRetStatistics->TotalTxPktCnt+=pInfo->stats.TotalTxPktCnt;
      if(resetStats==TRUE)
	memset((char *)&pInfo->stats,0, sizeof(IEC905_STATISTICS));

      pInfo = pInfo->pNext;
    }
}


/**************************************************************************************/
/*			iec905_getStats						      */
/* function looks at the statistics in a reostered control structure and adds them    */
/* to the total.  If resetStats = TRUE, it will writen the control stats to zero      */
/**************************************************************************************/
void iec905_getStats(IEC905_STATISTICS *pRetStatistics, int resetStats)
{
IEC905_MSG_CNTRL *pInfo;

    pRetStatistics->TotalTxPktCnt=0;
    pRetStatistics->TotalRxPktCnt=0;
    pRetStatistics->TotalRxMissingPktCnt=0;
    pRetStatistics->TotalRxPktWithBadHMAC=0;

    pInfo = keyChains.SVChain.pIpV4Storage;
    calc_stats(pInfo, pRetStatistics ,resetStats);

    pInfo = keyChains.SVChain.pIpV6Storage;
    calc_stats(pInfo, pRetStatistics ,resetStats); 
  
    pInfo = keyChains.GooseChain.pIpV4Storage;
    calc_stats(pInfo, pRetStatistics ,resetStats);

    pInfo = keyChains.GooseChain.pIpV6Storage;
    calc_stats(pInfo, pRetStatistics ,resetStats);

    pInfo = keyChains.TunnelChain.pIpV4Storage;
    calc_stats(pInfo, pRetStatistics ,resetStats);

    pInfo = keyChains.TunnelChain.pIpV6Storage;
    calc_stats(pInfo, pRetStatistics ,resetStats);
    
}