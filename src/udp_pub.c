/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : udp_pub.c						*/
/* PRODUCT(S)  : MMS-EASE Lite						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	UDP Publisher sample application				*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*	NONE								*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 08/31/11  JRB	   Fix CLTP encoding.				*/
/* 07/22/11  JRB	   Initial revision				*/
/************************************************************************/
#include "glbtypes.h"
#include "sysincs.h"
#include "udp.h"
#include "gensock2.h"
#include "clnp_sne.h"	/* for "clnp_snet_*"	*/
#include "mvl_acse.h"	/* only for MVL_CFG_INFO which is only for osicfgx*/
#include "iec_90_5.h"
#include <ws2tcpip.h>	/* for IP_MULTICAST_TTL	*/
/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

static ST_UCHAR test_key[] = {
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88};

ST_UCHAR test_id[]={0x99,0x98,0x97,0x96};


//for unit testing only
extern ST_VOID repeat_unit_test_key_payloads(int num_repeats);
extern ST_VOID repeat_unit_test_igmp(int num_repeats, SOCKET rxSocket);

/************************************************************************/
/************************************************************************/
SOCKET udp_pub_socket_get (void)
  {
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

/************************************************************************/
/*			init_log_cfg					*/
/* NOTE: need usrLogMaskMapCtrl to configure USER_LOG_*.		*/
/************************************************************************/
static ST_RET init_log_cfg (ST_VOID)
  {
#ifdef DEBUG_SISCO

#if defined(S_SEC_ENABLED)
  logCfgAddMaskGroup (&secLogMaskMapCtrl);
  logCfgAddMaskGroup (&ssleLogMaskMapCtrl);
#endif

//  logCfgAddMaskGroup (&mvlLogMaskMapCtrl);
//  logCfgAddMaskGroup (&mmsLogMaskMapCtrl);
  logCfgAddMaskGroup (&acseLogMaskMapCtrl);
  logCfgAddMaskGroup (&tp4LogMaskMapCtrl);
  logCfgAddMaskGroup (&clnpLogMaskMapCtrl);
//  logCfgAddMaskGroup (&asn1LogMaskMapCtrl);
  logCfgAddMaskGroup (&sxLogMaskMapCtrl);
#if defined(S_MT_SUPPORT)
  logCfgAddMaskGroup (&gsLogMaskMapCtrl);
#endif
  logCfgAddMaskGroup (&sockLogMaskMapCtrl);
  logCfgAddMaskGroup (&memLogMaskMapCtrl);
  logCfgAddMaskGroup (&memDebugMapCtrl);
  logCfgAddMaskGroup (&usrLogMaskMapCtrl);	/* Need this for USER_LOG_*	*/

  /* At initialization, install a SLOGIPC command handler. The       */
  /* build in SLOGIPC handler just receives the command and put's    */
  /* on a list to be handled by the application at it's leisure ...  */
  sLogCtrl->ipc.slog_ipc_cmd_fun = slog_ipc_std_cmd_fun;

  if (logcfgx_ex (sLogCtrl, "logcfg.xml", NULL, SD_FALSE, SD_FALSE) != SD_SUCCESS)
    {
    printf ("\n Parsing of 'logging' configuration file failed.");
    if (sLogCtrl->fc.fileName)
      printf ("\n Check log file '%s'.", sLogCtrl->fc.fileName);
    return (SD_FAILURE);
    }

  slog_start (sLogCtrl, MAX_LOG_SIZE);  /* call after logging parameters are configured	*/
  return (SD_SUCCESS);
#endif  /* DEBUG_SISCO */
  }

  
/************************************************************************/
/* NOTE: tunnel_pdu_ptr points to GOOSE/SMPVAL PDU starting right AFTER	*/
/*   Ethertype ID. This would be easier if it pointed to the		*/
/*   Ethertype ID, but clnp_snet_read doesn't give us that.		*/
/************************************************************************/
ST_UCHAR *udp_tunnel_enc (
	ST_UCHAR *dst_mac,		/* DST MAC (must be 6 bytes)	*/
	ST_UINT16 tci,			/* TCI from original message*/
    ST_UINT16 etype_id,	/* Ethertype ID from original message*/
	ST_UCHAR *etype_pdu_ptr,	/* pointer after Ethertype ID	*/
	ST_UINT16 etype_pdu_len,	/* len of PDU after Ethertype ID*/
	ST_UINT *enc_len_ptr)
  {

IEC_90_5_HDR_INFO hdr_info;
IEC_90_5_PAYLOAD_DEF payload_info;
ST_UCHAR *temp_ptr;
ST_UCHAR key_id[4];


/* example of filling in the hearder  */
hdr_info.SessionIdentifer = IEC_90_5_SI_TYPE_TUNNEL;
hdr_info.hmacAlg = HMAC_ALG_SHA_256_128;;
hdr_info.secAlgType = SEC_ALG_NONE	;
hdr_info.key_ptr = test_key;
hdr_info.key_len = sizeof(test_key);
hdr_info.timeToNextKey= 0;
memset(hdr_info.timeOfCurrentKey,0,4);
memset(key_id,0,4);
hdr_info.key_id = key_id;

/* fill in the payload_information  */
payload_info.payload_tag = TUNNEL_PAYLOAD_TYPE_TAG;
payload_info.appID = 0;
payload_info.pdu_len = etype_pdu_len;
payload_info.pdu_ptr = etype_pdu_ptr;
payload_info.etype_id = etype_id;
payload_info.simulationBit = 0x00;
payload_info.etype_id = etype_id;
payload_info.tci = tci;
payload_info.tpid = 0x00;
memcpy(&payload_info.dst_mac,dst_mac,6);
payload_info.next = NULL;

  temp_ptr = udp_iec_90_5_enc(&hdr_info,&payload_info,enc_len_ptr);
  return (temp_ptr);
  }


/************************************************************************/
/*                       main						*/
/************************************************************************/
int main (int argc, char *argv[])
  {
ST_RET ret;
ST_INT j;
GEN_SOCK_CTXT *sockCtx;
MVL_CFG_INFO mvlCfg;
UDP_PUB_CFG udp_pub_cfg;
SOCKET SendingSocket;	/* socket used to send all UDP packets	*/
ST_UCHAR test_pdu[30];
ST_UCHAR test_mac[6] = {0x01,0x33,0x34,0x35,0x36,0x37};


ST_UCHAR ret_test_mac[6];
ST_UINT16 ret_tci;
unsigned int ret_len;
unsigned int dec_len_ptr;
ST_UCHAR *ret_buf1;

ST_UCHAR *ret_buf;
IEC_90_5_RX rxd;
IEC_90_5_KEY_PAYLOAD_ID *root;
IEC_90_5_KEY_PAYLOAD_ID *root1;
IEC_KEY_ADDRESS keyAddress;
ST_UCHAR ipAddr1[] = {0x10,0x32,0x33,0x01};
ST_UCHAR ipAddr2[] = {0x10,0x32,0x33,0x02};
char *DataSetRef1  = "SISCO_IED/DataSet1";
unsigned long IpV4LocalAddress;
SOCKET IEC_90_5_rx_socket;
char InterfaceID[100];
ETHERTYPE_8021Q Val8021Q;
USHORT IP_TOS_DSCP=46;
IECSOCKET_TX_INFO *mysockInfo=NULL;


  setbuf (stdout, NULL);    /* do not buffer the output to stdout   */
  setbuf (stderr, NULL);    /* do not buffer the output to stderr   */

 
m_heap_check_enable=0x00;

/* This variable used to enable list validation and overwrite checking	*/
/* on every alloc  and free call. 					*/
m_check_list_enable=0x00;

/* Set this variable SD_FALSE to speed up the debug version. When SD_TRUE, it */
/* enables searching the memory list for the element before accessing	*/
/* the memory during chk_realloc and chk_free calls			*/
m_find_node_enable=0x00;

  strcpy( InterfaceID,"{62202186-02E1-44AA-9491-DF64A73F58C2}");
  ip_socket_init();				//initialize ip socket interface
  iec90_5_rx_init(&IEC_90_5_rx_socket);	//initalize the UDP socket (102) for 90-5 and store the results in the passed Socket Pointer
  iec90_5_tx_init();
  Val8021Q.priority =4;
  Val8021Q.vlanID =4;  

  keyAddress.typeOfAddress = IP_V4_ADDRESS_TYPE;
  keyAddress.lenOfAddress = 4;			  //for IPv4
  keyAddress.pAddress = inet_addr(ipAddr1);
//   open_tx_udp_socket_with_priorities(InterfaceID ,&Val8021Q, &IP_TOS_DSCP, &keyAddress);
  iec90_5_tx_open_socket_with_priorities(&mysockInfo,InterfaceID ,&Val8021Q, &IP_TOS_DSCP, &keyAddress);
// now try to send a packet

  
  iec90_5_tx (mysockInfo, "10.32.33.01", 102,DataSetRef1,strlen(DataSetRef1));
  iec90_5_tx_close_socket(mysockInfo);

//   open_tx_udp_socket_with_priorities(InterfaceID ,NULL, &IP_TOS_DSCP, &keyAddress);
//   open_tx_udp_socket_with_priorities(InterfaceID ,NULL ,NULL, &keyAddress);
  init_key_storage();				//initialize the key storage
   generate_symmetric_key(0, 0);

  repeat_unit_test_key_payloads(100);
  repeat_unit_test_igmp(200,IEC_90_5_rx_socket);

  keyAddress.typeOfAddress = IP_V4_ADDRESS_TYPE;
  keyAddress.lenOfAddress = 4;			  //for IPv4
  keyAddress.pAddress = ipAddr1;
  root = create_KeyPayload_tx( IEC_KEY_USAGE_TYPE_GOOSE, &keyAddress, NULL, DataSetRef1);
  keyAddress.pAddress = ipAddr2;
  root1 = create_KeyPayload_tx( IEC_KEY_USAGE_TYPE_GOOSE, &keyAddress,NULL, DataSetRef1);
//  root = create_KeyPayload( IEC_KEY_USAGE_TYPE_GOOSE, &keyAddress, DataSetRef1);
  destroy_keyPayload (root);
  destroy_keyPayload (root1);
//  add_tx_prime_key(&root, KEY_TYPE_AES_128,  sizeof(test_key),  test_key,  test_id,0);
//  add_tx_next_key(&root, KEY_TYPE_AES_128,  sizeof(test_key),  test_key,  test_id,0);
  sprintf(test_pdu,"0123456");
//  manage_tx_key(&root);
  ret_buf = udp_tunnel_enc(test_mac,0x3333,0x88b8,test_pdu,6,&ret_len);
  rxd.lengh_of_rxd_buffer=ret_len;
  rxd.rxd_buffer=ret_buf;
  udp_iec_90_5_dec( &rxd);	
  free_dec_info(&rxd);
//  destroy_tx_root(&root);

  return(0);
 
 
  ret = init_log_cfg ();
  if (ret)
    {
    printf ("log configuration error 0x%X", ret);
    return (ret);
    }

  //DEBUG: only need this to get network_device for PCAP.
  //DEBUG: calling this seems to link in most of stack. Try to fix that.
  ret = osicfgx ("osicfg.xml", &mvlCfg);	/* Ignore new data in mvlCfg*/
  if (ret)
    {
    printf ("osicfgx error 0x%X", ret);
    return (ret);
    }

  /* initialize gensock2 before calling any of the socket functions	*/
  /* DEBUG: only to start up sockets. delete this and init directly?	*/
  sockCtx = chk_calloc (1, sizeof(GEN_SOCK_CTXT));
  ret = sockStart ("udp_test", sockCtx);
  if (ret != SD_SUCCESS)
    {
    return (ret);
    }
#if 0
    /* Spawn the "Subscriber" as a separate thread.	*/
    {
    ST_THREAD_HANDLE thread1Handle;
    ST_THREAD_ID thread1Id;
    gs_start_thread (&subscriber_thread, (ST_THREAD_ARG)NULL, 
		 &thread1Handle, &thread1Id);
    }
    }
  else
    {
    printf ("ERROR reading 'udp_pub.cfg'. Publisher disabled.\n");
    /* No need to spawn thread for Subscriber. Just call as normal funct.*/
    subscriber_thread (NULL);
    }
  if (ret == SD_SUCCESS)	/* Publisher init successful	*/
    {
    SendingSocket = udp_pub_socket_get ();

    /* Wait for incoming GOOSE messages.	*/

    }

  //DEBUG: if we ever break out of loop, should probably also call sockEnd here too.
#endif

  return (0);
  }


  
