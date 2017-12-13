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
/* MODULE NAME : iec_90_5.h						    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Structures and functions declarations for 90-5.			    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 02/18/12  HSF	    Added optflds to sv initialize function	    */
/* 12/01/11  HSF	   Initial revision				    */
/****************************************************************************/
#ifndef IEC_90_5_INCLUDED
#define IEC_90_5_INCLUDED

#define SECURITY_INFO_SIZE	8	/* num bytes for Security info	*/


typedef struct iec_61850_90_5_hdr_info{
#define IEC_90_5_SI_TYPE_TUNNEL  0xA0
#define IEC_90_5_SI_TYPE_GOOSE	 0xA1
#define IEC_90_5_SI_TYPE_SV	0xA2
#define IEC_90_5_SI_TYPE_MNGT	 0x83
   ST_UINT8	SessionIdentifer;
/*the following is a set of security related information*/
   ST_UINT32 timeOfCurrentKey;		
   ST_UINT16 timeToNextKey;
#define SEC_ALG_NONE		0
#define SEC_ALG_AES_128_GCM	1
#define SEC_ALG_AES_256_GCM 2
   ST_UCHAR  secAlgType;
#define HMAC_ALG_None			0
#define HMAC_ALG_SHA_256_80		1
#define HMAC_ALG_SHA_256_128	2
#define HMAC_ALG_SHA_256_256	3
#define HMAC_ALG_AES_GMAC_64	4
#define HMAC_ALG_AES_GMAC_128	5
   ST_UCHAR hmacAlg;
   ST_ULONG keyLen;					//length of the key
   ST_CHAR *pKey;					//pointer to the  key  to be used
#define IEC_90_5_SIZE_OF_KEY_ID 4
   ST_CHAR *pKeyID;						//pointer to the key reference for the key being used
  }IEC905_SESS_PDU_HDR_INFO;

typedef  struct IEC905_SESS_PAYLOAD_DEF{
	  struct IEC905_SESS_PAYLOAD_DEF *next;	/*if not NULL, points to another structure.  This allows for the encoding*/
											/*of multiple payload packets*/
#define GOOSE_PAYLOAD_TYPE_TAG  0x81		/*tag for a GOOSE Payload*/
#define SV_PAYLOAD_TYPE_TAG   0x82			/*tag for a sv payload*/
#define TUNNEL_PAYLOAD_TYPE_TAG 0x83		/*tag for a tunnelled payload*/
#define  MNGT_PAYLOAD_TYPE_TAG  0x84		/*tag for mngt request/response payload*/
//the payload tag, simulationBit, and AppID are common information for all payloads*/
//and must be present as does the pdu_len and pdu_ptr*/
	  ST_UINT8		payload_tag;			/*tag for the payload type*/
	  ST_BOOLEAN	simulationBit;			/*True indicates that packet is in simulation mode*/
	  ST_UINT16		appID;					/*ApplicationID*/
	  ST_UINT16		pduLen;				/*pointer to the length of the encoded PDU*/
	  ST_UCHAR		*pPDU;				/*pointer to previously encoded goose/sv packet APDU only*/
/*the following fields are for tunnelled payloads only */
	  ST_UCHAR *dst_mac;			/* DST MAC (must be 6 bytes)	*/
	  ST_UINT16 tpid;				/* TPID from original message   */
	  ST_UINT16 tci;				/* TCI from original message*/
      ST_UINT16 etype_id;			/* Ethertype ID from original message*/
  }IEC905_SESS_PAYLOAD_DEF;


typedef struct iec_90_5_rx
{
IEC905_SESS_PDU_HDR_INFO *pHDR;			/*pointer to allocated header information rxd */
IEC905_SESS_PAYLOAD_DEF *pPayload;		/*pointer to the first payload definition in the payload chain */
ST_UCHAR *pRXDbuffer;				/* pointer to the buffer that was received and decoded - filled in by calling application*/
ST_UCHAR lenRXDBuffer;			/*length of the buffer	- filled in by calling application*/
}IEC_90_5_RX;

/*function definition to call in order to encode a 90_5 PDU */
/* returns a pointer to the encoded buffer that includes CLTP */
extern ST_UCHAR *iec905_sess_enc (
	IEC905_SESS_PDU_HDR_INFO *pHDR,			/*points to the header information*/
	IEC905_SESS_PAYLOAD_DEF *pPayload,	/*points to the first in the chain of payload information (freeing of the information is reponsibility of the caller*/
#define PAYLOAD_TYPE_UNKNOWN		0
#define PAYLOAD_TYPE_SINGLE_GOOSE	1
#define PAYLOAD_TYPE_SINGLE_SV		2
#define PAYLOAD_TYPE_TUNNEL			3
	ST_UINT32 *pEncLen,	    /*pointer to where the length of the encoded buffer is to be stored. */
	ST_UINT32 *SPDUnum	    /*pointer to the SPDU number to be used. Value will be incremented if encode succeeds*/
      );		



#define SIZEOF_IPV4_ADDRESS 4
// the following is used to store keys
#define SIZEOF_IPV6_ADDRESS		16
//this structure is used to store the IP Address, typically destination
#define IP_V4_ADDRESS_TYPE 			0
#define IP_DNS_NAME_TYPE			1					//this option shall not be used for key management.  It is up to the application to convert a DNSName to an
											//actual IP Address.  The define is being provided for completeness and traceability to the standard.
#define IP_V6_ADDRESS_TYPE 			2
#define ETHERNET_MAC_ADDRESS_TYPE 		3					//not for use in 90-5, for use with 62351-6
#define NO_ADDRESS_TYPE				4
typedef struct iec_comm_address{
  ST_UINT8 typeOfAddress;
  ST_UINT8 lenOfAddress;
  ST_UCHAR *pAddress;
}IEC_COMM_ADDRESS;

IEC_COMM_ADDRESS *create_address_structure(int typeOfAddress, char *pAddress);	    //function used to fill in the structure and hide nasty functions
void destroy_address_structure(IEC_COMM_ADDRESS *pAddress);			    //destroys the structure created in the create funciton




typedef struct iec905_key_info{
 struct iec905_key_info *pNext;
 struct iec905_key_info *pPrev;
 struct iec_chain_info *pChain;					//pointer to the chain that it belongs in.
#define SIZE_OF_KEY_ID 4
  unsigned char key_id[SIZE_OF_KEY_ID];
#define KEY_TYPE_AES_128   0					//these are the defines used for type_of_key value, these are the allows values from 90-5
#define KEY_TYPE_AES_256   1
  ST_UINT8  typeOfKey;
  ST_UINT16 keyLen;
  ST_UCHAR *pKey;						//pointer to the actual key
  time_t timeOfInitialUse;
  time_t timeOfexpiration;					//seconds of century at which time the key expires (timeof key acquisition + GDOI expiration time)
#define MAX_ALLOWED_KEY_EXPIRATION_SECONDS 172800  		// 48 hours * 60 minutes * 60 seconds , from 90-5
  ST_UINT32 elapsedTimeUntilExpiration;				//seconds remaining prior to expiration (returned via GDOI)
  struct iec905_msg_cntrl *pUsedBy;				//pointer to what makes use of the key
} IEC905_KEY_INFO;

typedef struct iec905_statistics {			    //structure used for statistic tracking
    unsigned long int TotalTxPktCnt;
    unsigned long int TotalRxPktCnt;
    unsigned long int TotalRxMissingPktCnt;
    unsigned long int TotalRxPktWithBadHMAC;
    } IEC905_STATISTICS;

typedef struct iec905_msg_cntrl{
  struct iec905_msg_cntrl *pPrev;				
  struct iec905_msg_cntrl *pNext;
#define IEC_KEY_USAGE_TYPE_SV		0
#define IEC_KEY_USAGE_TYPE_GOOSE 	1
#define IEC_KEY_USAGE_TYPE_TUNNEL 	2
#define MAX_IEC_KEY_USAGE_TYPE		2
  ST_UINT8 keyUsageType;
  IEC_COMM_ADDRESS keyAddress;				      //value for the destination (tx or subscription)
  IEC_COMM_ADDRESS srcKeyAddress;			      //value for the src address for IGMPV3 (subscription only)
  ST_UINT8 dataSetRefLen;
  ST_UCHAR *pDataSetRef;
  IEC905_KEY_INFO *pCurrentKey;
  IEC905_KEY_INFO *pNextKey;
  ST_UINT32 kdcRefNum;
  void *pKDCReserved;
#define IEC_ALLOW_NO_HASH	      0			      //is the default and won't declare a tamper if PDU is received with no hash
#define IEC_REQUIRE_HASH	      1			      //requires a HASH, if not present, then there has been a tamper
  int hashExpected;
  int requestedUpdate;					      //used to indicate that the user has been notified that the keys need to be updated (internal use only).
  unsigned long spdu_num_tx;				      //used to control the SPDU number on encode
  unsigned long last_spdu_num_rxd;			      //last spdu number received with valid decode
  IEC905_STATISTICS stats;
} IEC905_MSG_CNTRL;	

extern IEC_90_5_RX * iec905_create_dec_info( );
extern int iec905_sess_dec( IEC_90_5_RX *rx_ptr, struct iec_comm_address *pDestAddress);		/* pointer to a user allocated structure */
extern int iec905_destroy_dec_info( IEC_90_5_RX * stuff_to_free);
extern ST_UCHAR *udp_tunnel_dec (ST_UCHAR *udp_pdu_ptr, ST_INT udp_pdu_len, ST_UCHAR *dst_mac, ST_UINT16 *tci_ptr, ST_UINT *dec_len_ptr);
extern ST_UCHAR *usr_encrypt_payloads( IEC905_SESS_PDU_HDR_INFO *hdr, 	ST_UCHAR *unencrypted_buffer, 	ST_UINT32 *payload_len );
extern ST_BOOLEAN usr_create_HMAC( IEC905_SESS_PDU_HDR_INFO *hdr, ST_UCHAR *pData, ST_UINT32 DataLen, ST_UCHAR *pOutHash, ST_UINT32 *enc_len_ptr );
extern ST_BOOLEAN usr_compare_HMAC( IEC905_SESS_PDU_HDR_INFO *hdr, ST_UCHAR *pData, ST_UINT32 DataLen, ST_UCHAR *pCmpHash);


void iec905_init_key_storage();	  //initializes the key storage for the stack
IEC905_MSG_CNTRL *iec905_create_msg_cntrl_rx( ST_UINT8 usageType, IEC_COMM_ADDRESS *pAddress, IEC_COMM_ADDRESS *pSrcAddress, ST_CHAR *pDatSetRef);	//used to allocate key storage and put it into the appropriate chain
IEC905_MSG_CNTRL *iec905_create_msg_cntrl_tx( ST_UINT8 usageType, IEC_COMM_ADDRESS *pAddress, IEC_COMM_ADDRESS *pSrcAddress, ST_CHAR *pDatSetRef);
ST_BOOLEAN iec905_destroy_msg_cntrl (IEC905_MSG_CNTRL *pKeyRoot);	  //used to destroy and recover allocated resources and deletes any associated keys
IEC905_KEY_INFO * iec905_add_current_key( IEC905_MSG_CNTRL *pKeyRoot, ST_UINT8 typeOfAlg,ST_UINT16 key_len, ST_UCHAR *pKey, ST_UCHAR *pKeyID, ST_UINT32 time_remaining);
IEC905_KEY_INFO * iec905_add_next_key( IEC905_MSG_CNTRL *pKeyRoot, ST_UINT8 typeOfAlg,ST_UINT16 key_len, ST_UCHAR *pKey, ST_UCHAR *pKeyID,ST_UINT32 time_remaining);
IEC905_MSG_CNTRL *iec905_find_rxd_msg_cntrl(ST_UCHAR pduSI,  IEC_COMM_ADDRESS *pDestAddress , ST_UCHAR *pKeyID);



void iec905_getStats(IEC905_STATISTICS *pRetStatistics, int resetStats);


int iec905_manage_keys(IEC905_MSG_CNTRL *pKeyRoot);
ST_BOOLEAN iec905_destroy_key (IEC905_MSG_CNTRL *pKeyRoot)	;


//IGMP3 functions -- located in udp_rx.c
IEC905_MSG_CNTRL *iec905_igmpv3_group_enroll(ST_UINT8 usageType,  ST_UINT8 typeOfAddress, char *pMultiAddress, char *pSrcAddress, char *pDatSetRef, SOCKET pReceivingSocket);
IEC905_MSG_CNTRL *iec905_igmpv3_group_destroy(IEC905_MSG_CNTRL *pPayload,SOCKET pReceivingSocket);



/**************************************************/
/*  The following are functions found in the	  */
/*  udp_tx.c module				  */
typedef struct ethertype_8021Q {
  USHORT  priority;	  //priority specification
  USHORT  vlanID;	  //vlanID Setting
}ETHERTYPE_8021Q;

#define MAX_NUM_TC_INTERFACES_SUPPORTED 20   
#define MAX_PEAK_RATE 1000000		    //bits per second used in Traffic FlowSpec
#define MAX_TOKEN_RATE 100000		    //token rate for sending TC.

typedef struct iecsocket_info{
  HANDLE FlowHandle;
  HANDLE FilterHandle;
  SOCKET  SendingSocket;
}IECSOCKET_TX_INFO;

#define MAX_KEY_SIZE_TO_GENERATE (2048/8)	    //2048 bits divide by 8 gives the number of bytes required.
ST_UCHAR *generate_symmetric_key(int algorithmID, int size);

//*********************************Functions in iec90_5_udp_rx.c *************************************
//udp/socket functions
int iec905_ip_init();			      //used to initialize the ip socket interface, MUST BE CALLED before any other socket activity
#define IEC_61850_90_5_UDP_RX_PORT 102		      //port specified for use by 90-5
int iec905_rx_init(SOCKET *pReceivingSocket,unsigned long UDPSckBufSize);    //function used to initialize the UDP RX port used for 90-5
void iec905_close_socket(ReceivingSocket);

int iec905_tx_pdu_init();
int iec905_tx_pdu_open_socket_with_priorities(IECSOCKET_TX_INFO ** pSocketInfo, char *pInterfaceName, ETHERTYPE_8021Q *p8021Q, USHORT *pIP_TOS_DSCP, IEC_COMM_ADDRESS *pDestAddress);
int iec905_tx_pdu_close_socket (IECSOCKET_TX_INFO *SendingInfo);


#define UDP_HDR_SIZE_IPV6 16						//V6 is the worst case for pseudo HDR size
#define MAX_ALLOWED_UDP_PACKET_LENGTH	(65535 - UDP_HDR_SIZE_IPV6)	//maximum size allowed, in order to avoid Jumbo packets
int iec905_tx_pdu (IECSOCKET_TX_INFO *SendingInfo, IEC_COMM_ADDRESS *pMyAddressInfo, 	ST_INT IPPort,	ST_UCHAR *pEnc,	ST_UINT encLen, IEC905_MSG_CNTRL *pTxStats);




//***********************************************/
//  error code definitions			*/

typedef struct errorTracking{
	int    inUse;				//TRUE if there is a valid errorCode in the structure
	int	errorCode;			//Error that was passed into the ErrorHandler Function
	char * fileName;			//Name of the file that called the ErrorHandler funciton. May be NULL.
	unsigned long lineNumber;		//Number of the line, in the file, that called the ErrorHandler function.
	time_t timeOfError;
}ERROR_TRACKING;

int IEC905_ErrorHandler (int errorCode, char * fileName, unsigned long int lineNumber);   //function to call in case of error
char *IEC905_XlatErrorCode(int errorCode);					      //called to translate and error code to a string
ERROR_TRACKING *IEC905_GetSpecificEntry(unsigned int entryNumber);		      //called to retrieve other errors.  The maximum number is 
										      //determined by MAX_ALLOWED_LAST_ERRORS.  EntryNumber starts
										      //numbering at zero(0).  EntryNumber=0 is the same as GetLastError	
ERROR_TRACKING *IEC905_GetLastError();						      //used to get the last error

#define MAX_ALLOWED_LAST_ERRORS 10		//defines the depth of errors that will be maintained

#define SUCCESS_IEC905				  0
#define FAILURE_IEC905				  -1
#define OK_IEC905				  0
#define NOK_IEC905				  -1
#define INVALID_INPUT_PARAMETER			  -2

#define COMM_RELATED_ERROR_OFFSET		  -100
#define UNABLE_TO_INTIALIZE_IP_INTERFACE	      (COMM_RELATED_ERROR_OFFSET-0)
#define IP_PORT_INCORRECT_IEC905		      (COMM_RELATED_ERROR_OFFSET-1)
#define UNABLE_TO_OPEN_UDP_SOCKET_IEC905	      (COMM_RELATED_ERROR_OFFSET-2)
#define UDP_RX_BIND_FAILED_IEC905		      (COMM_RELATED_ERROR_OFFSET-3)
#define ETHERNET_INTERFACE_NOT_FOUND		      (COMM_RELATED_ERROR_OFFSET-4)
#define ETHERNET_INTERFACE_OPEN_FAILED		      (COMM_RELATED_ERROR_OFFSET-5)
#define UNABLE_TO_CREATE_SENDING_SOCKET		      (COMM_RELATED_ERROR_OFFSET-6)
#define UNABLE_TO_CONVERT_IP_ADDRESS		      (COMM_RELATED_ERROR_OFFSET-7)

#define SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET   -200
#define INVALID_IGMP_GROUP_ENROLL_PARAMETERS_IEC905   (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-1)
#define DUPLICATE_PAYLOAD_DETECTED_IEC905	      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-2)
#define IGMPV3_GROUP_MEMBERSHIP_ENROLL_FAILED_IEC905  (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-3)
#define IGMPV2_GROUP_MEMBERSHIP_ENROLL_FAILED_IEC905  (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-4)
#define INVALID_IGMP_GROUP_UNSUB_PARAMETER_IEC905     (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-5)
#define INVALID_IGMP_GROUP_UNSUBV2_FAILURE_IEC905     (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-6)
#define INVALID_IGMP_GROUP_UNSUBV3_FAILURE_IEC905     (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-7)
#define WINDOWS_FLOW_OBJECT_ALLOCATON_ERROR	      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-8)
#define TC_REGISTRATION_ERROR			      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-9)
//must have administrative priviledges to access the Traffic Control API
#define TC_OPEN_FAILED_CHK_ADMIN_PRIV		      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-10) 
#define TC_INTERFACE_LISTING_FALIED		      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-11)
#define INTERFACE_DIFFSERV_FAILED		      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-12)
#define TC_FLOWADD_FAILED			      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-13)
#define UNABLE_TO_ADD_TCFILTER			      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-14)
#define UNABLE_TO_DELETE_TC_FLOW		      (SUBSCRIPTION_AND_TRAFFIC_CONTROL_OFFSET-15)

#define CRYPTO_API_ERROR_OFFSET		  -300
#define CRYPTO_PROVIDER_OPEN_ERROR		      (CRYPTO_API_ERROR_OFFSET-0)
#define CRYPTO_RND_NUM_GEN_ERROR		      (CRYPTO_API_ERROR_OFFSET-1)
#define GENERAL_CRYPTO_PROBLEM			      (CRYPTO_API_ERROR_OFFSET-2)
#define CRYPTO_BLOCK_LEN_PROBLEM		      (CRYPTO_API_ERROR_OFFSET-3)

#define ENCODE_DECODE_ERROR_OFFSET   -400
#define LARGER_THAN_ALLOWED_PDU			      (ENCODE_DECODE_ERROR_OFFSET -0)
#define MEMORY_ALLOCATION_ERROR			      (ENCODE_DECODE_ERROR_OFFSET -1)
#define DECODE_ERROR_TOO_LARGE			      (ENCODE_DECODE_ERROR_OFFSET -2)
#define DECODE_UNKNOWN_TAG			      (ENCODE_DECODE_ERROR_OFFSET -3)
#define DECODE_ERROR_IN_PAYLOADS		      (ENCODE_DECODE_ERROR_OFFSET -4)
#define IEC_90_5_NO_CLTP  			      (ENCODE_DECODE_ERROR_OFFSET -5)
#define IEC_90_5_TAMPERDECTECT_HDR 		      (ENCODE_DECODE_ERROR_OFFSET -6)
#define	IEC_90_5_INVALID_HDR			      (ENCODE_DECODE_ERROR_OFFSET -7)
#define IEC_90_5_INVALID_HDR_LI 		      (ENCODE_DECODE_ERROR_OFFSET -8)
#define IEC_90_5_TAMPERDECTECT_NO_SIG		      (ENCODE_DECODE_ERROR_OFFSET -9)
#define IEC_90_5UNEXPECTED_PDU_RXD		      (ENCODE_DECODE_ERROR_OFFSET -10)
#define IEC_90_5_DUPLICATE_PACKET_RXD		      (ENCODE_DECODE_ERROR_OFFSET -11)

#define INTERNAL_SECURITY_ERROR_OFFSET -500
#define UNABLE_TO_ADD_KEY			      (INTERNAL_SECURITY_ERROR_OFFSET+0)
#define KEYS_NOK				      (INTERNAL_SECURITY_ERROR_OFFSET -1)	//indicates that neither the primary nor next key are valid
#define PRIME_KEY_OK_NEXT_KEY_NOK 		      (INTERNAL_SECURITY_ERROR_OFFSET -2)	//indicates that the primary key is OK, but there is no next key

//*************  SV ENCODE and DECODE FUNCTIONS And Defines  ************************

//The following are supported Data Types as defined in IEC 61850-9-2 ED.2

enum SV_DATATYPES {
  SV_BOOLEAN,
  SV_INT8,
  SV_INT16,
  SV_INT32,
  SV_INT64,
  SV_INT8U,
  SV_INT16U,
  SV_INT24U,		  //do not use
  SV_INT32U,
  SV_FLOAT32,
  SV_FLOAT64,
  SV_ENUMERATED,
  SV_CODEDENUM,
  SV_OCTETSTRING,
  SV_VISIBLESTRING,
  SV_UNICODESTRING,
  SV_OBJECTNAME,
  SV_OBJECTREFERENCE,
  SV_TIMESTAMP,
  SV_ENTRYTIME
};



typedef struct asdu_enc_info{
  char * pASDU;				//pointer to the beginning of the ASDU;
  ST_UINT16 *pSmpCntValue;		//pointer to where to place the SMPCNT value
  ST_UCHAR *pSmpSynch;			//pointer to the SMPSNYC Value
  char * pASDU_data;			//pointer to the beginning of the ASDU data	
  char * pASDU_TimeStamp;		//pointer to the UTC timestamp;
} ASDU_ENC_INFO;

typedef struct sv_enc_struct {
	ST_UINT16 length_to_send;						//Actual PDU size to send
	unsigned char * pSVbuffer;						//pointer to the allocated buffer
	unsigned int max_num_ASDUs_allocated;
	ST_UINT16 size_of_asdu;
	ST_UINT16 *pPDULen;							//pointer to where the PDU len is located
	ST_UCHAR *pNumASDUVal;							//pointer to where the number of ASDUs is located
	ST_UINT16 *pASDUSeqLen;							//pointer to the sequence length for ASDUs
	ST_UINT16 asdu_data_size;						//size in octets of the ASDU
	ASDU_ENC_INFO asdus[0];								//begining of the ASDU arrays, will be allocated to the max_num_ASDUs;
}SV_ENC_STRUCT;

SV_ENC_STRUCT * initialize_sv_encode_struct (char * pMsvid,		     //pointer to a NULL terminated MSVID value may not be more than 127
				    char * pDatSetRef,		 //pointer to a NULL terminated DataSetReference string, may not be more than 127
				    unsigned int max_num_asdus,		 //maximum number of ASDUs, may not be more than 127
				    ST_UINT32 confrev,
				    ST_UCHAR  smpSync,
				    ST_UINT16 smpRate,
				    ST_UINT16 *smpMod,
				    ST_UINT16 size_of_asdu_data, //size of ASDU data in bytes
#define SV_INCLUDE_REFRESH_TIME	  0x80
#define SV_INCLUDE_SAMPLE_SYNC	  0x40	      //do not used, just provided for backward compatibility with V1
#define SV_INCLUDE_SAMPLE_RATE	  0x20
#define SV_INCLUDE_DATA_SET	  0x10
#define SV_INCLUDE_SECURITY	  0x08	      //ignored, not used
#define SV_INCLUDE_UTC_TIME	  0x04	      //added by 90-5

				   ST_UINT8 sv_optflds
				    );
int sv_enc_update_lengths( SV_ENC_STRUCT *pCntrlStruc,		//pointer the the control structure for the buffer
			   unsigned int num_asdus_to_use	//number of the actual ASDUs that will be used
			  );
int destroy_sv_encode_struct (SV_ENC_STRUCT * pDestroy);
int sv_data_helper_prim(unsigned int dataType, int initial_data_offset, int *ret_len);
int sv_data_helper_calculate_samples_size_prim (unsigned int dataType, int initial_data_offset, int *ret_len, int *total_calc_len);


typedef struct asn1Info{
  ST_UINT16 len;
  unsigned char *pValue;
}SV_ASDUINFO;

typedef struct asdu_dec_info{
  SV_ASDUINFO msvID;
  SV_ASDUINFO datSetRef;
  SV_ASDUINFO smpCnt;
  SV_ASDUINFO confRev;
  SV_ASDUINFO refrTm;
  SV_ASDUINFO smpSynch;
  SV_ASDUINFO smpRate;
  SV_ASDUINFO samples;
  SV_ASDUINFO smpMod;
  SV_ASDUINFO utcTimeStamp;   
} ASDU_DEC_INFO;

typedef struct sv_dec_struct {
	unsigned int num_ASDUs;
	ASDU_DEC_INFO asdus[0];							//begining of the ASDU arrays, will be allocated to the max_num_ASDUs;
}SV_DEC_STRUCT;

SV_DEC_STRUCT * sv_decode (unsigned char * pSVPdu,		//pointer to the SVPDU to be decoded		     
				    ST_UINT16 SVPDULen							//length of the PDU to be decoded
				    );


/****************  functions used to interact with the KDC  ****************/

typedef struct kdc_credentials{		  // a placeholder for what needs to be done eventually.  Anticipate certificate information, but that may change.
  char * certificate_info;
}KDC_CREDENTIALS;

typedef struct kdc_ref{
    void *pReserved;			//reserved for internal use
    ST_UINT32 refNum;			//alternat reference number
    char *pUserRef;			//pointer to the user reference for the KDC Pair
}KDC_REF;


int iec905_destroy_kdc_credential(KDC_CREDENTIALS *pCredential);	  //destroys and frees a created KDC Credential structure
int iec905_init_kdc_interface(KDC_CREDENTIALS *pLocalCredential, int maxNumOfKDCPairs);	  //function is called in order to initialize what needs to be initialized for the KDC interface

KDC_REF *iec905_create_KDC_pair(char *userRef, 
			    IEC_COMM_ADDRESS *pPrimary,	      //addressing information for the primary KDC
			    KDC_CREDENTIALS *pPrimaryCredentials,   //pointer to the primary KDCs credentials that need to be used in phase 1 of GDOI 
			    IEC_COMM_ADDRESS  *pSecondary,	      //addressing information for the secondary KDC
			     KDC_CREDENTIALS *pSecondaryCredentials  //pointer to the secondary KDCs credentials that need to be used in phase 1 of GDOI
			     );
int iec905_destroy_KDC_pair(ST_UINT32 refNum);
int iec905_kdc_bind (IEC905_MSG_CNTRL *pMsgCntrl, KDC_REF *pKDCRef);
int iec905_get_kdc_tx_keys(IEC905_MSG_CNTRL * pMsgCntrl);
int iec905_get_kdc_rx_keys(IEC905_MSG_CNTRL * pMsgCntrl);



//***********************************************************************
//    REQUIRED FUNCTIONS TO BE SUPPLIED BY THE USER
void usr_notify_of_key_updated_needed(IEC905_MSG_CNTRL *pCntrl);	//called if keys need to be updated by the KDC
void usr_notify_of_error(int errorCode,char *fileName,unsigned long int lineNumber);
#endif	/* !IEC_90_5_INCLUDED	*/
