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
/* MODULE NAME : iec_90_5_udp_rx.c					    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	UDP Subscriber receive functions.				    */
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

#ifdef _WIN32
#include <ws2tcpip.h>	/* Add this for "ip_mreq".	*/
#include <windows.h>
#include <winsock2.h>
#endif

#include "iec_90_5.h"



/************************************************************************/
/*	iec905_ip_init				  		*/
/*									*/
/*   Function intializes the IP interface				*/
/*									*/
/*  Inputs: None							*/
/*									*/
/*  Outputs: A return code of SUCCESS or Error indication		*/
/************************************************************************/
int iec905_ip_init()
{
WORD wVersionRequested;
WSADATA wsaData;
int err;

/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    if(err = WSAStartup(wVersionRequested, &wsaData))
      return(UNABLE_TO_INTIALIZE_IP_INTERFACE );
    else
      return(SUCCESS_IEC905);
}



/************************************************************************/
/************************************************************************/
static int iec_udp_rx_init (unsigned short IPPort, SOCKET *pReceivingSocket, unsigned long UDPSckBufSize)
     {
     SOCKET             ReceivingSocket;
     SOCKADDR_IN        ReceiverAddr;
     SOCKADDR_IN        SenderAddr;
     int                SenderAddrSize = sizeof(SenderAddr);
     int                ByteReceived = 5;
     unsigned short	rxPort;
     int retVal;
     unsigned long myBufSize=UDPSckBufSize;

#if defined(_WIN32)
ST_ULONG nonblock_on=1;	/* CRITICAL: must be non-zero to enable non-blocking*/
#else
int nonblock_on=1;	/* CRITICAL: must be non-zero to enable non-blocking*/
#endif

  *pReceivingSocket= (SOCKET)NULL;
   if(IPPort==0)
      return(	IEC905_ErrorHandler (IP_PORT_INCORRECT_IEC905,__FILE__, __LINE__));	  //won't allow an initialization, a non-zero port must be specified

  // NOTE: assume WSAStartup already called.
  // Create a new socket to receive datagrams on.


  ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
 
  if (ReceivingSocket == INVALID_SOCKET)
      return(	IEC905_ErrorHandler (UNABLE_TO_OPEN_UDP_SOCKET_IEC905,__FILE__, __LINE__));
 
  // Set up a SOCKADDR_IN structure that will tell bind that we
  // want to receive datagrams from all interfaces using port 5150.
    rxPort = htons(IPPort);

  // The IPv4 family
  ReceiverAddr.sin_family = AF_INET;
  // Get Port no. from config
  ReceiverAddr.sin_port = rxPort;
  // From all interface (0.0.0.0)
  ReceiverAddr.sin_addr.s_addr = inet_addr ("0.0.0.0");	//binds OK. Receives loopback UDP OK.
 
  // Associate the address information with the socket using bind.
  // At this point you can receive datagrams on your bound socket.
  if (bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR)
    {
    printf("Server: bind() failed! Error: %d.\n", WSAGetLastError());
    // Close the socket
    closesocket(ReceivingSocket);
    // and exit with error
    return(IEC905_ErrorHandler (UDP_RX_BIND_FAILED_IEC905,__FILE__, __LINE__));
    }

  // Some info on the receiver side...
  getsockname(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));

  //override the UDP default socket buffer size
 
  if(UDPSckBufSize)
    retVal = setsockopt(ReceivingSocket,SOL_SOCKET,SO_RCVBUF,(char *)&myBufSize,sizeof(myBufSize));

  /* This ioctl call should make recvfrom return immediately.	*/
  ioctlsocket (ReceivingSocket, FIONBIO, &nonblock_on);


  *pReceivingSocket = ReceivingSocket;
  
  return (SUCCESS_IEC905);
  }

/************************************************************************/
/************************************************************************/
/* Function used to initialize the IEC 90-5 RX socket			*/

int iec905_rx_init(SOCKET *pReceivingSocket, unsigned long UDPSckBufSize )
{
  return(iec_udp_rx_init(IEC_61850_90_5_UDP_RX_PORT, pReceivingSocket,UDPSckBufSize));
}

/************************************************************************/
/* this function will attempt to enroll in a multicast group			*/
/*  Steps:																*/
/* 		1). create an appropriate payload								*/
/*		2). interact with the KDC to get the keys						*/
/*			if the keys aren't available, abort							*/
/*		3). create the multicast subscription							*/
/* 			if NOK, cleanup and abort									*/
/* and get the keys required											*/
/************************************************************************/
IEC905_MSG_CNTRL *iec905_igmpv3_group_enroll(ST_UINT8 usageType,  ST_UINT8 typeOfAddress, char *pMultiAddress, char *pSrcAddress, char *pDatSetRef, SOCKET pInReceivingSocket)
     {
     SOCKADDR_IN        SenderAddr;
     int                SenderAddrSize = sizeof(SenderAddr);
     int                ByteReceived = 5;
     IEC_COMM_ADDRESS	pAddress;
     unsigned long	multiAddress,srcAddress;		  /* for IPv4 */

     

struct ip_mreq_source mreq_source;	/* for Source Specific Multicast (SSM) IGMPv3	*/
struct ip_mreq mreq;			/* for use with IGMPv2 - used if pSrcAddress NULL) */
IEC905_MSG_CNTRL *pTempPayload;
SOCKET pReceivingSocket=pInReceivingSocket;
int retValue;

//only supporting ipV4 for now
    if((typeOfAddress!=IP_V4_ADDRESS_TYPE) || (pMultiAddress==NULL) || (pReceivingSocket==(SOCKET)NULL)) 
	{
	  IEC905_ErrorHandler (INVALID_IGMP_GROUP_ENROLL_PARAMETERS_IEC905,__FILE__, __LINE__);
	  return(NULL);
	}


    //fill in pAddress
    pAddress.typeOfAddress = typeOfAddress;
    pAddress.lenOfAddress = 4;
    multiAddress = inet_addr(pMultiAddress);
    if((pAddress.pAddress = calloc(1,4))==NULL)
     {
	IEC905_ErrorHandler (MEMORY_ALLOCATION_ERROR,__FILE__, __LINE__);
	return(NULL);
      }
    memcpy(pAddress.pAddress,(unsigned char *)&multiAddress,4);


    if((pTempPayload= iec905_create_msg_cntrl_rx( usageType, &pAddress,NULL,pDatSetRef))==NULL)
      {
	free(pAddress.pAddress);
	IEC905_ErrorHandler (DUPLICATE_PAYLOAD_DETECTED_IEC905,__FILE__, __LINE__);
	return(NULL);
      }
    else
      free(pAddress.pAddress);


    if(pSrcAddress)
      {

      pTempPayload->srcKeyAddress.lenOfAddress=4;
      pTempPayload->srcKeyAddress.typeOfAddress = typeOfAddress;
      if((pTempPayload->srcKeyAddress.pAddress = calloc(1,4))==NULL)
      {
	
	IEC905_ErrorHandler ( MEMORY_ALLOCATION_ERROR,__FILE__, __LINE__);
	return(NULL);
      }

      srcAddress =inet_addr(pSrcAddress);
      memcpy(pTempPayload->srcKeyAddress.pAddress,(unsigned char *)&srcAddress,sizeof(unsigned long));

      mreq_source.imr_multiaddr.s_addr=multiAddress;
      mreq_source.imr_interface.s_addr=htonl(INADDR_ANY);
      mreq_source.imr_sourceaddr.s_addr=srcAddress;

      if (setsockopt(pReceivingSocket,IPPROTO_IP,IP_ADD_SOURCE_MEMBERSHIP,
        (char *)&mreq_source,sizeof(mreq_source)))
	{
	retValue = WSAGetLastError();		    //just for debug purposes, not used
	if(retValue!=0x2740)
	  {
	  IEC905_ErrorHandler (IGMPV3_GROUP_MEMBERSHIP_ENROLL_FAILED_IEC905,__FILE__, __LINE__);
	  iec905_destroy_msg_cntrl (pTempPayload);
	  pTempPayload =  NULL;	
	  }
	}
      }
    else
      {
      mreq.imr_multiaddr.s_addr=multiAddress;;
      mreq.imr_interface.s_addr=htonl(INADDR_ANY);
      if (setsockopt(pReceivingSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&mreq,sizeof(mreq)))
	{
	IEC905_ErrorHandler (IGMPV2_GROUP_MEMBERSHIP_ENROLL_FAILED_IEC905,__FILE__, __LINE__);
	iec905_destroy_msg_cntrl (pTempPayload);
	pTempPayload =  NULL;
	}
      }
  

   return(pTempPayload);
  }

/******************************************************************************/
/*                     igmov3_group_destroy				      */
/* this function is called to unsibscribe from a multicast group	      */
/* the function steps:							      */
/*	1).  Unsubscribe from the IP multicast group (V3 or V2)		      */
/*	2).  Destory the Payload/Keys that are associated with the	      */
/*	      subscription.						      */
/* Failure to remove from the multicast group will result in no destruction   */
/* of the Payload or keys since information will still be being received      */
/*									      */
/*  Return expected:  NULL upon success.  Failure will return the pointer     */
/*	      that was passed in					      */
/******************************************************************************/
IEC905_MSG_CNTRL *iec905_igmpv3_group_destroy(IEC905_MSG_CNTRL *pPayload,SOCKET pInReceivingSocket)
{
SOCKET pReceivingSocket=pInReceivingSocket;
struct ip_mreq_source mreq_source;	/* for Source Specific Multicast (SSM) IGMPv3	*/
struct ip_mreq mreq;			/* for use with IGMPv2 - used if pSrcAddress NULL) */
IEC905_MSG_CNTRL *pTempPayload=pPayload;
unsigned long ipV4address,srcIpV4Address;
int retValue;

    if(pPayload==NULL)
      {
      IEC905_ErrorHandler (INVALID_IGMP_GROUP_UNSUB_PARAMETER_IEC905,__FILE__, __LINE__);
      return(NULL);	  //just a safety check
      }
//section that attempts to unsubscribe from the multicast group (IPv4 is only supported at this time
    if((pPayload->keyAddress.typeOfAddress!=IP_V4_ADDRESS_TYPE)||(pReceivingSocket==(SOCKET)NULL))
      return(pPayload);

    if(pPayload->srcKeyAddress.lenOfAddress>0)
      {
      ipV4address = *(unsigned long *)pPayload->keyAddress.pAddress;
      mreq_source.imr_multiaddr.s_addr=ipV4address;
      mreq_source.imr_interface.s_addr=htonl(INADDR_ANY);
      srcIpV4Address= *(unsigned long *)pPayload->srcKeyAddress.pAddress;
      mreq_source.imr_sourceaddr.s_addr=srcIpV4Address;

      if (setsockopt(pReceivingSocket,IPPROTO_IP,IP_DROP_SOURCE_MEMBERSHIP,
        (char *)&mreq_source,sizeof(mreq_source)))
	 {
	  retValue = WSAGetLastError();		    //for debug purposes only
	  IEC905_ErrorHandler (INVALID_IGMP_GROUP_UNSUBV3_FAILURE_IEC905,__FILE__, __LINE__);
	 }
	else
	{  
	  iec905_destroy_msg_cntrl (pTempPayload);
	  pTempPayload =  NULL;
	}
      }
    else
      {
      ipV4address = *(unsigned long *)pPayload->keyAddress.pAddress;
      mreq.imr_multiaddr.s_addr=ipV4address;
      mreq.imr_interface.s_addr=htonl(INADDR_ANY);
      if (!setsockopt(pReceivingSocket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char *)&mreq,sizeof(mreq)))
	{
	retValue = WSAGetLastError();		    //for debug purposes only
	IEC905_ErrorHandler (INVALID_IGMP_GROUP_UNSUBV2_FAILURE_IEC905,__FILE__, __LINE__);
	iec905_destroy_msg_cntrl (pTempPayload);
	pTempPayload =  NULL;
	}
      }

 return(pTempPayload);

}


/******************************************************************************/
/*                     iec905_close_socket				      */
/* closes the receive socket and may eventually free other allocated resources*/
/* if needed								      */
/******************************************************************************/
void iec905_close_socket(pInReceivingSocket)
{
    closesocket(pInReceivingSocket);
}