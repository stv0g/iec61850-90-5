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
/* MODULE NAME : sv_dec.c						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Sampled Value decoder (9-2 + 90-5 extensions)			    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 02/17/12   HSF	   Revised decoder to properly handle optional	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"



#define MAX_ALLOWED_SV_SIZE 1500						//only allow SVs of  1500 bytes

//********** size of the hdr ************************
//	tag 				60					1
//	length				  02  xx xx				3
//  number of ASDU			80  01 xx				3
//  sequence of ASDU			82  82 xx xx				4
#define SV_HDR_SIZE 11		

//**********size of the common ASDU information *****
#define SIZE_OF_INIT_SEQ 4
//  sequence			30 82 xx xx					4
#define SIZE_OF_MSVID	2
//  msvid					80 01 				2
#define SIZE_OF_DATSET	2
//  datSet					81 01				2
#define COMMON_SMP_OFFSET  8
#define SIZE_OF_SMPCNT 4	      //changed from 5->4 8/11
//  smpcnt					82 02  xx xx 			5
#define SIZE_OF_CONFREV	 6	      //changed from 7->6 8/11
// confrev					83 04  xx xx xx xx		7
#define SIZE_OF_REFTIM 10
//  refrtim		not included due to timestamp				10
#define SIZE_OF_SMPSYNCH 3
// smpsynch					85 01 xx			3
#define SIZE_OF_SMPRATE 0
// smprate					86 03 xx xx xx			0
#define SIZE_OF_ASDU_DATA 4
// asdu_data				87 82 xx xx				4
#define SIZE_OF_SMPMOD 0
// smpMod					88 03 xx xx xx			0
#define SIZE_OF_UTCTS 0
// utcTimeStamp				89 08 xx xx xx ...			0
#define COMMON_ASDU_SIZE	(SIZE_OF_INIT_SEQ +	SIZE_OF_MSVID +	SIZE_OF_DATSET+ SIZE_OF_SMPCNT +  SIZE_OF_CONFREV +SIZE_OF_REFTIM + SIZE_OF_SMPSYNCH +SIZE_OF_SMPRATE+ SIZE_OF_ASDU_DATA +	SIZE_OF_SMPMOD + SIZE_OF_UTCTS)

//to get the total size, need to add the size of the Msvid and datSetRef	


	   
//function takes in a pointer to the length, will return the detectedLength and provide a pointer to the value
static unsigned char *extract_length(unsigned char *pLen, ST_UINT16 *detectedLength)
{
ST_UINT8  len_of_len;
ST_UINT16 returnLength=0;

	if ( *pLen & 0x80)			//then have an extended length
	{
		len_of_len = (*pLen & 0x7f);
		if ((len_of_len >2) && (len_of_len !=0))		//then we have an error, length of the length should never be >2
		{
		  IEC905_ErrorHandler(DECODE_ERROR_TOO_LARGE,__FILE__,__LINE__);
		  *detectedLength = 0;
		  return(NULL);
		}
	      ++pLen;
	      if(len_of_len ==  2)
	      {
		//returnLength = returnLength <<8;
		returnLength = *pLen++<<8;
		returnLength += *pLen++;
	      }
	     else
	      returnLength = *pLen++;
	}
	else
	  returnLength = *pLen++;
	  

	*detectedLength = returnLength;
	return(pLen);
}



SV_DEC_STRUCT * sv_decode (unsigned char * pSVPdu,		//pointer to the SVPDU to be decoded		     
				    ST_UINT16 SVPDULen							//length of the PDU to be decoded
				    )
{
 unsigned char *pInt = pSVPdu;
ST_UINT16 intLen = SVPDULen;
SV_DEC_STRUCT *pMyDecStruct;
unsigned int i;
ASDU_DEC_INFO *pMyCurrentASDUInfo;
ST_UINT16 retLen;
unsigned char *pValue;
ST_UINT8 numberOfASDUs=0;
ST_UINT16 sizeToAllocate;

	

#define SV_PDU_TAG    0x60
	if(*pInt++ != SV_PDU_TAG )
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    return(NULL);
	   }
	
	pValue = extract_length(pInt,&retLen);



#define NUM_ASDU_TAG 0x80
	if(*pValue++ != NUM_ASDU_TAG )
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    return(NULL);
	   }
	
	pValue = extract_length(pValue,&retLen);
	numberOfASDUs = *pValue++;
	if(!numberOfASDUs)
	{
	  IEC905_ErrorHandler(DECODE_ERROR_TOO_LARGE,__FILE__,__LINE__);
	  return(NULL);
	}

//now we have enough to allocate the Decode Tracking Structure
	
	sizeToAllocate = sizeof(SV_DEC_STRUCT) + sizeof(ASDU_DEC_INFO) * (numberOfASDUs);
	if((pMyDecStruct = (SV_DEC_STRUCT *)calloc(1,sizeToAllocate))==NULL)
	  {
	    IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
	    return(NULL);
	   }
	pMyDecStruct->num_ASDUs=numberOfASDUs;

#define SEQUENCE_OF_ASDU_TAG 0xA2
	if(*pValue++ != SEQUENCE_OF_ASDU_TAG )
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    free(pMyDecStruct);
	    return(NULL);
	   }
	pValue = extract_length(pValue,&retLen);

	
	//now we need to iterate and fill in all of the ASDU information
	for (i = 0; i<numberOfASDUs;i++)
	{

	  pMyCurrentASDUInfo = &(pMyDecStruct->asdus[0]);


#define ASDU_SEQ_TAG  0x30
	if(*pValue++ != ASDU_SEQ_TAG )
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    free(pMyDecStruct);
	    return(NULL);
	   }
	pValue = extract_length(pValue,&retLen);

//decode the MSVID - tag is mandatory in 9-2
#define MSVID_TAG 0x80
	if(*pValue++ != MSVID_TAG)
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    free(pMyDecStruct);
	    return(NULL);
	   }
	else
	  {
	  pMyCurrentASDUInfo->msvID.pValue = extract_length(pValue,&pMyCurrentASDUInfo->msvID.len);
	  pValue = pMyCurrentASDUInfo->msvID.pValue + pMyCurrentASDUInfo->msvID.len;
	  //++pValue;
	  }

//deccode the DataSetReference, tag is optional in 9-2.
#define DATSET_TAG 0x81
	if(*pValue != DATSET_TAG)
	  {
	    pMyCurrentASDUInfo->datSetRef.pValue = NULL;
	   }
	  else
           {
	  ++pValue;
           pMyCurrentASDUInfo->datSetRef.pValue= extract_length(pValue,&pMyCurrentASDUInfo->datSetRef.len);
	   pValue = pMyCurrentASDUInfo->datSetRef.pValue + pMyCurrentASDUInfo->datSetRef.len;
	  //++pValue;
	   }

//deccode the SMPCNT, tag is mandatory in 9-2
#define SMPCNT_TAG 0x82
	if(*pValue != SMPCNT_TAG)
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    free(pMyDecStruct);
	    return(NULL);
	   }
	else
	   {
	   ++pValue;
	   pMyCurrentASDUInfo->smpCnt.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->smpCnt.len);
	   pValue = pMyCurrentASDUInfo->smpCnt.pValue + pMyCurrentASDUInfo->smpCnt.len;
	 //  ++pValue;
	   }


//decode ConfRev, tag is mandatory in 9-2
#define CONFREV_TAG 0x83
	if(*pValue++ != CONFREV_TAG)
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    free(pMyDecStruct);
	    return(NULL);
	   }
	 else
	    {
	    pMyCurrentASDUInfo->confRev.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->confRev.len);
	    pValue = pMyCurrentASDUInfo->confRev.pValue + pMyCurrentASDUInfo->confRev.len;
	  //++pValue; 
	    }
 

// decode RefrTim. This is optional in IEC 61850-9-2 ED.2, but should be mandatory for IEC 90-5
#define REFRTM_TAG 0x84     
	if(*pValue != REFRTM_TAG)
	  {
	    pMyCurrentASDUInfo->refrTm.pValue= NULL;
	   }
	else
	  {
	  ++pValue;
	  pMyCurrentASDUInfo->refrTm.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->refrTm.len);
	  pValue = pMyCurrentASDUInfo->refrTm.pValue + pMyCurrentASDUInfo->refrTm.len;
	  //++pValue;
	  }


//now decode the SmpSynch, tag is mandatory
#define SMPSYNCH_TAG 0x85
	if(*pValue != SMPSYNCH_TAG)
	  {
	  pMyCurrentASDUInfo->smpSynch.pValue=NULL;
	   }
	else
	  {
	  ++pValue;
	  pMyCurrentASDUInfo->smpSynch.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->smpSynch.len);
	  pValue = pMyCurrentASDUInfo->smpSynch.pValue + pMyCurrentASDUInfo->smpSynch.len;
	  //++pValue;	//increment for the next one to use
	  }


	  //this code initializes the SMPRate, and the code does not allow it to be changed dynamically.

//decode of SMPRate, is optional 
#define SMPRATE_TAG 0x86
	if(*pValue != SMPRATE_TAG)
	  {
	  pMyCurrentASDUInfo->smpRate.pValue = NULL;
	  }
	else
	  {
	  ++pValue;
	  pMyCurrentASDUInfo->smpRate.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->smpRate.len);
	  pValue = pMyCurrentASDUInfo->smpRate.pValue + pMyCurrentASDUInfo->smpRate.len;
	  //++pValue;
	  }

//decode the size of the samples, tag us nandatory	  
#define SAMPLES_TAG 0x87
	if(*pValue++ != SAMPLES_TAG)
	  {
	    IEC905_ErrorHandler(DECODE_UNKNOWN_TAG,__FILE__,__LINE__);
	    free(pMyDecStruct);
	    return(NULL);
	   }
	else
	  {
	  pMyCurrentASDUInfo->samples.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->samples.len);
	  pValue = pMyCurrentASDUInfo->samples.pValue + pMyCurrentASDUInfo->samples.len;
	  //++pValue;	
	  }

	  

//decode SMPMOD, this is an optional field
#define SMPMOD_TAG 0x88
	if(*pValue != SMPMOD_TAG)
	  {
	  pMyCurrentASDUInfo->smpMod.pValue=NULL;
	  }
	  {
	  ++pValue;
	  pMyCurrentASDUInfo->smpMod.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->smpMod.len);
	  pValue = pMyCurrentASDUInfo->smpMod.pValue + pMyCurrentASDUInfo->smpMod.len;
	 // ++pValue;
	  }

//decode the UTC Timestamp, this is optional
#define UTCTS_TAG 0x89
	if(*pValue != UTCTS_TAG)
	  {
	  pMyCurrentASDUInfo->utcTimeStamp.pValue= NULL;
	  }
	else
	  {
	  ++pValue;
	  pMyCurrentASDUInfo->utcTimeStamp.pValue=  extract_length(pValue,&pMyCurrentASDUInfo->utcTimeStamp.len);
	  pValue = pMyCurrentASDUInfo->utcTimeStamp.pValue + pMyCurrentASDUInfo->utcTimeStamp.len;
	  //++pValue;
	  }


      }	//end of the loop to fill in the ASDUs

    //now update the lengths in case the maximum ASDU are always sent



    return(pMyDecStruct);
    

	//now ready to start initializing the data
}


										