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
/* MODULE NAME : iec_90_5_dec.c						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Decoder for IEC 61850-90-5 Session/CLTP Protocol		    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 08/15/12  HSF	   Corrected SPDU header length value (constant)    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"




/************************************************************************/
/* This function encodes the either a GOOSE or SV Payload		*/
/************************************************************************/
static ST_UCHAR *decode_GOOSE_SV_MNGT_Payload(IEC905_SESS_PAYLOAD_DEF *pPayload, ST_UCHAR *pDec)
{

	pPayload->simulationBit = *pDec;
	++pDec;

	pPayload->appID = ntohs (*(ST_UINT16 *)(pDec));
	pDec+=sizeof(ST_UINT16);
	pPayload->pduLen = ntohs (*(ST_UINT16*)(pDec));
	pDec += sizeof(ST_UINT16);
	pPayload->pPDU = pDec;
	pDec+=pPayload->pduLen;
	return(pDec);
}


/************************************************************************/
/* This function encodes the a Tunnel Payload				*/
/************************************************************************/
static ST_UCHAR *decode_TUNNEL_Payload(IEC905_SESS_PAYLOAD_DEF *pPayload, ST_UCHAR *pDec)
{

	pPayload->simulationBit = *pDec;
	++pDec;

	pPayload->appID = ntohs (*(ST_UINT16 *)(pDec));
	pDec+=sizeof(ST_UINT16);
	pPayload->dst_mac=pDec;
	pDec+=6;
	pPayload->tpid = ntohs (*(ST_UINT16 *)(pDec));
	pDec +=sizeof(ST_UINT16);
	pPayload->tci = ntohs (*(ST_UINT16 *)(pDec));
	pDec +=sizeof(ST_UINT16);
	pPayload->pduLen = ntohs (*(ST_UINT16*)(pDec));
	pDec += sizeof(ST_UINT16);
	pPayload->etype_id=ntohs(*(ST_UINT16 *)(pDec));
	pDec += sizeof(ST_UINT16);
	pPayload->pPDU = pDec;
	pDec+=pPayload->pduLen;

	return(pDec);
}

/************************************************************************/
/* This function encodes the chain of payloads				*/
/************************************************************************/
static ST_UINT8 decode_payloads(
	    IEC_90_5_RX *rxd,					
		ST_UCHAR *pPayload,
		ST_UINT32 payloadLen)
{
IEC905_SESS_PAYLOAD_DEF **pNextPayload = &rxd->pPayload;		/*pointer of where to place the payload structure pointer	*/
ST_UCHAR *pCur=pPayload;
ST_UCHAR *pEndOfPayload = pCur+ payloadLen;
IEC905_SESS_PAYLOAD_DEF *pIntPayload;
ST_BOOLEAN problemDetected=FALSE;


  if(pNextPayload==NULL)
      return(SUCCESS_IEC905);

  while((pCur<pEndOfPayload) && (problemDetected==FALSE))
  {
    if((pIntPayload = (IEC905_SESS_PAYLOAD_DEF *)calloc(1, sizeof(IEC905_SESS_PAYLOAD_DEF)))==NULL)
	{
	IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
	problemDetected=TRUE;
	}
    else
    {
    if(pNextPayload!=NULL)
      *pNextPayload = pIntPayload;
    else
	problemDetected=TRUE;
      
      switch(*pCur)
      {
      case GOOSE_PAYLOAD_TYPE_TAG:
	  pIntPayload->payload_tag =GOOSE_PAYLOAD_TYPE_TAG;
	  pCur = decode_GOOSE_SV_MNGT_Payload(pIntPayload,++pCur);
	  break;

      case SV_PAYLOAD_TYPE_TAG:   
	  pIntPayload->payload_tag =SV_PAYLOAD_TYPE_TAG;
	  pCur = decode_GOOSE_SV_MNGT_Payload(pIntPayload,++pCur);
	  break;

    case TUNNEL_PAYLOAD_TYPE_TAG:
	  pIntPayload->payload_tag =TUNNEL_PAYLOAD_TYPE_TAG;
	  pCur = decode_TUNNEL_Payload(pIntPayload,++pCur);
	  break;
	  
    case MNGT_PAYLOAD_TYPE_TAG:
	  pIntPayload->payload_tag =MNGT_PAYLOAD_TYPE_TAG;
	  pCur = decode_GOOSE_SV_MNGT_Payload(pIntPayload,++pCur);
	  break;

    default:
	  problemDetected = TRUE;				/* don't understand the payload	*/
	  break;
     }
  }

  if(problemDetected==FALSE)
    {
		if(pNextPayload!=NULL)
		{
		*pNextPayload =pIntPayload;
		pNextPayload = &(pIntPayload->next);
	       }
	      else
		{
	        return(IEC905_ErrorHandler(DECODE_ERROR_IN_PAYLOADS,__FILE__,__LINE__));
	        break;
	      }

    }
  else
    {
	  return(IEC905_ErrorHandler(DECODE_ERROR_IN_PAYLOADS,__FILE__,__LINE__));
	  break;
	}

  }
  return(SUCCESS_IEC905);
}


/************************************************************************/
/* the following function is a user supplied function that decrypts the */
/* the user data and return a pointer and length to the encrypted data  */
/* so the rest of 90-5 packet can be put togther						*/
/************************************************************************/
static ST_UCHAR *decrypt_user_data(
    IEC905_SESS_PDU_HDR_INFO *hdr,					/* pointer on where to get security information */
	ST_UINT32 *payload_len,					/* length calculated for unencrypted payload */
	IEC905_SESS_PAYLOAD_DEF *payload_chain_ptr	/* ptr to the beggining of the payload chain  */
	)		
{


	return(NULL);
}

/************************************************************************/
/* Function:iec905_sess_dec											*/
/* Fills in and allocates HDR and payload information in the			*/
/* IEC_90_5_RX structure.												*/
/* in order to free the allocations, the user will need to call			*/
/* free_dec_info().  This will free the HDR and Payloads, but not		*/
/* the rxd structure or the buffer										*/
/************************************************************************/
int iec905_sess_dec(IEC_90_5_RX *rxd ,			    /*points to the rxd information	*/
			  IEC_COMM_ADDRESS *pDestAddress)	    /* points to the destination address and this is used to find the security information */	
  {
/* the overhead for the Session Hdr is		*/
/* 1 byte - SI								*/
/* 1 byte - LI								*/
/* 4 bytes - SPDU Length					Common HDR*/
/* 4 bytes  - SPDU number					Common HDR*/
/* 2 bytes  - version						Common HDR*/
/* 4 bytes - time of current key			Common HDR*/
/* 2 bytes - time of next key				Common HDR*/
/* 1 byte	- Encryption Alg				Common HDR*/
/* 1 byte - Signature Alg					Common HDR*/
/* 4 byte - KeyID							Common HDR */
#define IEC_90_5_SESSION_HDR_SIZE 24
#define IEC_90_5_SI_COMMHDR_SIZE 22			//changed from 18->22 8/11

ST_UCHAR *pDecBuf = rxd->pRXDbuffer;			/*points to buffer to be decoded	*/
ST_UINT32 decBufLen	= rxd->lenRXDBuffer;			/* length of the buffer				*/
IEC905_SESS_PDU_HDR_INFO *pHdr;
ST_UCHAR *pCur;
ST_UINT hdr_len= IEC_90_5_SESSION_HDR_SIZE ;
ST_UINT common_hdr_len = IEC_90_5_SI_COMMHDR_SIZE ;
ST_UCHAR *encrypted_user_data_ptr=NULL;
ST_UCHAR *pSpduStart;					/*needed for the signature			*/
ST_UINT32 spdu_num=0;
ST_UINT32 spdu_len=0;
ST_UINT32 payload_len=0;
ST_UINT32 totalCalcLen=0;
ST_UINT32 cltpLen;
ST_UINT8 retCode;
IEC905_MSG_CNTRL *pRxdCntl=NULL;
int retVal;
ST_UCHAR *pHMAC;
int errorDetected;

/* make sure that everything is NULL in the rxd buffer				*/



rxd->pHDR = NULL;
rxd->pPayload=NULL;


/*need to look at the main contents of the packet to see if the packet */
/*was tampered with														*/

 pHdr= NULL;
 pCur = pDecBuf;
 if(pCur==NULL)
  return(IEC905_ErrorHandler(IEC_90_5_NO_CLTP,__FILE__,__LINE__));


 if (pCur[1] == 0x40)		/*then we have a CLTP tag and can check the LI (byte 0) */
 {
	cltpLen = (ST_UINT32)pCur[0]+1;
 }
 else
 {
	 return(IEC905_ErrorHandler(IEC_90_5_NO_CLTP,__FILE__,__LINE__));
 }
 pCur+=cltpLen;
 pSpduStart=pCur;

#define KEY_ID_OFFSET 22
//before starting any decode, make sure that there is a security control structure for the usage and destination address
//if can't find the object, should not be receiving the PDU, don't process any further
  if((pRxdCntl =iec905_find_rxd_msg_cntrl(*pSpduStart,  pDestAddress, pCur+KEY_ID_OFFSET))==NULL)
      return(IEC905_ErrorHandler(IEC_90_5_TAMPERDECTECT_HDR,__FILE__,__LINE__));
 
  ++pRxdCntl->stats.TotalRxPktCnt;
    

 totalCalcLen = cltpLen + IEC_90_5_SESSION_HDR_SIZE;
 if(decBufLen<totalCalcLen)				/*if true, then something is wrong/abort	*/
	 return(IEC905_ErrorHandler(IEC_90_5UNEXPECTED_PDU_RXD,__FILE__,__LINE__));

 /*now check the type of the header		*/
 if( !((*pCur==IEC_90_5_SI_TYPE_TUNNEL) ||
	 (*pCur==IEC_90_5_SI_TYPE_GOOSE) ||
	 (*pCur==IEC_90_5_SI_TYPE_SV	) ||
	 (*pCur==IEC_90_5_SI_TYPE_MNGT)  ))
	 return(IEC_90_5_INVALID_HDR);

  /*now check the actual HDR.LI value	*/
  ++pCur;
  if(*pCur !=(IEC_90_5_SESSION_HDR_SIZE))
	 return(IEC905_ErrorHandler(IEC_90_5_INVALID_HDR_LI,__FILE__,__LINE__));

  /*skip the common header tag and length until we have another version to contend with*/

  pCur+=3;
  spdu_len = ntohl (*(ST_UINT32 *)pCur);	/* SPDU len	*/
  pCur += sizeof(spdu_len);


   spdu_num = ntohl (*(ST_UINT32 *)(pCur));

  totalCalcLen+=spdu_len;
  totalCalcLen = totalCalcLen  - IEC_90_5_SESSION_HDR_SIZE -3;


 /*now we know that we have enough to make it to the "end" */
 /*need to see what it should be with the signature			*/
 switch(pCur[13])			/*should be the SIG Alg			*/
 {
 case HMAC_ALG_SHA_256_80:
	 totalCalcLen+=11;		/*tag+10 bytes	*/
	 break;

 case HMAC_ALG_SHA_256_128:
 case HMAC_ALG_AES_GMAC_128:
	 totalCalcLen+=17;		/*tag+16 bytes	*/
	 break;

 case HMAC_ALG_SHA_256_256:
	 totalCalcLen+=33;		/*tag+32 bytes	*/
	 break;

 case HMAC_ALG_AES_GMAC_64:
	 totalCalcLen+=9;		/*tag+8 bytes	*/
	 break;

 default:
	 break;
 }


if(decBufLen>MAX_ALLOWED_UDP_PACKET_LENGTH)
  return(IEC905_ErrorHandler(MAX_ALLOWED_UDP_PACKET_LENGTH,__FILE__,__LINE__));

 /*can start and start filling in the hdr		*/

  pHdr = (IEC905_SESS_PDU_HDR_INFO *)calloc(1, sizeof(IEC905_SESS_PDU_HDR_INFO));
  if(pHdr==NULL)
     return(IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__));      

   rxd->pHDR = pHdr;

   pHdr->hmacAlg = pCur[13];
   pHdr->secAlgType = pCur[12];

   errorDetected=FALSE;
   pHdr->pKeyID = pCur+14;
   if((pHdr->hmacAlg!=HMAC_ALG_None ) || (pHdr->secAlgType!=SEC_ALG_NONE))	    //determine if we need keys at all
     {
     if(pRxdCntl->pCurrentKey!=NULL)
	{
        if(memcmp(pHdr->pKeyID, pRxdCntl->pCurrentKey->key_id,SIZE_OF_KEY_ID)!=0)	//then the ID is not the current key's ID
          {
	  if(pRxdCntl ->pNextKey!=NULL)
	    {
            pHdr-> keyLen = pRxdCntl ->pNextKey->keyLen;					
            pHdr->pKey = pRxdCntl ->pNextKey->pKey;
	    }
	  else
	    errorDetected=TRUE;
          }
       else
        {
        pHdr-> keyLen = pRxdCntl ->pCurrentKey->keyLen;					
        pHdr->pKey = pRxdCntl ->pCurrentKey->pKey;
	}
      }
      else
	errorDetected=TRUE;
    }

  if(errorDetected==TRUE)
    {
      iec905_manage_keys(pRxdCntl);
      free(pHdr);
      rxd->pHDR = NULL;
      return(IEC905_ErrorHandler(INVALID_INPUT_PARAMETER,__FILE__,__LINE__));	
    }
 /*check if the tag for the Signature is there */
 if(pCur[13]!=HMAC_ALG_None)
   {
	pHMAC = pSpduStart+spdu_len+2;		//need to include some tags.
	 if(*pHMAC!= 0x85)
	   {
	    free(pHdr);
	      rxd->pHDR = NULL;
	     rxd->pHDR = NULL;
	    ++pRxdCntl->stats.TotalRxPktWithBadHMAC;
	    return(IEC_90_5_TAMPERDECTECT_NO_SIG);
	    }
	++pHMAC;
	//check if the signature matches, if not, then there has been a tamper

	if((retVal = usr_compare_HMAC( pHdr,  pSpduStart, spdu_len, pHMAC))!=SUCCESS_IEC905)
	  {
	    free(pHdr);
	      rxd->pHDR = NULL;
	    iec905_manage_keys(pRxdCntl);
	    ++pRxdCntl->stats.TotalRxPktWithBadHMAC;
	    return(IEC_90_5_TAMPERDECTECT_NO_SIG);  
	  }

  }
 else
  {
    //check to see if expecting a HASH, if so, then there has been a tamper
    if(pRxdCntl->hashExpected == IEC_REQUIRE_HASH)
      {
	free(pHdr);
	rxd->pHDR = NULL;
	iec905_manage_keys(pRxdCntl);
	++pRxdCntl->stats.TotalRxPktWithBadHMAC;
	return(IEC905_ErrorHandler(IEC_90_5_TAMPERDECTECT_HDR,__FILE__,__LINE__));	
      }

  }

    if(pRxdCntl->last_spdu_num_rxd==0)
      pRxdCntl->last_spdu_num_rxd =spdu_num;
    else if(pRxdCntl->last_spdu_num_rxd+1<spdu_num)
      {
      if(pRxdCntl->last_spdu_num_rxd!=0)
	pRxdCntl->stats.TotalRxMissingPktCnt += spdu_num-pRxdCntl->last_spdu_num_rxd;
      pRxdCntl->last_spdu_num_rxd = spdu_num;
      }
   else if(spdu_num==pRxdCntl->last_spdu_num_rxd)	//we have a duplicate packet, need to ignore
    {
      	free(pHdr);
	  rxd->pHDR = NULL;
	iec905_manage_keys(pRxdCntl);
	return(IEC905_ErrorHandler(IEC_90_5_DUPLICATE_PACKET_RXD,__FILE__,__LINE__));	
    }
  else if (pRxdCntl->last_spdu_num_rxd<spdu_num)
    pRxdCntl->last_spdu_num_rxd = spdu_num;





  /*if the signature matches then we can continue     */
   pHdr->SessionIdentifer = *pSpduStart;


   //original memcpy(pHdr->timeOfCurrentKey,pCur+ 6,4);
   pHdr->timeOfCurrentKey = ntohl (*(ST_UINT32 *)(pCur));

   pHdr->timeToNextKey = ntohs (*(ST_UINT16 *)(pCur+ 10));

   /* place to call to get the actual key and keylenght */
   /*needs to be filled in later						*/

   /*assuming that the signature comes back good		*/
   /*now can start decoding payloads					*/
  pCur +=18;
  payload_len = ntohl (*(ST_UINT32 *)pCur);	/* payload len	*/
  pCur += sizeof(payload_len);

  retCode = decode_payloads(rxd,pCur,payload_len);
#if 0	    //don't destroy since it is up to the user
  if((retCode = decode_payloads(rxd,pCur,payload_len))!=SUCCESS_IEC905)
    {
      //need to destroy what was created
      iec905_destroy_dec_info(rxd);
      rxd=NULL;
    }
#endif
  iec905_manage_keys(pRxdCntl);				//check about the keys
  return (retCode);
  }


/************************************************************************/
/* Function: free_dec_info												*/
/* in order to free the allocations, the user will need to call			*/
/* free_dec_info().  This will free the HDR and Payloads, but not		*/
/* the rxd structure or the buffer										*/
/************************************************************************/
 int iec905_destroy_dec_info( IEC_90_5_RX *pRxd)
  {
	  IEC905_SESS_PDU_HDR_INFO *pHdr;
	  IEC905_SESS_PAYLOAD_DEF *pPayloads,*pTempPayload;

	  /*free the payload structures				*/
	  pPayloads = pRxd->pPayload;				/*first payload	*/
	  while(pPayloads !=NULL)
	  {
		  pTempPayload = pPayloads->next;
		  free ((BYTE *)pPayloads);
		  pPayloads = pTempPayload;
	  }

	  pRxd->pPayload = NULL;

	  /*now get rid of the allocated header structure			*/
	  pHdr = pRxd->pHDR;
	  if(pHdr!=NULL)
		  free ((BYTE *)pHdr);
	
	  pRxd->pHDR = NULL;
	  if(pRxd->pRXDbuffer)
	    free(pRxd->pRXDbuffer);

	  free(pRxd);

	  return(SUCCESS_IEC905);
  }


/************************************************************************/
/* Function: iec905_create_dec_info												*/
/* creates a receive structure for the user				*/
/************************************************************************/
IEC_90_5_RX * iec905_create_dec_info( )
{
  IEC_90_5_RX *pMyRxCntrl;
  if((pMyRxCntrl = calloc(1, sizeof(IEC_90_5_RX)))==NULL)
       IEC905_ErrorHandler (MEMORY_ALLOCATION_ERROR,__FILE__, __LINE__);
  return(pMyRxCntrl);
  
}
 