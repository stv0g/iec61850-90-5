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
/* MODULE NAME : iec_90_5_udp_tx.c					    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	UDP Publisher transmit functions.				    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who	   Comments					    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include <winsock2.h>
#include <ws2tcpip.h>	/* for IP_MULTICAST_TTL	*/
#include "ntddndis.h"
#include "traffic.h"
#include "TCGuid.h"
#include "ntddpsch.h"
#include "iec_90_5.h"



//#define IEC_90_5_CTX_STRING ""IEC 61850-90-5";
static char* userContext="IEC 61850-90-5";
static TCI_CLIENT_FUNC_LIST myCompletionFunctions;
static HANDLE myClientRegistrationHndle=NULL;
static ULONG lastTXInitError;
static PTC_IFC_DESCRIPTOR pDescriptor;
static TC_IFC_DESCRIPTOR interfaces[MAX_NUM_TC_INTERFACES_SUPPORTED ];
static ULONG adapterQOSModeDefault=ADAPTER_FLOW_MODE_STANDARD;
static ULONG adapterQOSMode=ADAPTER_FLOW_MODE_DIFFSERV;
static ULONG adapterQOSCnfrm;

/**************************************************************************/
/*	      create_address_structure					  */
/**************************************************************************/
IEC_COMM_ADDRESS *create_address_structure(int typeOfAddress, char *pAddress)
{
IEC_COMM_ADDRESS *pKeyAddress;
unsigned long ConvAddress;
char *MyConvAddress;

  if(typeOfAddress!=IP_V4_ADDRESS_TYPE)
  {
    IEC905_ErrorHandler(UNABLE_TO_CONVERT_IP_ADDRESS,__FILE__,__LINE__);
    return(NULL);
  }

  if((pKeyAddress = (IEC_COMM_ADDRESS *)malloc(sizeof(IEC_COMM_ADDRESS)))!=NULL)
    {
    pKeyAddress->lenOfAddress = 4;
    pKeyAddress->typeOfAddress = IP_V4_ADDRESS_TYPE;
    if((MyConvAddress = malloc(sizeof(unsigned long)))!=NULL)
      {
      ConvAddress = inet_addr(pAddress);
      memcpy(MyConvAddress,&ConvAddress,4);
      pKeyAddress->pAddress = MyConvAddress;
      }
    else
      {
      IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__); 
      free(pKeyAddress);
      pKeyAddress=NULL;
      }
    }
  else
    {
    IEC905_ErrorHandler(MEMORY_ALLOCATION_ERROR,__FILE__,__LINE__);
    }

  return(pKeyAddress);
}

/**************************************************************************/
/*	      destroy_address_structure					  */
/**************************************************************************/
void destroy_address_structure(IEC_COMM_ADDRESS *pAddress)
{
  
  if(pAddress==NULL)
    return;

  if(pAddress->pAddress!=NULL)
    free(pAddress->pAddress);

  free(pAddress);

  return;
}

/************************************************************************/
/*			iec905_tx_pdu					*/
/************************************************************************/
int iec905_tx_pdu (IECSOCKET_TX_INFO *SendingInfo,
	IEC_COMM_ADDRESS *pMyAddressInfo,
	ST_INT IPPort,	
	ST_UCHAR *enc_ptr,
	ST_UINT enc_len,
	IEC905_MSG_CNTRL *pTxStats		//used for statistic tracking purposes
	)
  {
  SOCKADDR_IN          ReceiverAddr;
  int TotalByteSent;
  SOCKET SendingSocket = SendingInfo->SendingSocket;
  ST_CHAR *IPAddr = pMyAddressInfo->pAddress;
 

  // Set up a SOCKADDR_IN structure that will identify who we
  // will send datagrams to. For demonstration purposes, let's
  // assume our receiver's IP address is 127.0.0.1 and waiting
  // for datagrams on port 5150.
  ReceiverAddr.sin_family = AF_INET;
  ReceiverAddr.sin_port = htons(IPPort);

  //DEBUG: just use first configured IP. Should we make sure there's exactly 1 ??
//  ReceiverAddr.sin_addr.s_addr = inet_addr("10.32.33.01");	//multicast addr

  ReceiverAddr.sin_addr.s_addr = *(unsigned long *)IPAddr;;	//multicast addr

  // Send a datagram to the receiver.
  if(TotalByteSent = sendto(SendingSocket, enc_ptr, enc_len, 0,
                            (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr))>0)
    ++pTxStats->stats.TotalTxPktCnt;

  // Some info on the receiver side...
  // Allocate the required resources
#if 0
  memset(&SrcInfo, 0, sizeof(SrcInfo));
  len = sizeof(SrcInfo);
  getsockname(SendingSocket, (SOCKADDR *)&SrcInfo, &len);
#endif

  return 0;
  }


/**************************************************************/
/*		DeleteTCFlow				      */
/* Removes a Windows Traffic Control Flow definition	      */
/**************************************************************/
static int DeleteTCFlow(HANDLE  FlowHandle)
{
  //the delete of the Flow will fail if there are any TCFilters active with it
  //therefore, must delete all Filters prior to the deletion of the Flow
  ULONG retVal;

  if(retVal =  TcDeleteFlow(FlowHandle))
    return (IEC905_ErrorHandler (UNABLE_TO_DELETE_TC_FLOW,__FILE__, __LINE__));
  else
    return(SUCCESS_IEC905);
}


/************************************************************************************************/
/*   Function:  AddFlow									*/
/*												*/
/*   Purpose: To create the structures needed to create a TCFlowOjbect that has a		*/
/*	      specific IP Priority (DSCPValue) and Ethernet Priority (Tci_info).		*/
/*												*/
/*   Returns:  an integer error/success value and fills in the FlowHandle pointer. The		*/
/*	      pointer will be NULL if there was an error.					*/
/*												*/
/*												*/
/************************************************************************************************/
static int AddFlow(HANDLE *pFlowHandle, HANDLE InterfaceHandle, HANDLE FlowCtxHandle, USHORT *DSCPValue, ETHERTYPE_8021Q *pTci_info )
    {


      char * pCurrentObject;
      QOS_OBJECT_HDR *pObjListEnd=NULL;
      QOS_DS_CLASS *pDSClassObject=NULL;
      QOS_TRAFFIC_CLASS *pTClassObject=NULL;
      TC_GEN_FLOW *pNewFlowObj = NULL;
      ULONG retVal;
      int Length = 0;
      int Len1=0;


      USHORT tci_info=0;
      USHORT tci_temp;

      if(pTci_info!=NULL)     //create the Etherype information for 802.1Q
      {
	tci_temp = ((pTci_info->priority) & 0x7)<< 13;	//only 3 bits allowed, regardless of what was passed in.
	tci_info = tci_temp;
	tci_info += (pTci_info->vlanID) & 0x0fff;
      } 

      //
      // Calculate the memory size required for the optional TC objects
      //
#if 0
  //stubbed out since setting this value does not allow a packet to be transmitted, needs further investigation
       if(pTci_info!=NULL)
        Length += sizeof(QOS_TRAFFIC_CLASS);
#endif

      if(DSCPValue!=NULL)
       Length += sizeof(QOS_DS_CLASS);

      //
      // Allocate the flow descriptor
      //
//      Len1 = sizeof(TC_GEN_FLOW) + sizeof(QOS_OBJECT_HDR) + Length;
      Len1 = sizeof(TC_GEN_FLOW) +  Length - sizeof(QOS_OBJECT_HDR);
      pNewFlowObj = (TC_GEN_FLOW *)malloc(Len1);

      if (!pNewFlowObj)
      {
	IEC905_ErrorHandler (WINDOWS_FLOW_OBJECT_ALLOCATON_ERROR,__FILE__, __LINE__);
        return(WINDOWS_FLOW_OBJECT_ALLOCATON_ERROR);
      }

      pNewFlowObj->ReceivingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;
      pNewFlowObj->ReceivingFlowspec.Latency = QOS_NOT_SPECIFIED;
      pNewFlowObj->ReceivingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;
      pNewFlowObj->ReceivingFlowspec.MinimumPolicedSize=  QOS_NOT_SPECIFIED;
      pNewFlowObj->ReceivingFlowspec.PeakBandwidth=  QOS_NOT_SPECIFIED;  
      pNewFlowObj->ReceivingFlowspec.ServiceType = SERVICETYPE_BESTEFFORT;
      pNewFlowObj->ReceivingFlowspec.TokenBucketSize = QOS_NOT_SPECIFIED;
      pNewFlowObj->ReceivingFlowspec.TokenRate=QOS_NOT_SPECIFIED;
 

      pNewFlowObj->SendingFlowspec.DelayVariation = QOS_NOT_SPECIFIED;
      pNewFlowObj->SendingFlowspec.Latency = QOS_NOT_SPECIFIED;
      pNewFlowObj->SendingFlowspec.MaxSduSize = QOS_NOT_SPECIFIED;
      pNewFlowObj->SendingFlowspec.MinimumPolicedSize=  QOS_NOT_SPECIFIED;
      pNewFlowObj->SendingFlowspec.PeakBandwidth=  QOS_NOT_SPECIFIED;  
      pNewFlowObj->SendingFlowspec.ServiceType = SERVICETYPE_BESTEFFORT;
      pNewFlowObj->SendingFlowspec.TokenBucketSize = QOS_NOT_SPECIFIED;
      pNewFlowObj->SendingFlowspec.TokenRate=QOS_NOT_SPECIFIED;


       //pNewFlowObj->TcObjectsLength = Length+sizeof(QOS_OBJECT_HDR);
      pNewFlowObj->TcObjectsLength = Length;

      //
      // Add any requested objects
      //


      pCurrentObject = (char *)&pNewFlowObj->TcObjects[0];
#if 0
  //stubbed out since setting this value does not allow a packet to be transmitted, needs further investigation
      if(pTci_info != NULL)
      {
        pTClassObject = (QOS_TRAFFIC_CLASS*)pCurrentObject;
        pTClassObject->ObjectHdr.ObjectType = QOS_OBJECT_TRAFFIC_CLASS;
        pTClassObject->ObjectHdr.ObjectLength = sizeof(QOS_TRAFFIC_CLASS);
        pTClassObject->TrafficClass = pTci_info->priority; //802.1p tag to be used  ---need to come back to this to figure out what the real setting should be.
	pTClassObject->TrafficClass=SERVICETYPE_BESTEFFORT;
        pCurrentObject += sizeof(QOS_TRAFFIC_CLASS);
      }
#endif

      if(DSCPValue != NULL)
      {
        pDSClassObject = (QOS_DS_CLASS*)pCurrentObject;
        pDSClassObject->ObjectHdr.ObjectType = QOS_OBJECT_DS_CLASS;
        pDSClassObject->ObjectHdr.ObjectLength = sizeof(QOS_DS_CLASS);
        pDSClassObject->DSField = *DSCPValue; //Services Type
	//now need to point to the next object that was allocated
	pCurrentObject += sizeof(QOS_DS_CLASS);
      }



      //pObjListEnd = (QOS_OBJECT_HDR *)pCurrentObject;
      //pObjListEnd->ObjectType = QOS_OBJECT_END_OF_LIST;
      //pObjListEnd->ObjectLength = sizeof(QOS_OBJECT_HDR);

    if(retVal = TcAddFlow(InterfaceHandle,FlowCtxHandle,0x0L,pNewFlowObj, pFlowHandle))
      {
      free(pNewFlowObj);
      return(IEC905_ErrorHandler (TC_FLOWADD_FAILED,__FILE__, __LINE__));
      }

    free(pNewFlowObj);
    return (SUCCESS_IEC905);
    }

/**************************************************************/
/*		AddFlowFiltertoFlow			      */
/* As a Windows Traffic control filter to a particular Flow   */
/**************************************************************/
static int AddFlowFiltertoFlow(IEC_COMM_ADDRESS *pDestAddress, HANDLE flowHandle,  HANDLE *pFilterHandle)
{
  TC_GEN_FILTER GenericFilter;
  IP_PATTERN defaultPattern, defaultMask;
  ULONG retVal;


  memset(&defaultPattern,0x0,sizeof(IP_PATTERN ));
  memset(&defaultMask,0x0,sizeof(IP_PATTERN ));
  //filter uses specified destination address and any local address

//  defaultPattern.DstAddr = inet_addr("10.32.33.01");
 // defaultPattern.DstAddr = inet_addr(pDestAddress->pAddress);	    //were not able to convert the passed in destination address
   defaultPattern.DstAddr = *(unsigned long * )pDestAddress->pAddress; 

  defaultPattern.SrcAddr = inet_addr("0.0.0.0");		 //specifies local address

  //filter specifies use of UDP IP
  defaultPattern.ProtocolId = IPPROTO_UDP;

  //will allow any socket locally going to a destination socket of 102 (assinged by IEC-90-5)
  defaultPattern.S_un.S_un_ports.s_srcport = 0;
  defaultPattern.S_un.S_un_ports.s_dstport = htons(IEC_61850_90_5_UDP_RX_PORT);

  //set up the default MASK
  
  defaultMask.DstAddr = 0xFFFFFFFF;
  defaultMask.S_un.S_un_ports.s_dstport=0xFFFF;
  defaultMask.ProtocolId = 0xFF;

  //now that we have the mask and the pattern, time to add the information to the Filter
   GenericFilter.AddressType = NDIS_PROTOCOL_ID_TCP_IP;
   GenericFilter.Mask = &defaultMask;
   GenericFilter.Pattern = &defaultPattern;
   GenericFilter.PatternSize = sizeof(IP_PATTERN);

  if(retVal = TcAddFilter(flowHandle,&GenericFilter,pFilterHandle))
    return(IEC905_ErrorHandler (UNABLE_TO_ADD_TCFILTER,__FILE__, __LINE__));
  else
    return(SUCCESS_IEC905);
  
}

//the following functions are required to satisfy the Windows
//traffic control API
static VOID notifyTCEventHndlr(
    IN	HANDLE		ClRegCtx,
    IN	HANDLE		ClIfcCtx,
	IN	ULONG		Event, 		// See list below
	IN	HANDLE	    SubCode,
	IN	ULONG		BufSize,
	IN	PVOID		Buffer
	)
{
}

static VOID addTCCompletHndlr(IN HANDLE ClFlowCtx, IN ULONG Status )
{
}

static VOID  modTCCompleteHndlr(IN HANDLE ClFlowCtx,IN ULONG Status)
{
}

static VOID delTCCompleteHndlr(IN HANDLE ClFlowCtx,IN ULONG Status)
{
}


/**************************************************************/
/*		iec905_tx_pdu_init			      */
/*Intializes internal structures required to tx packets       */
/* for windows, it also initializes the Traffic API	      */
/**************************************************************/
int iec905_tx_pdu_init()
{
int numBufferBytes = sizeof(interfaces);


if (myClientRegistrationHndle!=NULL)	  //then already initialized, just return SUCCESS
  return(SUCCESS_IEC905);

//need to initialize the Traffic Control API - MUST HAVE ADMIN PRIVILEGES
  myCompletionFunctions.ClNotifyHandler = (TCI_NOTIFY_HANDLER )& notifyTCEventHndlr;
  myCompletionFunctions.ClAddFlowCompleteHandler = NULL;
  myCompletionFunctions.ClModifyFlowCompleteHandler = NULL;
  myCompletionFunctions.ClDeleteFlowCompleteHandler = NULL;

  if((lastTXInitError = TcRegisterClient( CURRENT_TCI_VERSION,(HANDLE) &userContext,&myCompletionFunctions,&myClientRegistrationHndle))!=NO_ERROR)
  {
    if(lastTXInitError ==ERROR_OPEN_FAILED)
      {
	IEC905_ErrorHandler (TC_OPEN_FAILED_CHK_ADMIN_PRIV,__FILE__, __LINE__);
        return(TC_OPEN_FAILED_CHK_ADMIN_PRIV);
      }
    IEC905_ErrorHandler (TC_REGISTRATION_ERROR,__FILE__, __LINE__);
    return(TC_REGISTRATION_ERROR);
  }

//now need to get the set of intrfaces that support the Traffic Control API
if((lastTXInitError = TcEnumerateInterfaces(myClientRegistrationHndle, &numBufferBytes, (PTC_IFC_DESCRIPTOR) interfaces))!=NO_ERROR)
  {
    IEC905_ErrorHandler (TC_INTERFACE_LISTING_FALIED,__FILE__, __LINE__);
    TcDeregisterClient(myClientRegistrationHndle);	//deRegister if it failed.
    myClientRegistrationHndle = NULL;
    return(TC_INTERFACE_LISTING_FALIED);
  }

pDescriptor = (PTC_IFC_DESCRIPTOR )interfaces;	      //save for future use

return(SUCCESS_IEC905);
}

/**************************************************************/
/*		convert_LPWSTR_to_char			      */
/* used to convert a unicode interface GUID to prinatble      */
/* characters						      */
/**************************************************************/
static convert_LPWSTR_to_char(char *dest,LPWSTR src, int sizeOfdest)
{
int i=0;
int j=0;
char *src_ptr= (char *)src;

  memset(dest,0x0,sizeOfdest);
  while((src_ptr[i]!=0x0 || src_ptr[i+1]!=0x0) && (j<sizeOfdest))
    {
      dest[j] = src_ptr[i];
      i+=2;
      ++j;
    }
  
}

/**************************************************************/
/*		find_InterfaceName 			      */
/* Finds an ethernet interface name in Windows		      */
/**************************************************************/
static LPWSTR find_InterfaceName (char *pInterfaceID)
{
int found=FALSE;
char myDescriptorString[100];
PTC_IFC_DESCRIPTOR  pmyDescripPtr= pDescriptor;

  while((pDescriptor->Length!=0) && (found==FALSE))
    {
      convert_LPWSTR_to_char(myDescriptorString,pDescriptor->pInterfaceID, 100);
      if (!strcmp(myDescriptorString,pInterfaceID))
	found=TRUE; 
      else
	pmyDescripPtr += sizeof(TC_IFC_DESCRIPTOR);
    }

  if(found==TRUE)
    return(pDescriptor->pInterfaceName);
  else
    return(NULL);
}

/**************************************************************/
/*		iec905_tx_pdu_open_socket_with_priorities			      */
/* Finds an ethernet interface name in Windows		      */
/**************************************************************/
int iec905_tx_pdu_open_socket_with_priorities(IECSOCKET_TX_INFO **pUserSocketInfo, char *pInterfaceID, ETHERTYPE_8021Q *p8021Q, USHORT *pIP_TOS_DSCP, IEC_COMM_ADDRESS *pDestAddress)
{
ULONG retVal;
HANDLE FlowHandle=NULL;
HANDLE IfcHandle=NULL;
HANDLE FilterHandle;
HANDLE CIFlowCtx=NULL;
PTC_GEN_FLOW  _ppTcFlowObj=NULL;
LPWSTR pInterfaceName=NULL;
GUID qosGuid = GUID_QOS_FLOW_MODE;
#define UDP_MULTICAST_TTL 128
DWORD ttl=UDP_MULTICAST_TTL;
DWORD cksum= 1;			  //enabling UDP Checksum
IECSOCKET_TX_INFO *pSocketInfo;

  *pUserSocketInfo = NULL;

#ifdef _WIN32
//see if an interface can be openned
  if((pInterfaceName = find_InterfaceName(pInterfaceID))==NULL)
  {
    //the specified interface could not be found
    IEC905_ErrorHandler (ETHERNET_INTERFACE_NOT_FOUND,__FILE__, __LINE__);
    return(ETHERNET_INTERFACE_NOT_FOUND);
  }

  if((retVal =  TcOpenInterface(pInterfaceName, myClientRegistrationHndle,userContext, &IfcHandle))!=NO_ERROR)
  {
    //could not open interface that was found
    IEC905_ErrorHandler (ETHERNET_INTERFACE_OPEN_FAILED,__FILE__, __LINE__);
    return(ETHERNET_INTERFACE_OPEN_FAILED);
  }

// ---- will need to check if the flow already exists   before doing all this other stuff
// because Microsoft is not smart enough to know that it is the same flow

  //have to set the Interface to be Differentiated  QOS Service
  if(retVal = TcSetInterface(IfcHandle,&qosGuid,sizeof(ULONG),&adapterQOSModeDefault))
  {
    IEC905_ErrorHandler (INTERFACE_DIFFSERV_FAILED,__FILE__, __LINE__);
    TcCloseInterface(IfcHandle);		//close the interface that was successfully openned.
    return(INTERFACE_DIFFSERV_FAILED);
  }

//  retVal = TcAddFlow( IfcHandle,userContext,0x0,_ppTcFlowObj,&pFlowHandle);
  if(retVal = AddFlow(&FlowHandle, IfcHandle, CIFlowCtx,  pIP_TOS_DSCP, p8021Q  ))
  {
    TcCloseInterface(IfcHandle);		//close the interface that was successfully openned.
    return(retVal);
  }

//end of the other stuff

//now that we have a flow, time to add a Filter to it
   retVal = AddFlowFiltertoFlow(pDestAddress, FlowHandle,  &FilterHandle);

   TcCloseInterface(IfcHandle);
#endif

  //now need to open the sending socket and allocate the return structure
  if((pSocketInfo = (IECSOCKET_TX_INFO *)calloc(1,sizeof(IECSOCKET_TX_INFO)))!=NULL)
  {
    pSocketInfo->FilterHandle=FilterHandle;
    pSocketInfo->FlowHandle = FlowHandle;
    if((pSocketInfo->SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==INVALID_SOCKET)
      {
      free(pSocketInfo);		    //free the allocated structure
      pSocketInfo= NULL;

#ifdef _WIN32
      TcDeleteFilter(FilterHandle);   //delete the filter, leave the flow
#endif

      return(IEC905_ErrorHandler (UNABLE_TO_CREATE_SENDING_SOCKET,__FILE__, __LINE__));
      }
     else
      {
    //set the TTL
      if(setsockopt(pSocketInfo->SendingSocket,IPPROTO_IP,IP_MULTICAST_TTL,(char *)&ttl,sizeof(ttl)))
        {
      //this error is important as it will not allow the packets to route if it fails
	closesocket(pSocketInfo->SendingSocket);
	(char *)(pSocketInfo->SendingSocket)=NULL;    //to satisfy Klockwork	
        free(pSocketInfo);		    //free the allocated structure
        pSocketInfo= NULL;

#ifdef _WIN32
        TcDeleteFilter(FilterHandle);   //delete the filter, leave the flow
#endif

        return(IEC905_ErrorHandler (UNABLE_TO_CREATE_SENDING_SOCKET,__FILE__, __LINE__));
        }
    //set the UDP_XSUM option, don't care if this succeeds or fails since the XSUM is an option
      setsockopt(pSocketInfo->SendingSocket,IPPROTO_UDP,UDP_CHECKSUM_COVERAGE,(char *)&cksum,sizeof(cksum));
      *pUserSocketInfo = pSocketInfo;
      }
     }

  return(SUCCESS_IEC905);
}


/**************************************************************/
/*		iec905_tx_pdu_close_socket		      */
/*							      */
/*  Function closes a sending socket and destroys any	      */
/*  supporting informaiton (Flows and Filters for Windows)    */
/*  that were created					      */
/*							      */
/*  Retuns: Success or Error code			      */
/**************************************************************/
int iec905_tx_pdu_close_socket (IECSOCKET_TX_INFO *SendingInfo)
{

  if(SendingInfo==NULL)
    return(SUCCESS_IEC905);

#ifdef _WIN32
  TcDeleteFilter(SendingInfo->FilterHandle);
  TcDeleteFlow(SendingInfo->FlowHandle);
#endif

  closesocket(SendingInfo->SendingSocket);
  free(SendingInfo);

  return(SUCCESS_IEC905);

}