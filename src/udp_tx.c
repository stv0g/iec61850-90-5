/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : udp_tx.c						*/
/* PRODUCT(S)  : MMS-EASE Lite						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	UDP Publisher transmit functions.				*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*				udp_tx					*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who	   Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 07/22/11  JRB	   Initial Revision				*/
/************************************************************************/
#include "glbtypes.h"
#include "sysincs.h"
#include <winsock2.h>
#include <ws2tcpip.h>	/* for IP_MULTICAST_TTL	*/
#include "udp.h"
#include "ntddndis.h"
#include "traffic.h"
#include "TCGuid.h"
#include "ntddpsch.h"


SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;


/************************************************************************/
/*			udp_tx						*/
/************************************************************************/
int iec90_5_tx (IECSOCKET_TX_INFO *SendingInfo,
	ST_CHAR *IPAddr,
	ST_INT IPPort,	//DEBUG: right type?
	ST_UCHAR *enc_ptr,
	ST_UINT enc_len)
  {
  SOCKADDR_IN          ReceiverAddr, SrcInfo;
  int len;
  int TotalByteSent;
  SOCKET SendingSocket = SendingInfo->SendingSocket;

  // Set up a SOCKADDR_IN structure that will identify who we
  // will send datagrams to. For demonstration purposes, let's
  // assume our receiver's IP address is 127.0.0.1 and waiting
  // for datagrams on port 5150.
  ReceiverAddr.sin_family = AF_INET;
  ReceiverAddr.sin_port = htons(IPPort);

  //DEBUG: just use first configured IP. Should we make sure there's exactly 1 ??
  ReceiverAddr.sin_addr.s_addr = IPAddr;	//multicast addr

  // Send a datagram to the receiver.
  TotalByteSent = sendto(SendingSocket, enc_ptr, enc_len, 0,
                            (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));

  // Some info on the receiver side...
  // Allocate the required resources
  memset(&SrcInfo, 0, sizeof(SrcInfo));
  len = sizeof(SrcInfo);
  getsockname(SendingSocket, (SOCKADDR *)&SrcInfo, &len);
#if 0	//DEBUG: delete these printfs or change to logs
  printf("Client: Sending IP(s) used: %s\n", inet_ntoa(SrcInfo.sin_addr));

  printf("Client: Sending port used: %d\n", htons(SrcInfo.sin_port));

  // Some info on the sender side
  getpeername(SendingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));

  printf("Client: Receiving IP used: %s\n", inet_ntoa(ReceiverAddr.sin_addr));
  printf("Client: Receiving port used: %d\n", htons(ReceiverAddr.sin_port));
  printf("Client: Total byte sent: %d\n", TotalByteSent);
 
  // When your application is finished receiving datagrams close the socket.
  printf("Client: Finished sending. Closing the sending socket...\n");
#endif

  return 0;
  }



static int DeleteTCFlow(HANDLE  FlowHandle)
{
  //the delete of the Flow will fail if there are any TCFilters active with it
  //therefore, must delete all Filters prior to the deletion of the Flow
  ULONG retVal;

  if(retVal =  TcDeleteFlow(FlowHandle))
    return (IEC905_ErrorHandler (UNABLE_TO_DELETE_TC_FLOW,thisFileName, __LINE__));
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
      int status,i;

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

       if(pTci_info!=NULL)
        Length += sizeof(QOS_TRAFFIC_CLASS);

      if(DSCPValue!=NULL)
       Length += sizeof(QOS_DS_CLASS);

      //
      // Allocate the flow descriptor
      //
      Len1 = sizeof(TC_GEN_FLOW) + sizeof(QOS_OBJECT_HDR) + Length;
      pNewFlowObj = (TC_GEN_FLOW *)malloc(Len1);

      if (!pNewFlowObj)
      {
	IEC905_ErrorHandler (WINDOWS_FLOW_OBJECT_ALLOCATON_ERROR,thisFileName, __LINE__);
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


      pNewFlowObj->TcObjectsLength = Length+sizeof(QOS_OBJECT_HDR);

      //
      // Add any requested objects
      //


      pCurrentObject = (char *)&pNewFlowObj->TcObjects[0];
      if(DSCPValue != NULL)
      {
        pDSClassObject = (QOS_DS_CLASS*)pCurrentObject;
        pDSClassObject->ObjectHdr.ObjectType = QOS_OBJECT_DS_CLASS;
        pDSClassObject->ObjectHdr.ObjectLength = sizeof(QOS_DS_CLASS);
        pDSClassObject->DSField = *DSCPValue; //Services Type
	//now need to point to the next object that was allocated
	pCurrentObject += sizeof(QOS_DS_CLASS);
      }

      if(pTci_info != NULL)
      {
        pTClassObject = (QOS_TRAFFIC_CLASS*)pCurrentObject;
        pTClassObject->ObjectHdr.ObjectType = QOS_OBJECT_TRAFFIC_CLASS;
        pTClassObject->ObjectHdr.ObjectLength = sizeof(QOS_TRAFFIC_CLASS);
        pTClassObject->TrafficClass = pTci_info->priority; //802.1p tag to be used  ---need to come back to this to figure out what the real setting should be.
	pTClassObject->TrafficClass=SERVICETYPE_BESTEFFORT;
        pCurrentObject += sizeof(QOS_TRAFFIC_CLASS);
      }

      pObjListEnd = (QOS_OBJECT_HDR *)pCurrentObject;
      pObjListEnd->ObjectType = QOS_OBJECT_END_OF_LIST;
      pObjListEnd->ObjectLength = sizeof(QOS_OBJECT_HDR);

    if(retVal = TcAddFlow(InterfaceHandle,FlowCtxHandle,0x0L,pNewFlowObj, pFlowHandle))
      {
      free(pNewFlowObj);
      return(IEC905_ErrorHandler (TC_FLOWADD_FAILED,thisFileName, __LINE__));
      }

 //   free(pNewFlowObj);
    return (SUCCESS_IEC905);
    }

static int AddFlowFiltertoFlow(IEC_KEY_ADDRESS *pDestAddress, HANDLE flowHandle,  HANDLE *pFilterHandle)
{
  TC_GEN_FILTER GenericFilter;
  IP_PATTERN defaultPattern, defaultMask;
  ULONG retVal;


  memset(&defaultPattern,0x0,sizeof(IP_PATTERN ));
  memset(&defaultMask,0x0,sizeof(IP_PATTERN ));
  //filter uses specified destination address and any local address

//  if((defaultPattern.DstAddr = inet_addr(pDestAddress->pAddress))==NULL)	    //were not able to convert the passed in destination address
  if((defaultPattern.DstAddr = inet_addr("10.32.33.01"))==NULL)
    {
	IEC905_ErrorHandler (UNABLE_TO_CONVERT_IP_ADDRESS,thisFileName, __LINE__);
        return(UNABLE_TO_CONVERT_IP_ADDRESS);
    }


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
    return(IEC905_ErrorHandler (UNABLE_TO_ADD_TCFILTER,thisFileName, __LINE__));
  else
    return(SUCCESS_IEC905);
  
}

VOID notifyTCEventHndlr(
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

int iec90_5_tx_init()
{
int numBufferBytes = sizeof(interfaces);


if (myClientRegistrationHndle!=NULL)	  //then already initialized, just return SUCCESS
  return(SUCCESS_IEC905);

//need to initialize the Traffic Control API - MUST HAVE ADMIN PRIVILEGES
  myCompletionFunctions.ClNotifyHandler = (TCI_NOTIFY_HANDLER )& notifyTCEventHndlr;
//  myCompletionFunctions.ClAddFlowCompleteHandler = (TCI_ADD_FLOW_COMPLETE_HANDLER)&addTCCompletHndlr;
//  myCompletionFunctions.ClModifyFlowCompleteHandler = (TCI_MOD_FLOW_COMPLETE_HANDLER)&modTCCompleteHndlr;
//  myCompletionFunctions.ClDeleteFlowCompleteHandler = (TCI_DEL_FLOW_COMPLETE_HANDLER)&delTCCompleteHndlr;
  myCompletionFunctions.ClAddFlowCompleteHandler = NULL;
  myCompletionFunctions.ClModifyFlowCompleteHandler = NULL;
  myCompletionFunctions.ClDeleteFlowCompleteHandler = NULL;

  if((lastTXInitError = TcRegisterClient( CURRENT_TCI_VERSION,(HANDLE) &userContext,&myCompletionFunctions,&myClientRegistrationHndle))!=NO_ERROR)
  {
    if(lastTXInitError ==ERROR_OPEN_FAILED)
      {
	IEC905_ErrorHandler (TC_OPEN_FAILED_CHK_ADMIN_PRIV,thisFileName, __LINE__);
        return(TC_OPEN_FAILED_CHK_ADMIN_PRIV);
      }
    IEC905_ErrorHandler (TC_REGISTRATION_ERROR,thisFileName, __LINE__);
    return(TC_REGISTRATION_ERROR);
  }

//now need to get the set of intrfaces that support the Traffic Control API
if((lastTXInitError = TcEnumerateInterfaces(myClientRegistrationHndle, &numBufferBytes, (PTC_IFC_DESCRIPTOR) interfaces))!=NO_ERROR)
  {
    IEC905_ErrorHandler (TC_INTERFACE_LISTING_FALIED,thisFileName, __LINE__);
    TcDeregisterClient(myClientRegistrationHndle);	//deRegister if it failed.
    myClientRegistrationHndle = NULL;
    return(TC_INTERFACE_LISTING_FALIED);
  }

pDescriptor = (PTC_IFC_DESCRIPTOR )interfaces;	      //save for future use

return(SUCCESS_IEC905);
}

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

int iec90_5_tx_open_socket_with_priorities(IECSOCKET_TX_INFO **pUserSocketInfo, char *pInterfaceID, ETHERTYPE_8021Q *p8021Q, USHORT *pIP_TOS_DSCP, IEC_KEY_ADDRESS *pDestAddress)
{
ULONG retVal;
HANDLE FlowHandle=NULL;
HANDLE IfcHandle=NULL;
HANDLE CIFlowCtx = 1;
HANDLE FilterHandle;
PTC_GEN_FLOW  _ppTcFlowObj=NULL;
LPWSTR pInterfaceName=NULL;
GUID qosGuid = GUID_QOS_FLOW_MODE;

#define UDP_MULTICAST_TTL 128
DWORD ttl=UDP_MULTICAST_TTL;
DWORD cksum= 1;			  //enabling UDP Checksum
IECSOCKET_TX_INFO *pSocketInfo;

  *pUserSocketInfo = NULL;
//see if an interface can be openned
  if((pInterfaceName = find_InterfaceName(pInterfaceID))==NULL)
  {
    //the specified interface could not be found
    IEC905_ErrorHandler (ETHERNET_INTERFACE_NOT_FOUND,thisFileName, __LINE__);
    return(ETHERNET_INTERFACE_NOT_FOUND);
  }

  if((retVal =  TcOpenInterface(pInterfaceName, myClientRegistrationHndle,userContext, &IfcHandle))!=NO_ERROR)
  {
    //could not open interface that was found
    IEC905_ErrorHandler (ETHERNET_INTERFACE_OPEN_FAILED,thisFileName, __LINE__);
    return(ETHERNET_INTERFACE_OPEN_FAILED);
  }

// ---- will need to check if the flow already exists   before doing all this other stuff
// because Microsoft is not smart enough to know that it is the same flow

  //have to set the Interface to be Differentiated  QOS Service
  if(retVal = TcSetInterface(IfcHandle,&qosGuid,sizeof(ULONG),&adapterQOSMode))
  {
    IEC905_ErrorHandler (INTERFACE_DIFFSERV_FAILED,thisFileName, __LINE__);
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

  //now need to open the sending socket and allocate the return structure
  pSocketInfo = (IECSOCKET_TX_INFO *)calloc(1,sizeof(IECSOCKET_TX_INFO));

  pSocketInfo->FilterHandle=FilterHandle;
  pSocketInfo->FlowHandle = FlowHandle;
  if((pSocketInfo->SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==INVALID_SOCKET)
    {
    free(pSocketInfo);		    //free the allocated structure
    pSocketInfo= NULL;
    TcDeleteFilter(FilterHandle);   //delete the filter, leave the flow
    return(IEC905_ErrorHandler (UNABLE_TO_CREATE_SENDING_SOCKET,thisFileName, __LINE__));
    }
  else
    {
    //set the TTL
    if(setsockopt(pSocketInfo->SendingSocket,IPPROTO_IP,IP_MULTICAST_TTL,(char *)&ttl,sizeof(ttl)))
      {
      //this error is important as it will not allow the packets to route if it fails
      free(pSocketInfo);		    //free the allocated structure
      pSocketInfo= NULL;
      TcDeleteFilter(FilterHandle);   //delete the filter, leave the flow
      return(IEC905_ErrorHandler (UNABLE_TO_CREATE_SENDING_SOCKET,thisFileName, __LINE__));
      }
    //set the UDP_XSUM option, don't care if this succeeds or fails since the XSUM is an option
    setsockopt(pSocketInfo->SendingSocket,IPPROTO_UDP,UDP_CHECKSUM_COVERAGE,(char *)&cksum,sizeof(cksum));
    *pUserSocketInfo = pSocketInfo;
    }

#if 0
 SOCKET               SendingSocket;
  DWORD ttl;
//  DWORD cksum;
  DWORD nocksum = 0;	/* use this to "enable" checksum	*/

  // Create a new socket to send datagrams on.
  SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (SendingSocket == INVALID_SOCKET)
    printf("Client: Error at socket(): %ld\n", WSAGetLastError());
  else
    {
    // Add this to set TTL
    ttl = 128;
    if (setsockopt(SendingSocket,IPPROTO_IP,IP_MULTICAST_TTL,
        (char *)&ttl,sizeof(ttl)))
      printf ("Error %d setting IP_MULTICAST_TTL socket option", WSAGetLastError());

#if 0	//DEBUG: didn't work on XP
    cksum = 1;
    if (setsockopt(SendingSocket,IPPROTO_UDP,UDP_CHECKSUM_COVERAGE,
        (char *)&cksum,sizeof(cksum)))
      printf ("Error %d setting UDP_CHECKSUM_COVERAGE socket option", WSAGetLastError());
#endif

    if (setsockopt(SendingSocket,IPPROTO_UDP,UDP_NOCHECKSUM,
        (char *)&nocksum,sizeof(nocksum)))
      printf ("Error %d setting UDP_NOCHECKSUM socket option", WSAGetLastError());

#if 0	//DEBUG: do we need this?
    if (setsockopt(SendingSocket,IPPROTO_IP,IP_TTL,
        (char *)&ttl,sizeof(ttl)))
      printf ("Error %d setting IP_TTL socket option", WSAGetLastError());
#endif
    }
  return (SendingSocket);
  }
#endif
  return(SUCCESS_IEC905);
}

int iec90_5_tx_close_socket (IECSOCKET_TX_INFO *SendingInfo)
{
ULONG retVal;

  if(SendingInfo==NULL)
    return(SUCCESS_IEC905);

  TcDeleteFilter(SendingInfo->FilterHandle);
  TcDeleteFlow(SendingInfo->FlowHandle);
  closesocket(SendingInfo->SendingSocket);
  free(SendingInfo);

  return(SUCCESS_IEC905);

}