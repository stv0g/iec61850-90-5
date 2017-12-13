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
/* MODULE NAME : usr_sample.c						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Sample 90-5 application	code					    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 03/24/12  HSF	   Corrected calculation of key remaining time	    */
/* 03/24/12  HSF	   Corrected OptFld support			    */
/* 02/18/12  HSF	  Added sv_optflds to initialize function for sv    */
/* 12/01/11  HSF	   Initial Release				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"

#include <sys/types.h>
#include "signal.h"
#include "process.h"
#include "winsock2.h"



//#include <ws2tcpip.h>	/* for IP_MULTICAIEC_TTL	*/
#include "iec_90_5_load_cfg.h"
#include "iec_90_5.h"

#include "dblLnkLst.h"

static SOCKET IEC_90_5_rx_socket;


//statically defined PDUs to use to send
static unsigned char goose_packet[] = {0x61,0x82,0x02,0xf5,0x80,0x0d,
0x6d,0x79,0x64,0x6f,0x6d,0x2f,0x6d,0x79,0x67,0x63,0x52,0x65,0x66,0x81,0x01,0x04,
0x82,0x0f,0x6d,0x79,0x64,0x6f,0x6d,0x2f,0x6d,0x79,0x64,0x61,0x74,0x61,0x73,0x65,
0x74,0x83,0x09,0x74,0x65,0x73,0x74,0x41,0x70,0x70,0x49,0x44,0x84,0x08,0x3f,0xfb,
0x32,0x4c,0x00,0x00,0x00,0x00,0x85,0x01,0x01,0x86,0x01,0x01,0x87,0x01,0x00,0x88,
0x01,0x20,0x89,0x01,0x00,0x8a,0x01,0x03,0xab,0x82,0x02,0xa7,0xa2,0x82,0x02,0x65,
0xa2,0x35,0x85,0x01,0x03,0x87,0x05,0x08,0x40,0x87,0x7c,0xee,0x85,0x01,0x05,0x87,
0x05,0x08,0x40,0xc7,0x7c,0xee,0x85,0x01,0x07,0x87,0x05,0x08,0x41,0x03,0xc2,0x8f,
0x85,0x01,0x09,0x87,0x05,0x08,0x41,0x23,0xc2,0x8f,0x84,0x03,0x00,0x55,0x55,0x8c,
0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x2b,0x85,0x01,0x0f,0x87,0x05,0x08,0x41,
0x81,0xe1,0x48,0x85,0x01,0x11,0x87,0x05,0x08,0x41,0x91,0xe1,0x48,0x85,0x01,0x13,
0x87,0x05,0x08,0x41,0xa1,0xe1,0x48,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,
0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x19,0x87,0x05,0x08,0x41,0xd1,0xe1,0x48,
0x85,0x01,0x1b,0x87,0x05,0x08,0x41,0xe1,0xe1,0x48,0x85,0x01,0x1d,0x87,0x05,0x08,
0x41,0xf1,0xe1,0x48,0x85,0x01,0x1f,0x87,0x05,0x08,0x42,0x00,0xef,0x9e,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x25,
0x87,0x05,0x08,0x42,0x18,0xef,0x9e,0x85,0x01,0x27,0x87,0x05,0x08,0x42,0x20,0xef,
0x9e,0x85,0x01,0x29,0x87,0x05,0x08,0x42,0x28,0xef,0x9e,0x85,0x01,0x2b,0x87,0x05,
0x08,0x42,0x30,0xef,0x9e,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x31,0x87,0x05,0x08,0x42,0x48,0xef,0x9e,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x37,
0x87,0x05,0x08,0x42,0x60,0xef,0x9e,0x85,0x01,0x39,0x87,0x05,0x08,0x42,0x68,0xef,
0x9e,0x85,0x01,0x3b,0x87,0x05,0x08,0x42,0x70,0xef,0x9e,0x85,0x01,0x3d,0x87,0x05,
0x08,0x42,0x78,0xef,0x9e,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x43,0x87,0x05,0x08,0x42,0x88,0x77,0xcf,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x49,
0x87,0x05,0x08,0x42,0x94,0x77,0xcf,0x85,0x01,0x4b,0x87,0x05,0x08,0x42,0x98,0x77,
0xcf,0x85,0x01,0x4d,0x87,0x05,0x08,0x42,0x9c,0x77,0xcf,0x85,0x01,0x4f,0x87,0x05,
0x08,0x42,0xa0,0x77,0xcf,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x55,0x87,0x05,0x08,0x42,0xac,0x77,0xcf,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x5b,
0x87,0x05,0x08,0x42,0xb8,0x77,0xcf,0x85,0x01,0x5d,0x87,0x05,0x08,0x42,0xbc,0x77,
0xcf,0x85,0x01,0x5f,0x87,0x05,0x08,0x42,0xc0,0x77,0xcf,0x85,0x01,0x61,0x87,0x05,
0x08,0x42,0xc4,0x77,0xcf,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x67,0x87,0x05,0x08,0x42,0xd0,0x78,0x52,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x6d,
0x87,0x05,0x08,0x42,0xdc,0x78,0x52,0x85,0x01,0x6f,0x87,0x05,0x08,0x42,0xe0,0x78,
0x52,0x85,0x01,0x71,0x87,0x05,0x08,0x42,0xe4,0x78,0x52,0x85,0x01,0x73,0x87,0x05,
0x08,0x42,0xe8,0x78,0x52,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x79,0x87,0x05,0x08,0x42,0xf4,0x78,0x52,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x38,0x85,0x01,0x7f,
0x87,0x05,0x08,0x43,0x00,0x3c,0x29,0x85,0x02,0x00,0x81,0x87,0x05,0x08,0x43,0x02,
0x3c,0x29,0x85,0x02,0x00,0x83,0x87,0x05,0x08,0x43,0x04,0x3c,0x29,0x85,0x02,0x00,
0x85,0x87,0x05,0x08,0x43,0x06,0x3c,0x29,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,
0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x03,0x87,0x05,0x08,0x40,0x87,0x7c,
0xee,0x85,0x01,0x05,0x87,0x05,0x08,0x40,0xc7,0x7c,0xee,0x85,0x01,0x07,0x87,0x05,
0x08,0x41,0x03,0xc2,0x8f,0x85,0x01,0x09,0x87,0x05,0x08,0x41,0x23,0xc2,0x8f,0x84,
0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0x87,0x05,0x08,0x40,
0x87,0x7c,0xee
};

static unsigned char goose_tunnel_packet[] = {
0x00,0x00,0x03,0x01,0x00,0x00,0x00,0x00,0x61,0x82,0x02,0xf5,0x80,0x0d,
0x6d,0x79,0x64,0x6f,0x6d,0x2f,0x6d,0x79,0x67,0x63,0x52,0x65,0x66,0x81,0x01,0x04,
0x82,0x0f,0x6d,0x79,0x64,0x6f,0x6d,0x2f,0x6d,0x79,0x64,0x61,0x74,0x61,0x73,0x65,
0x74,0x83,0x09,0x74,0x65,0x73,0x74,0x41,0x70,0x70,0x49,0x44,0x84,0x08,0x3f,0xfb,
0x32,0x4c,0x00,0x00,0x00,0x00,0x85,0x01,0x01,0x86,0x01,0x01,0x87,0x01,0x00,0x88,
0x01,0x20,0x89,0x01,0x00,0x8a,0x01,0x03,0xab,0x82,0x02,0xa7,0xa2,0x82,0x02,0x65,
0xa2,0x35,0x85,0x01,0x03,0x87,0x05,0x08,0x40,0x87,0x7c,0xee,0x85,0x01,0x05,0x87,
0x05,0x08,0x40,0xc7,0x7c,0xee,0x85,0x01,0x07,0x87,0x05,0x08,0x41,0x03,0xc2,0x8f,
0x85,0x01,0x09,0x87,0x05,0x08,0x41,0x23,0xc2,0x8f,0x84,0x03,0x00,0x55,0x55,0x8c,
0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x2b,0x85,0x01,0x0f,0x87,0x05,0x08,0x41,
0x81,0xe1,0x48,0x85,0x01,0x11,0x87,0x05,0x08,0x41,0x91,0xe1,0x48,0x85,0x01,0x13,
0x87,0x05,0x08,0x41,0xa1,0xe1,0x48,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,
0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x19,0x87,0x05,0x08,0x41,0xd1,0xe1,0x48,
0x85,0x01,0x1b,0x87,0x05,0x08,0x41,0xe1,0xe1,0x48,0x85,0x01,0x1d,0x87,0x05,0x08,
0x41,0xf1,0xe1,0x48,0x85,0x01,0x1f,0x87,0x05,0x08,0x42,0x00,0xef,0x9e,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x25,
0x87,0x05,0x08,0x42,0x18,0xef,0x9e,0x85,0x01,0x27,0x87,0x05,0x08,0x42,0x20,0xef,
0x9e,0x85,0x01,0x29,0x87,0x05,0x08,0x42,0x28,0xef,0x9e,0x85,0x01,0x2b,0x87,0x05,
0x08,0x42,0x30,0xef,0x9e,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x31,0x87,0x05,0x08,0x42,0x48,0xef,0x9e,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x37,
0x87,0x05,0x08,0x42,0x60,0xef,0x9e,0x85,0x01,0x39,0x87,0x05,0x08,0x42,0x68,0xef,
0x9e,0x85,0x01,0x3b,0x87,0x05,0x08,0x42,0x70,0xef,0x9e,0x85,0x01,0x3d,0x87,0x05,
0x08,0x42,0x78,0xef,0x9e,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x43,0x87,0x05,0x08,0x42,0x88,0x77,0xcf,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x49,
0x87,0x05,0x08,0x42,0x94,0x77,0xcf,0x85,0x01,0x4b,0x87,0x05,0x08,0x42,0x98,0x77,
0xcf,0x85,0x01,0x4d,0x87,0x05,0x08,0x42,0x9c,0x77,0xcf,0x85,0x01,0x4f,0x87,0x05,
0x08,0x42,0xa0,0x77,0xcf,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x55,0x87,0x05,0x08,0x42,0xac,0x77,0xcf,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x5b,
0x87,0x05,0x08,0x42,0xb8,0x77,0xcf,0x85,0x01,0x5d,0x87,0x05,0x08,0x42,0xbc,0x77,
0xcf,0x85,0x01,0x5f,0x87,0x05,0x08,0x42,0xc0,0x77,0xcf,0x85,0x01,0x61,0x87,0x05,
0x08,0x42,0xc4,0x77,0xcf,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x67,0x87,0x05,0x08,0x42,0xd0,0x78,0x52,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x6d,
0x87,0x05,0x08,0x42,0xdc,0x78,0x52,0x85,0x01,0x6f,0x87,0x05,0x08,0x42,0xe0,0x78,
0x52,0x85,0x01,0x71,0x87,0x05,0x08,0x42,0xe4,0x78,0x52,0x85,0x01,0x73,0x87,0x05,
0x08,0x42,0xe8,0x78,0x52,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,
0x17,0x86,0xa2,0x17,0x85,0x01,0x79,0x87,0x05,0x08,0x42,0xf4,0x78,0x52,0x84,0x03,
0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0xa2,0x38,0x85,0x01,0x7f,
0x87,0x05,0x08,0x43,0x00,0x3c,0x29,0x85,0x02,0x00,0x81,0x87,0x05,0x08,0x43,0x02,
0x3c,0x29,0x85,0x02,0x00,0x83,0x87,0x05,0x08,0x43,0x04,0x3c,0x29,0x85,0x02,0x00,
0x85,0x87,0x05,0x08,0x43,0x06,0x3c,0x29,0x84,0x03,0x00,0x55,0x55,0x8c,0x06,0x29,
0x32,0x2e,0x00,0x17,0x86,0xa2,0x35,0x85,0x01,0x03,0x87,0x05,0x08,0x40,0x87,0x7c,
0xee,0x85,0x01,0x05,0x87,0x05,0x08,0x40,0xc7,0x7c,0xee,0x85,0x01,0x07,0x87,0x05,
0x08,0x41,0x03,0xc2,0x8f,0x85,0x01,0x09,0x87,0x05,0x08,0x41,0x23,0xc2,0x8f,0x84,
0x03,0x00,0x55,0x55,0x8c,0x06,0x29,0x32,0x2e,0x00,0x17,0x86,0x87,0x05,0x08,0x40,
0x87,0x7c,0xee
};




struct sv_thread_info{			  //just used to pass paramters to the demo SV transmit Thread
  KDC_REF *pMyKDCRef;
  char * pInterfaceID;
  IEC_COMM_ADDRESS *pSrcAddress;
  IEC_COMM_ADDRESS *pDestAddress;
  ETHERTYPE_8021Q Val8021Q;
  USHORT IP_TOS_DSCP;
  DWORD threadDelay;
  int *pCancelThreads;
};



static int DoIt = TRUE;
static STARTUP_CFG cfg;		      //holds configuration information
static FILE *StatisticFile=NULL;      //name of the statistics file to write information into.

typedef struct parseInfoQueue {	      //structure of information used to pass information from the 
  DBL_LNK l;			      //socket receive thread to the parse thread
  IEC_90_5_RX *pMyRxCntrl;
  IEC_COMM_ADDRESS  *pDestAddr;
}PARSEINFOQUEUE;


PARSEINFOQUEUE *listHead;	     //list of information/queue between the receive socket thread and parse thread

static CRITICAL_SECTION  _dblLnkCriticalSection;  //used as a mutex between receive socket and parse thread
						  //to control access to the queue


/************************************************************************/
/* 		usr_create_kdc_credentials				*/
/*									*/
/*  function allocates and initializes the credential information	*/
/*  Inputs:	 None							*/
/*									*/
/* Return: NULL or a pointer to a credential structure			*/
/************************************************************************/
static KDC_CREDENTIALS *usr_create_kdc_credential()
{
  KDC_CREDENTIALS *pCredential=NULL;

      if((pCredential = calloc (1, sizeof(KDC_CREDENTIALS)))==NULL)
	IEC905_ErrorHandler( MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);

      return(pCredential);
}

/************************************************************************/
/*		  example_tunnel_enc 					*/
/* uses static PDU and encodes/sends it via 90-5 as a payload		*/
/************************************************************************/
unsigned char *example_tunnel_enc (IEC905_MSG_CNTRL *pMsgCntrl,
	unsigned char *dIEC_mac,		/* DST MAC (must be 6 bytes)	*/
	ST_UINT16 tci,			/* TCI from original message*/
	ST_UINT16 etype_id,	/* Ethertype ID from original message*/
	ST_UCHAR *etype_pdu_ptr,	/* pointer after Ethertype ID	*/
	ST_UINT16 etype_pdu_len,	/* len of PDU after Ethertype ID*/
	ST_UINT *enc_len_ptr)
  {

IEC905_SESS_PDU_HDR_INFO hdr_info;
IEC905_SESS_PAYLOAD_DEF payload_info;
unsigned char *temp_ptr;
unsigned char key_id[4];
time_t seconds;

iec905_manage_keys(pMsgCntrl);				//check about the keys
/* example of filling in the hearder  */
hdr_info.SessionIdentifer = IEC_90_5_SI_TYPE_TUNNEL;
hdr_info.hmacAlg = HMAC_ALG_SHA_256_128;;
hdr_info.secAlgType = SEC_ALG_NONE	;
hdr_info.pKey = pMsgCntrl->pCurrentKey->pKey;;
hdr_info.keyLen = pMsgCntrl->pCurrentKey->keyLen;
seconds = time(NULL);
hdr_info.timeToNextKey=  (ST_UINT16)(pMsgCntrl->pCurrentKey-> timeOfexpiration -seconds)/60;
hdr_info.timeOfCurrentKey=(ST_UINT32)pMsgCntrl->pNextKey->timeOfInitialUse;;
memcpy(key_id,pMsgCntrl->pCurrentKey->key_id,4);
hdr_info.pKeyID = key_id;

/* fill in the payload_information  */
payload_info.payload_tag = TUNNEL_PAYLOAD_TYPE_TAG;
payload_info.appID = 0;
payload_info.pduLen = etype_pdu_len;
payload_info.pPDU = etype_pdu_ptr;
payload_info.etype_id = etype_id;
payload_info.simulationBit = 0x00;
payload_info.etype_id = etype_id;
payload_info.tci = tci;
payload_info.tpid = 0x00;
memcpy(&payload_info.dst_mac,dIEC_mac,6);
payload_info.next = NULL;

  temp_ptr = iec905_sess_enc(&hdr_info,&payload_info,enc_len_ptr,&pMsgCntrl->spdu_num_tx);
  return (temp_ptr);
  }


/************************************************************************/
/*		  example_GOOSE_enc					*/
/* uses static GOOSE PDU and encodes/sends it via 90-5 as a payload	*/
/************************************************************************/
unsigned char *example_GOOSE_enc (IEC905_MSG_CNTRL *pMsgCntrl,
	ST_UINT16 enc_pdu_len,	
	ST_UCHAR *enc_ptr,
	ST_UINT *enc_len_ptr)
  {

IEC905_SESS_PDU_HDR_INFO hdr_info;
IEC905_SESS_PAYLOAD_DEF payload_info;
IEC905_SESS_PAYLOAD_DEF payload_info1;
unsigned char *temp_ptr;
unsigned char key_id[4];
time_t seconds;

iec905_manage_keys(pMsgCntrl);				//check about the keys
/* example of filling in the hearder  */
hdr_info.SessionIdentifer = IEC_90_5_SI_TYPE_GOOSE;
hdr_info.hmacAlg = HMAC_ALG_SHA_256_128;;
hdr_info.secAlgType = SEC_ALG_NONE	;
hdr_info.pKey = pMsgCntrl->pCurrentKey->pKey;
hdr_info.keyLen = pMsgCntrl->pCurrentKey->keyLen;
seconds = time(NULL);
hdr_info.timeToNextKey=  (ST_UINT16)(pMsgCntrl->pCurrentKey-> timeOfexpiration -seconds)/60;
hdr_info.timeOfCurrentKey=(ST_UINT32)pMsgCntrl->pNextKey->timeOfInitialUse;
memcpy(key_id,pMsgCntrl->pCurrentKey->key_id,4);
hdr_info.pKeyID = key_id;

/* fill in the payload_information  */
payload_info.payload_tag = GOOSE_PAYLOAD_TYPE_TAG;
payload_info.appID = 0;
payload_info.pduLen = enc_pdu_len;
payload_info.pPDU = enc_ptr;
payload_info.simulationBit = 0x00;
payload_info.next = NULL;


payload_info1.payload_tag = GOOSE_PAYLOAD_TYPE_TAG;	    //example of how to send multiple payloads
payload_info1.appID = 0;
payload_info1.pduLen = enc_pdu_len;
payload_info1.pPDU = enc_ptr;
payload_info1.simulationBit = 0x00;
payload_info1.next = NULL;
payload_info.next = &payload_info1;


  temp_ptr = iec905_sess_enc(&hdr_info,&payload_info,enc_len_ptr,&pMsgCntrl->spdu_num_tx);
  return (temp_ptr);
  }


/************************************************************************/
/*			      example_SV_enc				*/
/* example of how to use the supplied SV encoder to format a SV payload	*/
/* and send it via 90-5							*/
/************************************************************************/
unsigned char *example_SV_enc (IEC905_MSG_CNTRL *pMsgCntrl,
	ST_UINT16 enc_pdu_len,	
	ST_UCHAR *enc_ptr,
	ST_UINT *enc_len_ptr)
  {

IEC905_SESS_PDU_HDR_INFO hdr_info;
IEC905_SESS_PAYLOAD_DEF payload_info;
unsigned char *temp_ptr;
unsigned char key_id[4];
time_t seconds;

iec905_manage_keys(pMsgCntrl);				//check about the keys
/* example of filling in the hearder  */
hdr_info.SessionIdentifer = IEC_90_5_SI_TYPE_SV;
hdr_info.hmacAlg = HMAC_ALG_SHA_256_128;;
hdr_info.secAlgType = SEC_ALG_NONE	;
hdr_info.pKey = pMsgCntrl->pCurrentKey->pKey;
hdr_info.keyLen = pMsgCntrl->pCurrentKey->keyLen;
seconds = time(NULL);
hdr_info.timeToNextKey=  (ST_UINT16)(pMsgCntrl->pCurrentKey-> timeOfexpiration -seconds)/60;
hdr_info.timeOfCurrentKey= (ST_UINT32)pMsgCntrl->pNextKey->timeOfInitialUse;

memcpy(key_id,pMsgCntrl->pCurrentKey->key_id,4);
hdr_info.pKeyID = key_id;

/* fill in the payload_information  */
payload_info.payload_tag = SV_PAYLOAD_TYPE_TAG ;
payload_info.appID = 0;
payload_info.pduLen = enc_pdu_len;
payload_info.pPDU = enc_ptr;
payload_info.simulationBit = 0x00;
payload_info.next = NULL;

  temp_ptr = iec905_sess_enc(&hdr_info,&payload_info,enc_len_ptr, &pMsgCntrl->spdu_num_tx);
  return (temp_ptr);
  }




/************************************************************************/
/*			      SoftwareInitalizationSequence		*/
/* example of Software Initialization Sequence - see source code 	*/
/* documentation							*/
/************************************************************************/
static int SoftwareInitalizationSequence()
{
int retVal;
KDC_CREDENTIALS *pMyLocalCredentials;

    iec905_init_key_storage();				//no error is returned

  //initialize ip socket interface
  if((retVal = iec905_ip_init())!=SUCCESS_IEC905)				
    {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);
    return(retVal);
    }
  
  //create a local credential structure
  if((pMyLocalCredentials = usr_create_kdc_credential())==NULL)  //for the local crendentials
    {
    IEC905_ErrorHandler (FAILURE_IEC905, __FILE__, __LINE__);
    return(FAILURE_IEC905);
    }
		
  //place holder for filling in the actual credentials when that is all worked out

  //now initialize the interface to the KDC with the local credentials

  if((retVal = iec905_init_kdc_interface(pMyLocalCredentials,10))!=SUCCESS_IEC905)
    {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);
    free(pMyLocalCredentials);
    return(retVal);
    }
 
  //the local credentials can no be freed since the init function creates an
  //internal copy of the information

   if((retVal=iec905_tx_pdu_init())!=SUCCESS_IEC905)
    {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);
    free(pMyLocalCredentials);
    return(retVal);
    }


  free(pMyLocalCredentials);
  return(SUCCESS_IEC905);
}



/************************************************************************/
/*			      transmit_sequence_sv_thread		*/
/* Thread that shows how to set up and encode an SV at APDU level  	*/
/* then also sends							*/
/************************************************************************/
static void transmit_sequence_sv_thread(void *arg)
{
struct sv_thread_info *pMyThreadInfo = (struct sv_thread_info *)arg;
KDC_REF *pMyKDCRef;
IEC905_MSG_CNTRL *pMyCntrl;
IECSOCKET_TX_INFO *mysockInfo=NULL;
SV_ENC_STRUCT *mySVInfo;
int retVal;
int offset_of_data,i,j;
unsigned char * pRetBuf;
int total_size;
int ret_len;
unsigned char *asdu_data; 
unsigned int z=0;
ST_UINT8 sv_optflds;
ST_UINT16 smp_mod;



  
  pMyKDCRef = pMyThreadInfo->pMyKDCRef;
//transmit sequence 

  //if NULL is returned that there is a matching control structure that is already in use.  
  //Duplicate Control structures are not allowed
   if((pMyCntrl=iec905_create_msg_cntrl_tx( IEC_KEY_USAGE_TYPE_SV, pMyThreadInfo->pSrcAddress, pMyThreadInfo->pDestAddress, "SISCOExample/MYSVDataSet"))==NULL)
	 return;

  //if success is not returned, indicates that the KDC Reference is bad
  if((retVal=iec905_kdc_bind(pMyCntrl,pMyKDCRef))!=SUCCESS_IEC905)	  
    {
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }

  //if success is not returned, indicates that keys could not be obtained.
  if((retVal=iec905_get_kdc_tx_keys(pMyCntrl))!=SUCCESS_IEC905)
    {
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }


  //   open_tx_udp_socket_with_priorities(InterfaceID ,&Val8021Q, &IP_TOS_DSCP, &keyAddress);
  if((retVal=iec905_tx_pdu_open_socket_with_priorities(&mysockInfo,pMyThreadInfo->pInterfaceID ,&pMyThreadInfo->Val8021Q, &pMyThreadInfo->IP_TOS_DSCP, pMyThreadInfo->pDestAddress))!=SUCCESS_IEC905)
      {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);
	//things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);

    return;
    }
 
  if(mysockInfo==NULL)
    {
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }
  
  //the sending methodolgy is to encode a packet and then call the transmit function

  //calculate the size of the samples in the ASDU (example shows 10 SV_INT32 values
  total_size = 0;

#define NUMBER_OF_SV_ULONGS 256
  for(i = 0;i<NUMBER_OF_SV_ULONGS;++i)
  {
    offset_of_data= sv_data_helper_calculate_samples_size_prim (SV_INT32, 0, &ret_len, &total_size);
  }

#define NUM_ASDUS_TO_DEFINE_MAX 3
  sv_optflds =  SV_INCLUDE_REFRESH_TIME + SV_INCLUDE_DATA_SET +SV_INCLUDE_UTC_TIME;	    //change 8//1
  smp_mod = 1;
  if((mySVInfo= initialize_sv_encode_struct ("ExampleSVID", "SISCOExample/MYSVDataSet",NUM_ASDUS_TO_DEFINE_MAX ,1,0x1,16,&smp_mod,total_size,sv_optflds))==NULL)
    {
    iec905_destroy_msg_cntrl(pMyCntrl);
    iec905_tx_pdu_close_socket(mysockInfo);
    mysockInfo = NULL;
    return;
    }

  asdu_data = mySVInfo->asdus[0].pASDU_data;
  offset_of_data= 0;
  j=0;

  while(*(pMyThreadInfo->pCancelThreads) == FALSE)
  {
    offset_of_data = 0;
    for(i=0;i<NUMBER_OF_SV_ULONGS;++i)
    { 
      *(ST_UINT32 *)(asdu_data+offset_of_data) = htonl(j);//need to put things in the buffer in network order
      ++j;
      offset_of_data= sv_data_helper_calculate_samples_size_prim (SV_INT32, offset_of_data, &ret_len, &total_size);
    }

    //transmit 2 ASDUs for the UDP test
    sv_enc_update_lengths( mySVInfo,2);

    pRetBuf = example_SV_enc (pMyCntrl, mySVInfo->length_to_send,mySVInfo->pSVbuffer, &ret_len);
  
     if(pRetBuf!=NULL)
       {
       iec905_tx_pdu (mysockInfo,pMyThreadInfo->pDestAddress, 102,pRetBuf,ret_len,pMyCntrl);
       free(pRetBuf);
	   }
	 else
	   {
	   //things that are created, must be destroyed.
       iec905_destroy_msg_cntrl(pMyCntrl);
       iec905_tx_pdu_close_socket(mysockInfo);
 //      free(mysockInfo);
       return;
	   }
     Sleep((cfg.transmissionIntervalMsec+pMyThreadInfo->threadDelay));
  }

  iec905_tx_pdu_close_socket(mysockInfo);
}


/************************************************************************/
/*			      transmit_sequence_goose_thread		*/
/* Thread causes a GOOSE payload/90-5 PDU to be sent	        	*/
/* then also sends							*/
/************************************************************************/
static void transmit_sequence_goose_thread(void *arg)
{
struct sv_thread_info *pMyThreadInfo = (struct sv_thread_info *)arg;
KDC_REF *pMyKDCRef;
IEC905_MSG_CNTRL *pMyCntrl;
IECSOCKET_TX_INFO *mysockInfo=NULL;
int retVal;
unsigned char * pRetBuf;
int ret_len;

  
  pMyKDCRef = pMyThreadInfo->pMyKDCRef;
//transmit sequence 

  //if NULL is returned that there is a matching control structure that is already in use.  
  //Duplicate Control structures are not allowed
  if((pMyCntrl=iec905_create_msg_cntrl_tx( IEC_KEY_USAGE_TYPE_GOOSE, pMyThreadInfo->pSrcAddress, pMyThreadInfo->pDestAddress, "SISCOExample/MYGOOSEDataSet"))==NULL)
    return;

  //if success is not returned, indicates that the KDC Reference is bad
  if((retVal=iec905_kdc_bind(pMyCntrl,pMyKDCRef))!=SUCCESS_IEC905)
    {
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }
    

  //if success is not returned, indicates that keys could not be obtained.
  if((retVal=iec905_get_kdc_tx_keys(pMyCntrl))!=SUCCESS_IEC905)
    {
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }


  //   open_tx_udp_socket_with_priorities(InterfaceID ,&Val8021Q, &IP_TOS_DSCP, &keyAddress);
  if((retVal=iec905_tx_pdu_open_socket_with_priorities(&mysockInfo,pMyThreadInfo->pInterfaceID ,&pMyThreadInfo->Val8021Q, &pMyThreadInfo->IP_TOS_DSCP, pMyThreadInfo->pDestAddress))!=SUCCESS_IEC905)
      {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }
 
  if(mysockInfo==NULL)
    {
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }

  while(*(pMyThreadInfo->pCancelThreads) == FALSE)
  {

    pRetBuf = example_GOOSE_enc (pMyCntrl, sizeof(goose_packet),goose_packet, &ret_len);

    if(pRetBuf!=NULL)
      {
      iec905_tx_pdu (mysockInfo,pMyThreadInfo->pDestAddress, 102,pRetBuf,ret_len,pMyCntrl);
      free(pRetBuf);
      }
    else
      {
      //things that are created, must be destroyed.
       iec905_destroy_msg_cntrl(pMyCntrl);
       iec905_tx_pdu_close_socket(mysockInfo);
       return;
      }

    Sleep((cfg.transmissionIntervalMsec+pMyThreadInfo->threadDelay));
  }

  iec905_tx_pdu_close_socket(mysockInfo);
}

/************************************************************************/
/*		transmit_sequence_tunnelled_goose_thread		*/
/* Thread causes a Tunnelled payload/90-5 PDU to be sent	       	*/
/* then also sends							*/
/************************************************************************/
static void transmit_sequence_tunnelled_goose_thread(void *arg)
{
struct sv_thread_info *pMyThreadInfo = (struct sv_thread_info *)arg;
KDC_REF *pMyKDCRef;
IEC905_MSG_CNTRL *pMyCntrl;
IECSOCKET_TX_INFO *mysockInfo=NULL;
int retVal;
unsigned char * pRetBuf;
int ret_len;
ST_UCHAR teIEC_mac[6] = {0x01,0x33,0x34,0x35,0x36,0x37};


  pMyKDCRef = pMyThreadInfo->pMyKDCRef;
//transmit sequence 

  //if NULL is returned that there is a matching control structure that is already in use.  
  //Duplicate Control structures are not allowed
  if((pMyCntrl=iec905_create_msg_cntrl_tx(IEC_KEY_USAGE_TYPE_TUNNEL, pMyThreadInfo->pSrcAddress, pMyThreadInfo->pDestAddress, "SISCOExample/MYGOOSEDataSet"))==NULL)
	return;

  //if success is not returned, indicates that the KDC Reference is bad
  if((retVal=iec905_kdc_bind(pMyCntrl,pMyKDCRef))!=SUCCESS_IEC905)
    {
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }

  //if success is not returned, indicates that keys could not be obtained.
  if((retVal=iec905_get_kdc_tx_keys(pMyCntrl))!=SUCCESS_IEC905)
    {
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }


  //   open_tx_udp_socket_with_priorities(InterfaceID ,&Val8021Q, &IP_TOS_DSCP, &keyAddress);
  if((retVal=iec905_tx_pdu_open_socket_with_priorities(&mysockInfo,pMyThreadInfo->pInterfaceID ,&pMyThreadInfo->Val8021Q, &pMyThreadInfo->IP_TOS_DSCP, pMyThreadInfo->pDestAddress))!=SUCCESS_IEC905)
      {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);  
    //things that are created, must be destroyed.
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
    }
 

  if(mysockInfo==NULL)	      //to satisfy Klockwork
   {
    iec905_destroy_msg_cntrl(pMyCntrl);
    return;
   }

  //the sending methodolgy is to encode a packet and then call the transmit function

  //calculate the size of the samples in the ASDU (example shows 10 SV_INT32 values


  while(*(pMyThreadInfo->pCancelThreads) == FALSE)
  {

    pRetBuf= example_tunnel_enc(pMyCntrl,teIEC_mac,0x3333,0x88b8,goose_tunnel_packet,sizeof(goose_tunnel_packet),&ret_len);

    if(pRetBuf!=NULL)
      {
      iec905_tx_pdu (mysockInfo,pMyThreadInfo->pDestAddress, 102,pRetBuf,ret_len,pMyCntrl);
      free(pRetBuf);
	  }
	else
	  {
      //things that are created, must be destroyed.
       iec905_destroy_msg_cntrl(pMyCntrl);
       iec905_tx_pdu_close_socket(mysockInfo);
       return;
      }
 //   printf("+");
    Sleep((cfg.transmissionIntervalMsec+pMyThreadInfo->threadDelay));
  }

  iec905_tx_pdu_close_socket(mysockInfo);
}

/************************************************************************/
/*		sample_usr_sv_decode					*/
/* Example of decoding SV packet that was received		       	*/
/*	      								*/
/************************************************************************/
static void sample_usr_sv_decode(ST_UINT16 len, ST_UCHAR *pPdu, IEC_90_5_RX *pMyRxInfo)
{
    //inputs the length of the data and the start of the data pointer.
    //note, the pPdu, in the payload is not allocated, it is just a pointer
    //into the decoded buffer. The RXInfo pointer is provided so that the
    //user can destroy strcuture, allocated payloads, and the allocated buffer that held the 90-5 packet
    //when the use of the information is complete. 

SV_DEC_STRUCT *pSVinfo;		    //the call to the decode function allocates this, the user may free it
				    //when done.  It may not be used (due to pointers into the pMyRxInfo buffer)
				    //after the pMyRxInfo is destroyed
ASDU_DEC_INFO *pASDUinfo;	    //there may be multple ASDUs in the sv packet, will only deal with the 1st one.
				    //the actual number of ASDUs is found in pSVInfo->num_ASDUs.
unsigned char *pNullTerminate;
   

  if((pSVinfo=sv_decode(pPdu,len))!=NULL)   //decode was successful and can use the information
    {					    //for demonstration purposes, will just print out the dataSet information
	pASDUinfo = &pSVinfo->asdus[0];
	if(pASDUinfo->datSetRef.pValue)
	  {
           pNullTerminate = pASDUinfo->datSetRef.pValue + pASDUinfo->datSetRef.len;
	   *pNullTerminate = 0;		    //know that the next byte in the buffer is a tag, so just overwrite it with a zero
	  }
//	printf("%s",pASDUinfo->datSetRef.pValue);

	//remember other values are in the buffer in network order so must be converted appropriately

	//the user can hold on to the Ctnrl and decoded SV information for as long as they want
	//when done free the pSVinfo and destroy the Ctnrl
	free(pSVinfo);
     }


}

/*****************************************************************************/
/*		sample_usr_iec905_rx  					     */
/* Sample user function to receive the IEC90-5 information that has been     */
/* received and then performs the APDU decode (only SV support currently)    */
/*****************************************************************************/
static void sample_usr_iec905_rx(IEC_90_5_RX *pMyRxInfo)
{
IEC905_SESS_PAYLOAD_DEF *payload;		
		
		

  payload = pMyRxInfo->pPayload;
  while(payload)		  //need to loop through the entire chaing of payloads and decode
  {
    switch(payload->payload_tag)
      {
      case SV_PAYLOAD_TYPE_TAG:	  //actually have a SV decoder provided
	sample_usr_sv_decode(payload->pduLen, payload->pPDU, pMyRxInfo);
        break;

      case GOOSE_PAYLOAD_TYPE_TAG:  //currently don't have a decoder
      case TUNNEL_PAYLOAD_TYPE_TAG:  //currently don't have a decoder
      case MNGT_PAYLOAD_TYPE_TAG:   //currently don't have a decoder
      default:
	break;
      }
    payload=payload->next;
  }

  //destroys the strcuture, allocated payloads, and the allocated buffer that held the 90-5 packet.
  iec905_destroy_dec_info( pMyRxInfo);	
}

/*****************************************************************************/
/*		parse_function 						      */
/* Sample user function to receive the IEC90-5 information that has been     */
/* received and then performs the APDU decode (only SV support currently)    */
/*****************************************************************************/
static void parse_function(IEC_90_5_RX *pMyRxCntrl,IEC_COMM_ADDRESS  *pDestAddr)
{
int retVal;


	  if((retVal =iec905_sess_dec(pMyRxCntrl ,pDestAddr))==SUCCESS_IEC905)	   /*points to the rxd information	*/
            {
	    sample_usr_iec905_rx(pMyRxCntrl);
	    if(pDestAddr->pAddress!=NULL)
	      free(pDestAddr->pAddress);
	    free(pDestAddr);
	    }
	  else 
	    {
            iec905_destroy_dec_info(  pMyRxCntrl);
	    free(pDestAddr->pAddress);
	    free(pDestAddr); 
	    }
}


/*****************************************************************************/
/*		parse_thread						     */
/* Pulls information from a queue that has the received information from     */
/* the receive socket thread						     */
/*****************************************************************************/
static void parse_thread(void *args)
{
PARSEINFOQUEUE *parseStuff;

    while(1)
    {
      while(listHead)	//then there is something to do
      {
        EnterCriticalSection(&_dblLnkCriticalSection);	    //start of critical section to see if there is something on the head of the list to parse
        parseStuff = (PARSEINFOQUEUE *)dblLnkUnlinkFirst ((DBL_LNK**)&listHead);
        LeaveCriticalSection(&_dblLnkCriticalSection);
        if(parseStuff!=NULL)
        {
        parse_function(parseStuff->pMyRxCntrl,parseStuff->pDestAddr);
        free(parseStuff);
        }
      }
    Sleep(1);
  }
}

/*****************************************************************************/
/*		sample_rxd_thread					      */
/*****************************************************************************/
static void sample_rxd_thread(void *args)
{
int fromlen, byte_count;
struct sockaddr addr;
struct sockaddr_in *pIPv4Addr;
unsigned char *pDecodeBuffer;
IEC_90_5_RX *pMyRxCntrl;
int nfds,retVal;
fd_set readfds;
IEC_COMM_ADDRESS *pDestAddr;
#define MAX_RXD_WAIT_BEFORE_WAKEUP 5000 //usec
long spdu_len;

#define MAX_UDP_BUF_SIZE 64000
static unsigned char buf[MAX_UDP_BUF_SIZE];
PARSEINFOQUEUE *parseInfo;



  //initalize the UDP receive socket (102) for 90-5 and store the results in the passed Socket Pointer

  if((retVal = iec905_rx_init(&IEC_90_5_rx_socket, cfg.updScktBufSize))!=SUCCESS_IEC905)		
    {
    IEC905_ErrorHandler (retVal, __FILE__, __LINE__);
    return;
    }


     FD_ZERO (&readfds);
     FD_SET (IEC_90_5_rx_socket, &readfds);
     /* Wait forever for incoming UDP packet.    */
#if defined(_WIN32)
     nfds = 1;    /* On _WIN32, ignored, but compiler wants it 
initialized.*/
#else
     nfds = ReceivingSocket+1;    /* ReceivingSocket is only fds to check, so set "nfds=ReceivingSocket+1".*/
#endif
   while (1)
     {
     nfds = select (nfds, &readfds, NULL, NULL, NULL);    /* wait forever    */
    if(nfds == SOCKET_ERROR)
    {
        printf("Error %d occured\n",WSAGetLastError());
    }
    else if (nfds > 0)
       {    /* UDP packet available on the socket. Receive it.    */
       //get the length of the packet waiting
       fromlen = sizeof(addr);
      //line removed due to performance issues
       byte_count = recvfrom(IEC_90_5_rx_socket,buf, MAX_UDP_BUF_SIZE,MSG_PEEK, &addr, &fromlen);
       if(byte_count>0)
	 {
         if((pDecodeBuffer = malloc(byte_count))!=NULL)
	  {
	    //line removed due to performance issues
	    byte_count = recvfrom(IEC_90_5_rx_socket,pDecodeBuffer, byte_count,0, &addr, &fromlen);

#define SPDU_LEN_OFFSET 6
	    spdu_len = ntohl(*(ST_UINT32 *)(pDecodeBuffer +SPDU_LEN_OFFSET));
	    spdu_len +=21;		    


	     if((pMyRxCntrl = iec905_create_dec_info())!=NULL)
	      {
	       pMyRxCntrl->lenRXDBuffer = byte_count;
	       pMyRxCntrl->pRXDbuffer = pDecodeBuffer;
	      }
	    else
	      {
	      IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
	      }


	    if(addr.sa_family==AF_INET)
	      {
	      pDestAddr = calloc(1,sizeof(IEC_COMM_ADDRESS));
	      if(pDestAddr!=NULL)
		{
		pIPv4Addr = (struct sockaddr_in *)&addr;
		pDestAddr->typeOfAddress= IP_V4_ADDRESS_TYPE;
		pDestAddr->lenOfAddress =4;
		if((pDestAddr->pAddress = malloc(SIZEOF_IPV4_ADDRESS))!=NULL)
		  memcpy(pDestAddr->pAddress , (unsigned char *)&pIPv4Addr->sin_addr.S_un.S_addr,SIZEOF_IPV4_ADDRESS);
		parseInfo = malloc(sizeof(PARSEINFOQUEUE));
		if((parseInfo!=NULL)&& (pMyRxCntrl!=NULL))
		  {
		  parseInfo->pMyRxCntrl = pMyRxCntrl;
		  parseInfo->pDestAddr = pDestAddr;
		  EnterCriticalSection(&_dblLnkCriticalSection);
		  dblLnkAddLast  ((DBL_LNK **)&listHead, (DBL_LNK *)parseInfo);	
		  LeaveCriticalSection(&_dblLnkCriticalSection);
		  }
	       else
		  {
		  if(pMyRxCntrl!=NULL)
		     free(pMyRxCntrl);
		  
		  if(pDecodeBuffer!=NULL)
		     free(pDecodeBuffer);

		  if(pDestAddr->pAddress!=NULL)
		    free(pDestAddr->pAddress);

		  if(pDestAddr!=NULL)
		    free(pDestAddr);

		  if(parseInfo!=NULL)
		    free(parseInfo);
		  
		  }
	       }
	      else
		{
		free(pDecodeBuffer);
		free(pMyRxCntrl);
		}
	      }
	    else
	      {
	      free(pDecodeBuffer);
	     free(pMyRxCntrl);
	      }
	    }
	  }
	else
	  {
	  //declare an allocation error, read the socket and continue like nothing happenned
	  IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
          byte_count = recvfrom(IEC_90_5_rx_socket,buf, byte_count,0, &addr, &fromlen); 
	  }
      }
    }
 }




/*****************************************************************************/
/*		get_stats_thread					     */
/* Thread that updates the statistics in the display and writes to the       */
/* statistics file							    */
/*****************************************************************************/
static void get_stats_thread(void *args)
{
IEC905_STATISTICS stats;
time_t reset;
#define DEFAULT_TIME_DELAY_TO_RESET 3600 //1 hour
int elapsedTimeToReset=DEFAULT_TIME_DELAY_TO_RESET; 
time_t timeToLog;
unsigned int logInterval=0;
unsigned long int logCnt=0;

 timeToLog = time(NULL);
 if((StatisticFile!=NULL) && (cfg.logIntervalSeconds>0))
    {
    logInterval = cfg.logIntervalSeconds;
    fprintf(StatisticFile,"TotalTxPktCnt,TotalRxPktCnt,TotalRxMissingPktCnt,TotalRxPktWithBadHMAC\n");
    timeToLog = time(NULL) + logInterval; 
    }


if(cfg.statResetTime!=0)
  elapsedTimeToReset = cfg.statResetTime;

reset = time(NULL) + elapsedTimeToReset;

 while(1)
  {

   if(time(NULL)>=reset)
    {
    iec905_getStats(&stats, TRUE);
    reset = time(NULL)+elapsedTimeToReset;
    }
   else
        iec905_getStats(&stats, FALSE);

  printf("\n\n*************** STATS  **************************\n");
  printf("Total Transmitted Packets: %lu\n",stats.TotalTxPktCnt);
  printf("Total Packets Received: %lu\n",stats.TotalRxPktCnt);
  printf("Total Missing Pakcets: %lu\n",stats.TotalRxMissingPktCnt);
  printf("Total Bad HMAC Packets: %lu\n",stats.TotalRxPktWithBadHMAC);
  printf("Number of records exported: %lu\n",logCnt);
  printf("*************************************************\n");

  if((time(NULL) > timeToLog) && (logInterval!=0))
    {
    fprintf(StatisticFile,"%lu,%lu,%lu,%lu\n",stats.TotalTxPktCnt,stats.TotalRxPktCnt,stats.TotalRxMissingPktCnt,stats.TotalRxPktWithBadHMAC);
    timeToLog = time(NULL) + logInterval;
    ++logCnt;
    }
  Sleep(5000);
  }
}


/************************************************************************/
/*                      cntrlCfun					*/
/*  function that receives a CNTRL-C and causes main() to exit		*/
/************************************************************************/
ST_VOID ctrlCfun (int i)
   {
   DoIt = FALSE;
   }

/************************************************************************/
/*                       main						*/
/************************************************************************/
int main (int argc, char *argv[])
  {
char InterfaceID[MAX_ALLOWED_VALUE_SIZE];
ETHERTYPE_8021Q Val8021Q;
USHORT IP_TOS_DSCP=46;
IECSOCKET_TX_INFO *mysockInfo=NULL;
int retVal,i;
KDC_CREDENTIALS *pKDCCredential1;
KDC_CREDENTIALS *pKDCCredential2;
KDC_REF *pRetKDCRef;
IEC_COMM_ADDRESS kdcAddress;	      //doesn't have anything in it, but is used to show the process
struct sv_thread_info mySVThreadInfo;
struct sv_thread_info myGOOSEThreadInfo;
struct sv_thread_info myTunnelledGOOSEThreadInfo;
int cancelThreads=FALSE;
IEC905_MSG_CNTRL *pMyRxCntrl;
IEC_COMM_ADDRESS *pKeyAddress;
DWORD totalDelay=0;
UINT    wTimerRes;  // timer resolturion
//UINT    wTimerID;  // timer ID
TIMECAPS  timecaps;  // needed by timeGetDevCaps
uintptr_t threadHandle;
DWORD windowsLastError;

  setbuf (stdout, NULL);    /* do not buffer the output to stdout   */
  setbuf (stderr, NULL);    /* do not buffer the output to stderr   */

 InitializeCriticalSection(&_dblLnkCriticalSection);		    //put in for the testing of a separate parse thread

//do this near top of "main". I don't remember if you need both, but it shouldn't hurt.
/* Set the ^c catcher */
   signal (SIGINT, ctrlCfun);
//   signal (SIGABRT, ctrlCfun);

#define DEFAULT_TRANS_TIME 30

  cfg.transmissionIntervalMsec = DEFAULT_TRANS_TIME;
  startup_cfg_read ("startup.cfg",&cfg);

  if(cfg.logIntervalSeconds!=0)
    StatisticFile = fopen("statistics.log","w+");
 
  strcpy( InterfaceID,cfg.interaceID);


 
  Val8021Q.priority =4;
  Val8021Q.vlanID =4; 

//set the precision of the sleep timer
   if ( timeGetDevCaps( &timecaps, sizeof( TIMECAPS ) ) == TIMERR_NOERROR )
    {
    // get optimal resolution
    wTimerRes = max( timecaps.wPeriodMin, 1);

    // set minimal res for our timer
    if( timeBeginPeriod( 1 ) != TIMERR_NOERROR )
     printf("timeBeginPeriod Error\n");
    else
     printf("Min Timer Resolution: %u\n", wTimerRes);
     }
   else
     printf("timeGetDevCaps Error\n");



//the following section is the normal set of initialization call

  //initialize key storage before the receive socket so that everything is
  //OK should a packet come in once the receive Socket is set up
  if((retVal = SoftwareInitalizationSequence())!=SUCCESS_IEC905)
    {
      printf("Could not initialize software\n");
      printf(" Error: %s\n",IEC905_XlatErrorCode(retVal));
    }

  //before transmitting, need to ope a transmit socket.  In this case, we need to do
  //something a little special in order to allow priorities to be set in Windows and Linux
  //thus a intervening function has been provided



//KDC registration process *************************************************************

  //get the credentials for the primary and secondary KDCs (if there are primary and secondary
  //the credential and KDC pair creation will need to be performed for as many KDCs as need
  //to be communicated with/used.
  
  pKDCCredential1 = usr_create_kdc_credential();
  pKDCCredential2 = usr_create_kdc_credential();

  //this would typically be the spot to fill in the credentials, but have none currently

  //we will use the dummy addressing for the KDC pair
  if((pRetKDCRef = iec905_create_KDC_pair(NULL, &kdcAddress,pKDCCredential1 ,&kdcAddress,pKDCCredential2))==NULL)
    {
    free(pKDCCredential1); 
    free(pKDCCredential2 );
    return(0);
    }
  
// start the parse thread before the receive thread
    _beginthread(parse_thread,0, NULL);

  //spawn the receive thread - no arguments are needed
  threadHandle=_beginthread(sample_rxd_thread,0, (void *) NULL);

#if 1
// can compile in the following section if a higher thread priority is needed.
  if(threadHandle!=-1L)
    {
     retVal= SetThreadPriority((HANDLE)threadHandle,THREAD_PRIORITY_TIME_CRITICAL);
//     retVal= SetThreadPriority((HANDLE)threadHandle,THREAD_PRIORITY_HIGHEST);
    if(!retVal)
      windowsLastError = GetLastError();
    }

#endif

//now can add the subscriptions
    while((IEC_90_5_rx_socket==0) && (DoIt==TRUE))    //need to wait a bit to let the rxd thread initial the receive socket
      Sleep(5000);

    if(IEC_90_5_rx_socket!=0)
      {
      pMyRxCntrl = iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,  IP_V4_ADDRESS_TYPE, cfg.subscriptions[0].subAddress, cfg.subscriptions[0].srcAddress, 
			    cfg.subscriptions[0].dataSetRef, IEC_90_5_rx_socket);
     if(pMyRxCntrl)
      {
      retVal=iec905_kdc_bind(pMyRxCntrl,pRetKDCRef );
      if(retVal==SUCCESS_IEC905)
         retVal = iec905_get_kdc_tx_keys(pMyRxCntrl);
      }
    }


  //spawn a thread to continously transmit a SV packet
  if(strlen(cfg.destIPAddressSMV)!=0)
    {
    if((pKeyAddress= create_address_structure(IP_V4_ADDRESS_TYPE, cfg.destIPAddressSMV))!=NULL)
      {
      mySVThreadInfo.pMyKDCRef = pRetKDCRef;
      mySVThreadInfo.pSrcAddress = pKeyAddress;
      mySVThreadInfo.pDestAddress = pKeyAddress;
      mySVThreadInfo.pCancelThreads = &cancelThreads;
      mySVThreadInfo.pInterfaceID = InterfaceID;
      mySVThreadInfo.Val8021Q= Val8021Q;
      mySVThreadInfo.IP_TOS_DSCP=IP_TOS_DSCP;
      mySVThreadInfo.threadDelay = 0;
      totalDelay = cfg.threadStrtDelay;

     _beginthread(transmit_sequence_sv_thread,0, (void *) &mySVThreadInfo);
     }
    else
      printf("SMV Publication Address Could not be converted\n");
    }
  else
     printf("SMV Publication Address not configured\n");   


  if(strlen(cfg.destIPAddressGOOSE)!=0)
    {
    if((pKeyAddress= create_address_structure(IP_V4_ADDRESS_TYPE, cfg.destIPAddressGOOSE))!=NULL)
      {
      //spawn a thread to continously transmit a GOOSE packet
      myGOOSEThreadInfo.pMyKDCRef = pRetKDCRef;
      myGOOSEThreadInfo.pSrcAddress = pKeyAddress;
      myGOOSEThreadInfo.pDestAddress = pKeyAddress;
      myGOOSEThreadInfo.pCancelThreads = &cancelThreads;
      myGOOSEThreadInfo.pInterfaceID = InterfaceID;
      myGOOSEThreadInfo.Val8021Q= Val8021Q;
      myGOOSEThreadInfo.IP_TOS_DSCP=IP_TOS_DSCP;
      myGOOSEThreadInfo.threadDelay = totalDelay;
      totalDelay += cfg.threadStrtDelay;


      _beginthread(transmit_sequence_goose_thread,0, (void *) &myGOOSEThreadInfo);
      }
    else
      printf("GOOSE Publication Address Could not be converted\n");
    }
  else
     printf("GOOSE Publication Address not configured\n");



  if(totalDelay)
    Sleep(10);

  if(strlen(cfg.destIPAddressTunnell)!=0)
    {
    if((pKeyAddress= create_address_structure(IP_V4_ADDRESS_TYPE, cfg.destIPAddressTunnell))!=NULL)
      {
     //spawn a thread to continously transmit a Tunnelled GOOSE packet
      myTunnelledGOOSEThreadInfo.pMyKDCRef = pRetKDCRef;
      myTunnelledGOOSEThreadInfo.pSrcAddress = pKeyAddress;
      myTunnelledGOOSEThreadInfo.pDestAddress = pKeyAddress;
      myTunnelledGOOSEThreadInfo.pCancelThreads = &cancelThreads;
      myTunnelledGOOSEThreadInfo.pInterfaceID = InterfaceID;
      myTunnelledGOOSEThreadInfo.Val8021Q= Val8021Q;
      myTunnelledGOOSEThreadInfo.IP_TOS_DSCP=IP_TOS_DSCP;
      myTunnelledGOOSEThreadInfo.threadDelay= totalDelay;

      _beginthread(transmit_sequence_tunnelled_goose_thread,0, (void *) &myTunnelledGOOSEThreadInfo);
      }
    else
      printf("Tunnel Publication Address Could not be converted\n");
    }
  else
     printf("Tunnel Publication Address not configured\n");


    while((IEC_90_5_rx_socket==0) && (DoIt==TRUE))    //need to wait a bit to let the rxd thread initial the receive socket
      Sleep(5000);

    if(IEC_90_5_rx_socket!=0)
      {
      for(i=0;i<cfg.numCfgSubscriptions;++i)
	{
        pMyRxCntrl = iec905_igmpv3_group_enroll(cfg.subscriptions[i].usage,  IP_V4_ADDRESS_TYPE, cfg.subscriptions[i].subAddress, cfg.subscriptions[i].srcAddress, 
			    cfg.subscriptions[i].dataSetRef, IEC_90_5_rx_socket);
       if(pMyRxCntrl)
        {
        retVal=iec905_kdc_bind(pMyRxCntrl,pRetKDCRef );
        retVal = iec905_get_kdc_tx_keys(pMyRxCntrl);
        cfg.subscriptions[0].pRxdCntrl= (void *)pMyRxCntrl;
        }
      }
    }

  _beginthread(get_stats_thread,0, (void *) &mySVThreadInfo);


  while(DoIt==TRUE)
    Sleep(5000);

  cancelThreads = TRUE;
  Sleep(1000);

//termination sequence - unsubscribe for multicast


  for(i=0;i<cfg.numCfgSubscriptions;++i)
    iec905_igmpv3_group_destroy((IEC905_MSG_CNTRL *)cfg.subscriptions[i].pRxdCntrl,IEC_90_5_rx_socket);

  TerminateThread((HANDLE)threadHandle,0);

  iec905_close_socket(IEC_90_5_rx_socket);

  if(StatisticFile!=NULL)
    fclose(StatisticFile);

  //free allocated information
  free(pKDCCredential1);
  free(pKDCCredential2);
  free(pRetKDCRef);

  return(0);
  }

/**************************************************************/
/*	    usr_notify_of_key_updated_needed		      */
/*							      */
/*  Function is called to notify the user that the keys	      */
/*  found int the CTNRL_STRUCTURE need to be updated.	      */
/*  The user may need to spawn a thread since COMMS to a      */
/*  KDC may be needed					      */
/*							      */
/*  Input: pCntrl - pointer to the control structure	      */
/*							      */
void usr_notify_of_key_updated_needed(IEC905_MSG_CNTRL *pCntrl)
{
  //may desire to spawn a thread to complete this to renew keys

  iec905_get_kdc_tx_keys(pCntrl);
}
 
/**************************************************************/
/*	    usr_notify_of_error				      */
/* called when the internal 90-5 ErrorHandling routine is     */
/* called.  Users would typically hook this information into  */
/* their normal logging routines.			      */
/**************************************************************/ 
void usr_notify_of_error(int errorCode,char *fileName,unsigned long int lineNumber)
{

  printf("\nError Generated (%s , %lu): %s\n",fileName,lineNumber,IEC905_XlatErrorCode(errorCode) );

}
