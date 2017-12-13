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
/* MODULE NAME : iec_90_5_ErrorHandler.c				    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Provides common error handling functions to allow users		    */
/*	to hook into their own logging subsystem			    */
/*									    */
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				    */
/*	IEC905_ErrorHandler()						    */
/*	IEC905_XlatErrorCode()						    */
/*	IEC905_GetSpecificEntry()					    */
/*	IEC905_GetLastError()						    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/


#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"



typedef struct error_Xlat
{
  int ErrorCode;
  char *XlatString;
} ERROR_XLAT;

static ERROR_XLAT translationTable[] = {
   { SUCCESS_IEC905,"Succes or OK result"},
   { FAILURE_IEC905,"Failure of NOK"},
   { UNABLE_TO_INTIALIZE_IP_INTERFACE, "Initialization of IP Failed"},
   { IP_PORT_INCORRECT_IEC905, "A IPPort of zero(0) was specified, must be a non-zero value"},
   { UNABLE_TO_OPEN_UDP_SOCKET_IEC905, "Unable to open the specified UDP Socket"},   	
   { UDP_RX_BIND_FAILED_IEC905, "Unable to bind to the IEC 90-5 receive socket (102 is the default)"},
   { ETHERNET_INTERFACE_NOT_FOUND , "The specified Ethernet Interface is not found/installed"},
   { ETHERNET_INTERFACE_OPEN_FAILED	, "The Etherenet interface exists, but could not be opened. Check priviledges"},
   { UNABLE_TO_CONVERT_IP_ADDRESS , "The specified IP Address could not be used.  It must be a V4 dotted decimal address currently."},
   { INVALID_IGMP_GROUP_ENROLL_PARAMETERS_IEC905 ," A non-supported IP Address or NULL parameters were passed"},
   { DUPLICATE_PAYLOAD_DETECTED_IEC905	 , "Security Payload already exists"},
   { IGMPV3_GROUP_MEMBERSHIP_ENROLL_FAILED_IEC905, "IGMPv3 subscription failed - check source address and/or that subscription address is a multicast address"},
   { IGMPV2_GROUP_MEMBERSHIP_ENROLL_FAILED_IEC905, " IGMPv3 subscription failed - check that the subscription address is a multicast address"},
   { INVALID_IGMP_GROUP_UNSUB_PARAMETER_IEC905, "A invalid parameter was detected in the IGMP unsubscribe function"},
   { INVALID_IGMP_GROUP_UNSUBV2_FAILURE_IEC905, "The un-subscribe for IGMPv2 failed"},
   { INVALID_IGMP_GROUP_UNSUBV3_FAILURE_IEC905, "The un-subscribe for IGMPv3 failed"},
   { TC_REGISTRATION_ERROR, "(Windows Specific): Could not register with the Traffic Control API.  It is not available on WindowsXP. Also, make sure that the QOS packet scheduler is installed and enabled for Windows"},
   { TC_OPEN_FAILED_CHK_ADMIN_PRIV , "(Windows Specific): Could not open Traffic Control API. API is not supported in WindowsXP or earlier.  Check that the application has administrative priviledges"},
   { TC_INTERFACE_LISTING_FALIED , "(Windows Specific): Could not obtain list of interfaces that support Traffic Control"},	              
   { INTERFACE_DIFFSERV_FAILED , "(Windows Specific): Could not use interface with differentiate services"},
   { TC_FLOWADD_FAILED , "(Windows Specific): Could not create the requested traffic control flow object"},
   { UNABLE_TO_ADD_TCFILTER	, "Could not create the requested transmit QOS"},
   { UNABLE_TO_DELETE_TC_FLOW	, "(Windows Specific): Could not delete the requested traffic flow object, probably in use by another QOS Filter"},
   { CRYPTO_API_ERROR_OFFSET, "Could not find the appropriate Crypto API"},
   { CRYPTO_PROVIDER_OPEN_ERROR, "Could not open the crypto API"},
   { CRYPTO_RND_NUM_GEN_ERROR, "Could not generate a cyrptographic random number"},
   { GENERAL_CRYPTO_PROBLEM, "Problem with crypto API"},
   { CRYPTO_BLOCK_LEN_PROBLEM, "Problem with crypto block lengths"},
   { LARGER_THAN_ALLOWED_PDU, "PDU Size exceeds preset allowed Maximum size"},
   { MEMORY_ALLOCATION_ERROR, "Attempt to allocate memory failed"},
   { DECODE_ERROR_TOO_LARGE, "A parameter was encountered during the decode that was too large"},
   { DECODE_UNKNOWN_TAG, "An unknown ASN.1 was encountered, or was out of sequence"},
   { INVALID_INPUT_PARAMETER, "An invalid function parameter was detected"},
   { UNABLE_TO_ADD_KEY, "A key was not able to be added or obtained"},
   { DECODE_ERROR_IN_PAYLOADS,"Problem detected in decoding Payloads"} ,
   { IEC_90_5_NO_CLTP  , "No CLTP protocol detected"},
   { IEC_90_5_TAMPERDECTECT_HDR, "PDU Tampering detected during decode"},
   { IEC_90_5_INVALID_HDR, "Invalid header detected during decode"},
   { IEC_90_5_INVALID_HDR_LI , "Invalid header length detected, possible tamper"},
   { IEC_90_5_TAMPERDECTECT_NO_SIG, "Tamper detected due to lack of expected signature"},
   { IEC_90_5UNEXPECTED_PDU_RXD, "Security structure not found for incoming PDU, PDU discarded"}, 
   { KEYS_NOK, "Neither the current nor next key information is set or correct"},
   { PRIME_KEY_OK_NEXT_KEY_NOK, "Current key is OK, Next key needs to be updated"},
   { IEC_90_5_DUPLICATE_PACKET_RXD, " A previously received packet was received again. Packet was not processed"}
};



static ERROR_TRACKING LastErrors[MAX_ALLOWED_LAST_ERRORS+1];
static int num_XlatTblEntries = sizeof(translationTable)/sizeof(ERROR_XLAT);
static char *unknown_error="Unknown Error Code";

/********************************************************************************/
/*		    IEC905_ErrorHandler						*/
/*										*/
/*  Function is called if an error is detected in the IEC 90-5 Code		*/
/*										*/
/*  Inputs:  errorCode - one of the defined IEC90-5 error codes	(in IEC_90_5.h)	*/
/*	      filename - name of the file that generated the call		*/
/*	      lineNumber - line number in the file that generated the error	*/
/*										*/
/*  Output:  The passed in errorCode						*/
/*										*/
/********************************************************************************/
int IEC905_ErrorHandler (int errorCode, char * fileName, unsigned long int lineNumber)
{
int i;

//move the array of information
  for(i=0; i<MAX_ALLOWED_LAST_ERRORS;++i)
	{
		memcpy(&LastErrors[i+1], &LastErrors[i], sizeof(ERROR_TRACKING));
	}
  LastErrors[0].errorCode = errorCode;
  LastErrors[0].fileName = fileName;
  LastErrors[0].lineNumber = lineNumber;
  LastErrors[0].timeOfError = time(NULL);
  LastErrors[0].inUse = TRUE;

  //$$$place the integration into the normal application logging function here
  usr_notify_of_error(errorCode,fileName,lineNumber);

  return(errorCode);
}



  
/********************************************************************************/
/*		    IEC905_XlatErrorCode					*/
/*										*/
/*  Function is called to retrieve the the textual representation of		*/
/*  an error code								*
/*										*/
/*  Inputs:  Error code for lookup						*/
/*										*/
/*  Output:  Pointer to a string.  Either the error codes meaning or 		*/
/*	      "Unknown Error Code"  when the code is not found			*/
/*										*/
/********************************************************************************/
char *IEC905_XlatErrorCode(int errorCode)
{
int i;
char * retVal =unknown_error;
  for(i=0;i<num_XlatTblEntries;i++)
  {
    if(translationTable[i].ErrorCode == errorCode)
     {
      retVal = translationTable[i].XlatString;
      break;
     }
  }
  return(retVal);
}


/********************************************************************************/
/*		   IEC905_GetSpecificEntry					*/
/*										*/
/*  Function is called to retrieve the last error that was passed into		*/
/*  IEC905_ErrorHandler	()							*/
/*										*/
/*  Inputs:  unsigned integer EntryNumber	  				*/
/*										*/
/*  Output:  Pointer to the last tracking structure (ERROR_TRACKING *)		*/
/*	    which contains errorCode, timestamp of error, filename, and		*/
/*	    linenumber.								*/
/*										*/
/*  Note to application developers:						*/
/*    The filename and linenumber entries may be null.				*/
/*    If the function is called prior to any errors, the timestamp will be	*/
/*    a value of zero()								*/
/********************************************************************************/
ERROR_TRACKING *IEC905_GetSpecificEntry(unsigned int entryNumber)
{
  if(entryNumber >= MAX_ALLOWED_LAST_ERRORS)	//make sure that the entryNumber is in range
    return(NULL);
  else
    {
    if(LastErrors[entryNumber].inUse ==TRUE)
      return(&LastErrors[entryNumber]);
    else
      return(NULL);
    }
}   

/********************************************************************************/
/*		    IEC905_GetLastError						*/
/*										*/
/*  Function is called to retrieve the last error that was passed into		*/
/*  IEC905_ErrorHandler	()							*/
/*										*/
/*  Inputs:  None								*/
/*										*/
/*  Output:  Pointer to the last tracking structure (ERROR_TRACKING *)		*/
/*	    which contains errorCode, timestamp of error, filename, and		*/
/*	    linenumber.								*/
/*										*/
/*  Note to application developers:						*/
/*    The filename and linenumber entries may be null.				*/
/*    If the function is called prior to any errors, the timestamp will be	*/
/*    a value of zero()								*/
/********************************************************************************/
ERROR_TRACKING *IEC905_GetLastError()
{ 
  return(IEC905_GetSpecificEntry(0));
}