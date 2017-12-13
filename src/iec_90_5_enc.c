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
/* MODULE NAME :iec_90_5_enc.c						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Encoder for IEC 61850-90-5 Session/CLTP Protocol		    */
/*									    */
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				    */
/*	NONE								    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 09/25/12  HSF	   Updated SPDU_LEN calculation to remove the length */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 08/15/12  HSF	   Corrected SPDU header length value		    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"


/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/


static ST_UINT16 spdu_ver = 1;




/************************************************************************/
/* This function encodes the chain of payloads				*/
/************************************************************************/
static ST_UCHAR *encode_payloads(
		IEC905_SESS_PAYLOAD_DEF *payload_chain,
		ST_UCHAR *enc_ptr)
{
IEC905_SESS_PAYLOAD_DEF *internal_payload_chain = payload_chain;
ST_UCHAR *cur_ptr = enc_ptr;

  while(internal_payload_chain)
  {
	  /*encode the payload tag  */
	  *cur_ptr++ = internal_payload_chain->payload_tag;

	  /*encode the simulation bit  */
	  if(internal_payload_chain->simulationBit)
		  *cur_ptr++=0x01;
	  else
		  *cur_ptr++=0x00;

	  /*encode the appID */
      *(ST_UINT16 *)cur_ptr = htons(internal_payload_chain->appID);	/* APPID*/
		cur_ptr += 2;

	  /* now if it is tunneled payload there is more to do */
	  if(internal_payload_chain->payload_tag==TUNNEL_PAYLOAD_TYPE_TAG)
	  {
		  memcpy(cur_ptr,&internal_payload_chain->dst_mac,6);
		  cur_ptr+=6;

		  *(ST_UINT16 *)cur_ptr = htons(internal_payload_chain->tpid);	/* TPID*/
		   cur_ptr += 2;

		   *(ST_UINT16 *)cur_ptr = htons(internal_payload_chain->tci);	/* TCI*/
		   cur_ptr += 2;
	  }

	  /*encode the lentgth	(common for all Payloads			*/
	    *(ST_UINT16 *)cur_ptr = htons(internal_payload_chain->pduLen);	
 	   cur_ptr += 2;

	  if(internal_payload_chain->payload_tag==TUNNEL_PAYLOAD_TYPE_TAG)
	  {
	    /* Encode the Ethertype ID.	*/
		*(ST_UINT16 *)cur_ptr = htons(internal_payload_chain->etype_id);
		cur_ptr += 2;
	  }

  /* Copy the rest of the "tunnelled" PDU	*/
  memcpy (cur_ptr, internal_payload_chain->pPDU, (size_t)(internal_payload_chain->pduLen));
  cur_ptr += internal_payload_chain->pduLen;
  internal_payload_chain = internal_payload_chain->next;
  }
  return(cur_ptr);
}

/************************************************************************/
/* the following function is a user supplied function that encrypts the */
/* the user data and return a pointer and length to the encrypted data  */
/* so the rest of 90-5 packet can be put togther						*/
/************************************************************************/
static ST_UCHAR *encrypt_user_data(
    IEC905_SESS_PDU_HDR_INFO *hdr,					/* pointer on where to get security information */
	ST_UINT32 *payload_len,					/* length calculated for unencrypted payload */
	IEC905_SESS_PAYLOAD_DEF *payload_chain_ptr	/* ptr to the beggining of the payload chain  */
	)		
{
ST_UCHAR secAlgType = hdr->secAlgType;			/* type of encryption to be used	     */
ST_UCHAR *unencrypted_buffer;
ST_UCHAR *encrypted_buffer;
/*ST_UINT16 encrypted_data_len;				*//*to be used to contain the encrypted user data length */

	/* make sure that there is not a mistake					*/
	if((secAlgType==SEC_ALG_NONE) || (secAlgType>SEC_ALG_AES_256_GCM))
	{
	    IEC905_ErrorHandler (INVALID_INPUT_PARAMETER,__FILE__,__LINE__);
	    return(NULL);
	}

	/* allocate a buffer for the non-encrypted user data	*/
	unencrypted_buffer = malloc(*payload_len);

	/*now created the un-encrypted user data in the buffer */
	encode_payloads(payload_chain_ptr,unencrypted_buffer);

	/* allocate the buffer to use for encryption, typically this will be the original lenght */
	/* and up to 256 additional bytes														 */
	encrypted_buffer = usr_encrypt_payloads(hdr, unencrypted_buffer,payload_len);

	/* get rid of the unencrypted buffer that was allocated									*/
	free (unencrypted_buffer);

	return(encrypted_buffer);
}

/************************************************************************/
/* NOTE: tunnel_pdu_ptr points to GOOSE/SMPVAL PDU starting right AFTER	*/
/*   Ethertype ID. This would be easier if it pointed to the		*/
/*   Ethertype ID, but clnp_snet_read doesn't give us that.		*/
/************************************************************************/
ST_UCHAR *iec905_sess_enc (
	IEC905_SESS_PDU_HDR_INFO *hdr,			/*points to the header information*/
	IEC905_SESS_PAYLOAD_DEF *payload,		/*points to the first in the chain of payload information (freeing of the information is reponsibility of the caller*/
	ST_UINT32 *enc_len_ptr,
	ST_UINT32 *pSPDUnum				//pointer to the SPDU number to be used, will be incremented if encode is successful
	)
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
#define IEC_90_5_SI_COMMHDR_SIZE 22		  //changed from 18->22 on 8/11

ST_UINT32 spdu_num = *pSPDUnum;	/* increment each time PDU sent	*/
ST_UCHAR *enc_buf;
ST_UCHAR *cur_ptr;
ST_UINT hdr_len= IEC_90_5_SESSION_HDR_SIZE ;
ST_UINT common_hdr_len = IEC_90_5_SI_COMMHDR_SIZE ;
ST_UINT32 spdu_len=0;
ST_UINT32 payload_len=0;
ST_UINT alloc_len;	/* allocated len for encoded message	*/
ST_UINT enc_len;	/* total len of encoded message (should match alloc_len)	*/
ST_UCHAR *encrypted_user_data_ptr=NULL;
ST_UCHAR *spdu_start_ptr;					/*needed for the signature			*/
IEC905_SESS_PAYLOAD_DEF *temp_payload_ptr=payload;


/* determine how large of a packet it allocate  */

/* first need to run through the chain of payload definition  and calculate the payload length*/
  while(temp_payload_ptr!=NULL)
  {
	switch(temp_payload_ptr->payload_tag)
	{
#define NON_TUNNEL_PAYLOAD_HDR_SIZE	6					/* 1 byte tag + 1 byte Simulation + 2 bytes appID + 2 bytes length = 6 */
	case GOOSE_PAYLOAD_TYPE_TAG:
	case  SV_PAYLOAD_TYPE_TAG:
	case  MNGT_PAYLOAD_TYPE_TAG:
		payload_len += (temp_payload_ptr->pduLen)+6;	/* add the PDU length  */
		temp_payload_ptr= temp_payload_ptr->next;
		break;

/* overhead for Tunnel Payload			*/
/* 1 byte -		Payload Tag				*/
/* 1 byte -		Simulation Bit			*/
/* 2 bytes -	appid					*/
/* 6 bytes -	destination Mac			*/
/* 2 bytes -	TPID					*/
/* 2 bytes -	TCI						*/
/* 2 bytes -	pdu_len					*/
/* 2 bytes -    ethertype (88 b8)		*/
#define TUNNEL_PAYLOAD_HDR_SIZE			18				/* 1 byte tag + 1 byte Simulation + 2 bytes appID + 6 bytes dest mac address 2 bytes length = 6 + ethertype */
	case TUNNEL_PAYLOAD_TYPE_TAG:
		payload_len = (temp_payload_ptr->pduLen)+TUNNEL_PAYLOAD_HDR_SIZE;
		temp_payload_ptr= temp_payload_ptr->next;
		 break;

	default:							/* should never hit this default, but provided as a safety anyway will just use the common stuff */
		payload_len += (temp_payload_ptr->pduLen)+6;	/* add the PDU length  */
		temp_payload_ptr= temp_payload_ptr->next;
		break;		
	}
  }

//if the payload is to be encrypted, the payloads need to be constructed into a buffer and then encrypted
if(hdr->secAlgType!=SEC_ALG_NONE)
{
  encrypted_user_data_ptr = encrypt_user_data(hdr, &payload_len, payload);
  if (encrypted_user_data_ptr == NULL)			/*then there was an issue and need to halt the encode			*/
    {
      	  IEC905_ErrorHandler (INVALID_INPUT_PARAMETER,__FILE__,__LINE__);
	  return(NULL);
    }
} 

/* now have calculated the payload length, now need to calculate the SPDU length  */

  spdu_len = payload_len + IEC_90_5_SESSION_HDR_SIZE+sizeof(payload_len)-6; //-6 to remove SPDU lenght from calculation.

#define CLTP_HDR_SIZE 2;
  alloc_len = spdu_len + CLTP_HDR_SIZE;

  if(hdr->hmacAlg!=HMAC_ALG_None)		/*then we need to allocate additional bytes for the HMAC */
  {
#define MAX_HMAC_SIZE 64				/*oversized for now										*/
      alloc_len += MAX_HMAC_SIZE;
  }

  if((enc_buf = calloc (1, alloc_len))==NULL)
    {
      	  IEC905_ErrorHandler ( MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
	  if(encrypted_user_data_ptr!=NULL)
	    free(encrypted_user_data_ptr);
	  return(NULL);
    }

  cur_ptr = enc_buf;

  /* Encode CLTP Transport	*/
  *(cur_ptr++) = 0x01;		/* LI - Transport Unit Data header len (variable part empty)*/
  *(cur_ptr++) = 0x40;		/* Transport Unit Data PDU	*/
  /* Session	*/  
   spdu_start_ptr = cur_ptr;			/*save for the signature  */
  *(cur_ptr++) = hdr->SessionIdentifer;		/* SI (Session Identifier)	*/
  /* LI is len of header. DOES NOT include user data.	*/
  *(cur_ptr++) = hdr_len;	/* LI	*/

 
  *(cur_ptr++) = 0x80;			/* commonHeader	*/
  *(cur_ptr++) = common_hdr_len;	/* commonHeader	length	*/

  *(ST_UINT32 *)cur_ptr = htonl(spdu_len);	/* SPDU len	*/
  cur_ptr += sizeof(spdu_len);

  *(ST_UINT32 *)cur_ptr = htonl(spdu_num);	/* SPDU number	*/
  cur_ptr += sizeof(spdu_num);

  *(ST_UINT16 *)cur_ptr = htons(spdu_ver);	/* SPDU Version	*/
  cur_ptr += sizeof(spdu_ver);
  
  /* fill in the security informaiton */
  *(ST_UINT32 *)cur_ptr = htonl(hdr->timeOfCurrentKey);					
//  memcpy(cur_ptr,hdr->timeOfCurrentKey,4);
  cur_ptr+=4;
//need to calculate the timeToNextKey

  *(ST_UINT16 *)cur_ptr = htons(hdr->timeToNextKey);	/* SPDU Version	*/
   cur_ptr+=2;
  *(cur_ptr++)=hdr->secAlgType;
  *(cur_ptr++)=hdr->hmacAlg;

  /*fill in the key id							*/
  memcpy(cur_ptr,hdr->pKeyID,IEC_90_5_SIZE_OF_KEY_ID);


  cur_ptr += IEC_90_5_SIZE_OF_KEY_ID;


/* now need to peform the payload encoding					*/
  if (encrypted_user_data_ptr==NULL)
  {
	*(ST_UINT32 *)cur_ptr = htonl(payload_len);
	cur_ptr += sizeof(payload_len);
	cur_ptr = encode_payloads(payload,cur_ptr);
  }
  else
  {
	   *(ST_UINT32 *)cur_ptr = htonl(payload_len);
        cur_ptr += sizeof(payload_len);  
	    memcpy(cur_ptr, encrypted_user_data_ptr,payload_len);
		free(encrypted_user_data_ptr);
		cur_ptr +=payload_len;
  }

  enc_len = (ST_UINT)(cur_ptr - enc_buf);		/* then encode_length may not match the allocated length since the size of the signature is not known up front*/
  *enc_len_ptr = enc_len;

  spdu_num++;	/* increment global spdu_num for next PDU*/

  /* now need to do the signature	*/
  if( (hdr->hmacAlg>HMAC_ALG_None) && (hdr->hmacAlg<=HMAC_ALG_AES_GMAC_128))
  {
   *cur_ptr++ = 0x85;						/*tag for the signature			*/

   usr_create_HMAC( hdr, spdu_start_ptr, spdu_len, cur_ptr, enc_len_ptr);
  }

  if(*enc_len_ptr>MAX_ALLOWED_UDP_PACKET_LENGTH)      //did the encode for nothing since the packet is too large to avoid jumbo frames
    {
      *enc_len_ptr = 0;
      free(enc_buf);
      enc_buf = NULL;
      IEC905_ErrorHandler (LARGER_THAN_ALLOWED_PDU,__FILE__,__LINE__);
    }

  ++(*pSPDUnum);		//increment SPDU number
  return (enc_buf);
  }



  

  
