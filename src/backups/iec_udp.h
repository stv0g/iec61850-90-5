#ifndef UDP_INCLUDED
#define UDP_INCLUDED
/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : udp.h							*/
/* PRODUCT(S)  : MMS-EASE Lite						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	Structures and functions for UDP publisher and subscriber.	*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 07/22/11  JRB	   Initial revision				*/
/************************************************************************/


#define SECURITY_INFO_SIZE	8	/* num bytes for Security info	*/
#define	UDP_MAX_ADDR		5

/* UDP Publisher configuration.	*/
typedef struct
  {
  IEC_CHAR IPAddr   [UDP_MAX_ADDR][MAX_IDENT_LEN+1];	/* array of multicast IP Addr*/
  IEC_UCHAR MACAddr [UDP_MAX_ADDR][6];			/* array of MAC Addr*/
  IEC_INT numAddr;	/* Number of Addr configured*/
  IEC_INT IPPort;
  } UDP_PUB_CFG;

/* UDP Subscriber configuration.	*/
typedef struct
  {
  IEC_CHAR IPAddr   [UDP_MAX_ADDR][MAX_IDENT_LEN+1];	/* multicast IP Addr	*/
  /* DEBUG: added this SrcAddr for Source Specific Multicast (SSM)	*/
  IEC_CHAR SrcIPAddr   [UDP_MAX_ADDR][MAX_IDENT_LEN+1];	/* Source IP Addr	*/
  IEC_INT numIPAddr;	/* Number of IPAddr configured*/
  IEC_INT numMACAddr;	/* Number of MACin configured*/
  IEC_INT IPPort;
  IEC_UCHAR MACin  [UDP_MAX_ADDR][6];	/* array of MACs to remap	*/
  IEC_UCHAR MACout [UDP_MAX_ADDR][6];	/* array of MACs remapped	*/
  IEC_UINT16 VLAN_ID;
  IEC_UINT16 VLAN_PRIORITY;
  } UDP_SUB_CFG;

IEC_RET udp_pub_cfg_read (
	IEC_CHAR *cfg_filename,	/* usually "udp_pub.cfg"	*/
	UDP_PUB_CFG *udp_pub_cfg);

IEC_RET udp_sub_cfg_read (
	IEC_CHAR *cfg_filename,	/* usually "udp_sub.cfg"	*/
	UDP_SUB_CFG *udp_sub_cfg);

int udp_tx (SOCKET SendingSocket,
	IEC_CHAR *IPAddr,
	IEC_INT IPPort,
	IEC_UCHAR *enc_ptr,
	IEC_UINT enc_len);

IEC_RET udp_rx_init (UDP_SUB_CFG *udp_sub_cfg, SOCKET *pReceivingSocket);
IEC_RET udp_rx (
	SOCKET ReceivingSocket,
	IEC_UCHAR *rx_buf,
	IEC_INT *rx_len);

IEC_THREAD_RET IEC_THREAD_CALL_CONV subscriber_thread (IEC_THREAD_ARG ta);

	
#endif	/* !UDP_INCLUDED	*/
