/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : udp_rx.c						*/
/* PRODUCT(S)  : MMS-EASE Lite						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	UDP Subscriber receive functions.				*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*			udp_rx_init					*/
/*			udp_rx						*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who	   Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 07/22/11  JRB	   Initial revision				*/
/************************************************************************/
#include "glbtypes.h"
#include "sysincs.h"
#include "mem_chk.h"
/* NOTE: "sysincs.h" includes <winsock2.h>.	*/
#include <ws2tcpip.h>	/* Add this for "ip_mreq".	*/
#include "udp.h"
#include "iec_90_5.h"

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

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
IEC_90_5_KEY_PAYLOAD_ID *igmpv3_group_enroll(ST_UINT8 usageType,  ST_UINT8 typeOfAddress, char *pMultiAddress, char *pSrcAddress, char *pDatSetRef, SOCKET pReceivingSocket)
     {
     SOCKADDR_IN        SenderAddr;
     int                SenderAddrSize = sizeof(SenderAddr);
     int                ByteReceived = 5;
     IEC_KEY_ADDRESS	pAddress;
     unsigned long	multiAddress;		  /* for IPv4 */
     unsigned long      srcAddress;		  /* for IPv4 */
     

struct ip_mreq_source mreq_source;	/* for Source Specific Multicast (SSM) IGMPv3	*/
struct ip_mreq mreq;			/* for use with IGMPv2 - used if pSrcAddress NULL) */
IEC_90_5_KEY_PAYLOAD_ID *pTempPayload;

//only supporting ipV4 for now
    if((typeOfAddress!=IP_V4_ADDRESS_TYPE) && (pMultiAddress!=NULL)) 
	return(NULL);

    //fill in pAddress
    pAddress.typeOfAddress = typeOfAddress;
    pAddress.lenOfAddress = 4;
    multiAddress = inet_addr(pMultiAddress);
    pAddress.pAddress = (ST_UCHAR *)&srcAddress;

    if((pTempPayload= create_KeyPayload( usageType, &pAddress,pDatSetRef))==NULL)
		return(NULL);

    if(pSrcAddress)
      {
      mreq_source.imr_multiaddr.s_addr=multiAddress;
      mreq_source.imr_interface.s_addr=htonl(INADDR_ANY);
      mreq_source.imr_sourceaddr.s_addr=inet_addr(pSrcAddress);

      if (setsockopt(pReceivingSocket,IPPROTO_IP,IP_ADD_SOURCE_MEMBERSHIP,
        (char *)&mreq_source,sizeof(mreq_source)))
	{
	destroy_keyPayload (pTempPayload);
	pTempPayload =  NULL;
	}

      pTempPayload->srcKeyAddress.lenOfAddress=4;
      pTempPayload->srcKeyAddress.typeOfAddress = typeOfAddress;
      pTempPayload->srcKeyAddress.pAddress = chk_calloc(1,sizeof(unsigned long));
      memcpy(pTempPayload->srcKeyAddress.pAddress,(char *)&srcAddress,sizeof(unsigned long));
      }
    else
      {
      mreq.imr_multiaddr.s_addr=multiAddress;;
      mreq.imr_interface.s_addr=htonl(INADDR_ANY);
      if (setsockopt(pReceivingSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&mreq,sizeof(mreq)))
	{
	destroy_keyPayload (pTempPayload);
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
IEC_90_5_KEY_PAYLOAD_ID *igmpv3_group_destroy(IEC_90_5_KEY_PAYLOAD_ID *pPayload,SOCKET pReceivingSocket)
{

struct ip_mreq_source mreq_source;	/* for Source Specific Multicast (SSM) IGMPv3	*/
struct ip_mreq mreq;			/* for use with IGMPv2 - used if pSrcAddress NULL) */
IEC_90_5_KEY_PAYLOAD_ID *pTempPayload=pPayload;
unsigned long ipV4address;

    if(pPayload==NULL)
      return(NULL);	  //just a safety check

//section that attempts to unsubscribe from the multicast group (IPv4 is only supported at this time
    if(pPayload->keyAddress.typeOfAddress!=IP_V4_ADDRESS_TYPE)
      return(pPayload);

    if(pPayload->srcKeyAddress.lenOfAddress>0)
      {
      ipV4address = (unsigned long )*pPayload->keyAddress.pAddress;
      mreq_source.imr_multiaddr.s_addr=ipV4address;
      mreq_source.imr_interface.s_addr=htonl(INADDR_ANY);
      ipV4address = (unsigned long )*pPayload->srcKeyAddress.pAddress;
      mreq_source.imr_sourceaddr.s_addr=ipV4address;

      if (!setsockopt(pReceivingSocket,IPPROTO_IP,IP_DROP_SOURCE_MEMBERSHIP,
        (char *)&mreq_source,sizeof(mreq_source)))
	{
	destroy_keyPayload (pTempPayload);
	pTempPayload =  NULL;
	}
      }
    else
      {
      ipV4address = (unsigned long )*pPayload->keyAddress.pAddress;
      mreq.imr_multiaddr.s_addr=ipV4address;
      mreq.imr_interface.s_addr=htonl(INADDR_ANY);
      if (!setsockopt(pReceivingSocket,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char *)&mreq,sizeof(mreq)))
	{
	destroy_keyPayload (pTempPayload);
	pTempPayload =  NULL;
	}
      }

 return(pTempPayload);

}

