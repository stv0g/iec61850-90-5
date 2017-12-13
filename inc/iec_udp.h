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
/* MODULE NAME : iec_udp.h						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Structures and functions for UDP publisher and subscriber.	    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/

#ifndef UDP_INCLUDED
#define UDP_INCLUDED



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
