/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : udp_sub.c						*/
/* PRODUCT(S)  : MMS-EASE Lite						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	UDP Subscriber sample application				*/
/*									*/
/* NOTE: If this is linked with 'udp_pub.c', USE_SUBSCRIBER_THREAD must	*/
/*	be defined, to avoid having 2 "main" functions.			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*	NONE								*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 08/31/11  JRB	   Fix CLTP decoding.				*/
/* 07/22/11  JRB	   Initial revision				*/
/************************************************************************/

#include "glbtypes.h"
#include "sysincs.h"
#include "udp.h"
#include "gensock2.h"
#include "ethertyp.h"
#include "mvl_acse.h"	/* only for MVL_CFG_INFO which is only for osicfgx*/
#include "iec_90_5.h"
/************************************************************************/
/* For debug version, use a static pointer to avoid duplication of 	*/
/* __FILE__ strings.							*/
/************************************************************************/

#ifdef DEBUG_SISCO
SD_CONST static ST_CHAR *SD_CONST thisFileName = __FILE__;
#endif

/************************************************************************/
/* Global variables.							*/
/************************************************************************/

/************************************************************************/
//DEBUG: standard vlan_hdr_encode needed ptr 1 byte ahead of data.
//use this instead.
/************************************************************************/
ST_UCHAR *vlan_hdr_encode_new(ST_UCHAR *bufPtr, /* buffer to encode into */
	ST_INT *asn1Len,  /* ptr to len encoded    */
	ST_UINT16 tci) 	//DEBUG: old funct got tci from ETYPE_INFO struct
  {
  /* Assume we have a good buffer length from etype_encode() */

  /* Encode the VLAN info, from back to front */
  *(--bufPtr) = (ST_UCHAR) (tci & 0xFF);
  *(--bufPtr) = (ST_UCHAR) (tci >> 8);   /* TCI */	
  *(--bufPtr) = (ST_UCHAR) 0x00;
  *(--bufPtr) = (ST_UCHAR) 0x81;	       /* TPID */
  
  (*asn1Len) += VLAN_HEAD_LEN;
  
  return (bufPtr);
  }

/************************************************************************/
/*			init_log_cfg					*/
//DEBUG: copied from scl_srvr.c but removed exit & added return
// do like this in scl_srvr.c too??
/************************************************************************/
static ST_RET init_log_cfg (ST_VOID)
  {
#ifdef DEBUG_SISCO

#if defined(S_SEC_ENABLED)
  logCfgAddMaskGroup (&secLogMaskMapCtrl);
  logCfgAddMaskGroup (&ssleLogMaskMapCtrl);
#endif
#if defined(SISCO_STACK_CFG)
  logCfgAddMaskGroup (&sscLogMaskMapCtrl);	//DEBUG: add this for sstackcfgxml.c??
						//not in S_SEC section cuz configs stack AND ssec stuff
#endif
  logCfgAddMaskGroup (&ipcLogMaskMapCtrl);	//DEBUG: added for snap. put ifdef around?
						//DEBUG: this pulls in ipc_utl.c even if no other code used from there.

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

#if 0
/************************************************************************/
/************************************************************************/
ST_UCHAR *udp_tunnel_dec (
	ST_UCHAR *udp_pdu_ptr,		/* UDP PDU pointer	*/
	ST_INT udp_pdu_len,		/* UDP PDU length 	*/
	ST_UCHAR *dst_mac,		/* out: DST MAC (must be 6 bytes)	*/
	ST_UINT16 *tci_ptr,		/* out: TCI decoded		*/
	ST_UINT *dec_len_ptr)		/* in/out: ptr to decoded length*/
  {
ST_UCHAR *cur_ptr;
ST_UINT32 spdu_len;
ST_UINT32 payload_len;
ST_UINT16 tunnel_pdu_len;
ST_UINT32 spdu_num;	/* increments each time PDU sent	*/
ST_UINT16 spdu_ver;	/* should always be 1	*/
ST_UINT16 tci;
ST_INT extra_header_bytes;	/* number of header bytes to ignore.	*/

  /* Check for correct CLTP Transport and Session encoding.	*/
  if (udp_pdu_ptr [0] != 0x01  ||	/* LI - Transport Unit Data header len (variable part empty)*/
      udp_pdu_ptr [1] != 0x40  ||	/* Transport Unit Data PDU	*/
      udp_pdu_ptr [2] != 0xa0  ||	/* SI (Session Identifier)	*/
      udp_pdu_ptr [4] != 0x80)		/* commonHeader	*/
    return (NULL);	/* invalid PDU	*/

  if (udp_pdu_ptr [5]			/* commonHeader	length	*/
      != 0x12)
    return (NULL);	/* invalid PDU	*/

  if (udp_pdu_ptr [3] <			/* header len	*/
      udp_pdu_ptr [5]			/* commonHeader	length	*/
      + 2)
    return (NULL);	/* invalid PDU	*/
  extra_header_bytes = udp_pdu_ptr [3] - udp_pdu_ptr [5] - 2;

  cur_ptr = &udp_pdu_ptr [6];	/* point after commonHeader length. */

  /* do reverse of encode	*/
  spdu_len = ntohl (*(ST_UINT32 *)cur_ptr);	/* SPDU len	*/
  cur_ptr += sizeof(spdu_len);

  spdu_num = ntohl (*(ST_UINT32 *)cur_ptr);	/* SPDU number	*/
  cur_ptr += sizeof(spdu_num);

  spdu_ver = ntohs (*(ST_UINT16 *)cur_ptr);	/* SPDU Version	*/
  cur_ptr += sizeof(spdu_ver);
  
  //DEBUG: just leave space for security stuff for now. Fill this in later.
  cur_ptr += SECURITY_INFO_SIZE;

  payload_len = ntohl (*(ST_UINT32 *)cur_ptr);
  cur_ptr += sizeof(payload_len);

  if (*cur_ptr++ != 0x83)			/*tag for Tunnelled PDU */
	  return (NULL);
  /* skip to the tci   1- simulation, 2 - appid, 2 tpid */
  cur_ptr +=5;
 
  tci = ntohs (*(ST_UINT16 *)cur_ptr);	/* TCI	*/
  cur_ptr += sizeof(tci);

    /* START payload	*/
  memcpy (dst_mac, cur_ptr, 6);
  cur_ptr += 6;

  /* Encode the "tunnel_pdu_len"	*/
  tunnel_pdu_len = ntohs (*(ST_UINT16 *)cur_ptr);
  cur_ptr += sizeof(tunnel_pdu_len);

  if (tunnel_pdu_len > udp_pdu_ptr + udp_pdu_len - cur_ptr)
    {
    printf ("ERROR: Debug this.\n");	//DEBUG: do we ever get here?
    USER_LOG_ERR1 ("ERROR: decoded tunnel pdu len '%u' exceeds number of bytes received.",
                   tunnel_pdu_len);
    return (NULL);	/* ERROR	*/
    }

  *dec_len_ptr = tunnel_pdu_len;	/* len to return to caller	*/
  *tci_ptr = tci;
  return (cur_ptr);	/* should now point to Tunneled PDU	*/
  }
#endif


ST_UCHAR *udp_tunnel_dec (
	ST_UCHAR *udp_pdu_ptr,		/* UDP PDU pointer	*/
	ST_INT udp_pdu_len,		/* UDP PDU length 	*/
	ST_UCHAR *dst_mac,		/* out: DST MAC (must be 6 bytes)	*/
	ST_UINT16 *tci_ptr,		/* out: TCI decoded		*/
	ST_UINT *dec_len_ptr)		/* in/out: ptr to decoded length*/
{
	IEC_90_5_RX rxd;
	IEC_90_5_PAYLOAD_DEF *pPayload;

	rxd.lengh_of_rxd_buffer = udp_pdu_len;
	rxd.rxd_buffer = udp_pdu_ptr;
	udp_iec_90_5_dec( &rxd);
	pPayload = rxd.payload;
	*tci_ptr = pPayload->tci;
	memcpy(dst_mac,pPayload->dst_mac,6);
	return(NULL);
}

/************************************************************************/
/*			subscriber_thread				*/
/* Main code for IEC 61850-90-5 subsriber.				*/
/* May be spawned as a thread.						*/
/************************************************************************/
ST_THREAD_RET ST_THREAD_CALL_CONV subscriber_thread (ST_THREAD_ARG ta)
  {
ST_RET ret;
UDP_SUB_CFG udp_sub_cfg;
SOCKET ReceivingSocket;		/* initialized by udp_rx_init	*/
ST_UCHAR rx_buf [2048];		/* should be big enough for any message	*/
ST_INT rx_len;
ST_UCHAR *tunnel_pdu_ptr;
ST_UINT16 tci;		/* TCI decoded		*/
ST_INT dec_len;	/* decoded len		*/
ST_INT enc_len;	/* encoded len		*/
ST_UCHAR *enc_ptr;
ST_UCHAR dst_mac [6];
ST_INT j;

  ret = udp_sub_cfg_read ("udp_sub.cfg", &udp_sub_cfg);
  if (ret)
    {
    printf ("ERROR reading 'udp_sub.cfg'. Subscriber disabled.\n");
    return ((ST_THREAD_RET)ret);
    }

  ret =  udp_rx_init (&udp_sub_cfg, &ReceivingSocket);
  if (ret)
    {
    printf ("ERROR initializing receiving socket\n");
    return ((ST_THREAD_RET)ret);
    }

  /* Wait for incoming UDP "Tunnelled" messages.	*/
  while (1)
    {
    fd_set readfds;
    int nfds;
    FD_ZERO (&readfds);
    FD_SET (ReceivingSocket, &readfds);
    /* Wait forever for incoming UDP packet.	*/
#if defined(_WIN32)
    nfds = 1;	/* On _WIN32, ignored, but compiler wants it initialized.*/	
#else
    nfds = ReceivingSocket+1;    /* ReceivingSocket is only fds to check, so set "nfds=ReceivingSocket+1".*/
#endif
    nfds = select (nfds, &readfds, NULL, NULL, NULL);	/* wait forever	*/
    if (nfds > 0)
      {	/* UDP packet available on the socket. Receive it.	*/
      rx_len = sizeof (rx_buf);	/* set "maximum" msg len	*/
      udp_rx (ReceivingSocket, rx_buf, &rx_len);
      USER_LOG_SERVER0 ("UDP packet received");
      USER_LOG_SERVERH (rx_len, rx_buf);

      
      tunnel_pdu_ptr = udp_tunnel_dec (
	rx_buf,		/* UDP PDU pointer	*/
	rx_len,		/* UDP PDU length 	*/
	dst_mac,	/* out: DST MAC (must be 6 bytes)	*/
	&tci,		/* out: TCI decoded		*/
	&dec_len);	/* out: ptr to decoded length*/

      /* If dst_mac matches any MAC_remap entry in udp_sub.cfg, remap it.	*/
      /* NOTE: If remap not configured, MACin is all 0, so should never match dst_mac.*/
      for (j = 0; j < udp_sub_cfg.numMACAddr; j++)
        {
        if (memcmp (dst_mac, udp_sub_cfg.MACin[j], 6) == 0)
          {
          memcpy (dst_mac, udp_sub_cfg.MACout[j], 6);	/* remap destination MAC*/
          break;
          }
        }

      /* Now we have GOOSE APDU. We need to add the VLAN information.	*/
 
//DEBUG: for now ignore decoded TCI. Just use configured values.
//Do we want to keep this? If so, delete tci decoding above.
// Also, do this just once at startup.

      /* Compute TCI from VLAN-PRIORITY and VLAN-ID.		*/
      /* CFI (bit 12) in TCI is always 0, so do nothing with it.	*/
      tci =  udp_sub_cfg.VLAN_PRIORITY << 13;	/* this also forces bit 12 to be 0*/
      tci |= udp_sub_cfg.VLAN_ID;

      /* NOTE: We're encoding backwards in rx_buf, overwriting UDP header info.*/
      assert (dec_len + 16 <= rx_len);	/* make sure there's room for VLAN, dst_mac, src_mac*/
      enc_len = dec_len;
      /* NOTE: tunnel_pdu_ptr is now pointing somewhere in the middle of rx_buf.*/

      //DEBUG: vlan_hdr_encode needs pointer 1 byte ahead of tunnel_pdu_ptr
      //       This function is easier to use.
      enc_ptr = vlan_hdr_encode_new(tunnel_pdu_ptr,	/* buffer to encode into	*/
                &enc_len,	/* ptr to len encoded	*/
                tci);

      /* Send "real" GOOSE.	*/
      enc_ptr -= 6;	/* point to SRC MAC	*/
      memcpy (enc_ptr, clnp_param.loc_mac, CLNP_MAX_LEN_MAC);	/* SRC MAC*/
      enc_ptr -= 6;	/* point to DST MAC	*/
      memcpy (enc_ptr, dst_mac, CLNP_MAX_LEN_MAC);/* DST MAC*/
      enc_len += 12;	/* add 12 for both MACs	*/
      clnp_snet_write_raw (enc_ptr, enc_len);
      }
    }
  return ((ST_THREAD_RET)SD_SUCCESS);
  }

/* NOTE: include "main" only if "subscriber_thread" is NOT spawned as a	*/
/*       separate thread.						*/
#if !defined(USE_SUBSCRIBER_THREAD)
/************************************************************************/
/*                       main						*/
/************************************************************************/
int main ()
  {
ST_RET ret;
GEN_SOCK_CTXT *sockCtx;
MVL_CFG_INFO mvlCfg;

  setbuf (stdout, NULL);    /* do not buffer the output to stdout   */
  setbuf (stderr, NULL);    /* do not buffer the output to stderr   */

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

  ret = clnp_snet_init (&clnp_param);	/* pass global struct	*/
  if (ret)
    {
    printf ("clnp_snet_init error 0x%X", ret);
    return (ret);
    }

  /* initialize gensock2 before calling any of the socket functions	*/
  /* DEBUG: only to start up sockets. delete this and init directly?	*/
  sockCtx = calloc (1, sizeof(GEN_SOCK_CTXT));
  ret = sockStart ("udp_test", sockCtx);
  if (ret != SD_SUCCESS)
    {
    return (ret);
    }

  /* This basically waits for incoming UDP "Tunnelled" messages,	*/
  /* and processes them.						*/
  subscriber_thread (NULL);

  //DEBUG: call sockEnd or something to clean up if we find way to break out of "subscriber_thread".

  return (0);
  }
#endif	/* !defined(USE_SUBSCRIBER_THREAD)	*/


