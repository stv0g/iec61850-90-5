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
/* MODULE NAME : iec_crypto_windows.c					    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Windows API interface to generate HMAC				    */
/*	Provides dummy functions for encryption				    */
/*									    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 08/15/12  HSF	   Fixed missing HASH length			    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"  

//#define USE_WINCRYPT			/*use this define in order to use WINCRYPT	- NO AES-GMAC*/
#define USE_BCRYPT			/*use this define in order to use BCRYPT	*/


#ifdef USE_WINCRYPT
#include "wincrypt.h"
#endif

#ifdef USE_BCRYPT
#include <windows.h> 
#include <winerror.h> 
#include "bcrypt.h"
#include <sal.h>

#ifndef BCRYPT_AES_GMAC_ALGORITHM
#define BCRYPT_AES_GMAC_ALGORITHM L"AES-GMAC"			/*define the algorithm even though the include may be out of date */
#endif

#ifndef BCRYPT_CHAIN_MODE_GCM
#define BCRYPT_CHAIN_MODE_GCM      L"ChainingModeGCM"
#endif


static const  
BYTE Message[] =  
{ 
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 
}; 
 
static const  
BYTE HmacKey[] = 
{ 
    0x1b, 0x20, 0x5a, 0x9e, 0x2b, 0xe3, 0xfe, 0x85,  
    0x9c, 0x37, 0xf1, 0xaf, 0xfe, 0x81, 0x88, 0x92,  
    0x63, 0x27, 0x38, 0x61, 
}; 
#endif



#ifdef USE_BCRYPT

static ST_UCHAR * create_90_5_HMAC(IEC905_SESS_PDU_HDR_INFO *hdr, ST_UCHAR *pData, ST_UINT32 DataLen)
{
	//--------------------------------------------------------------------
//  Declare variables.

//--------------------------------------------------------------------
//  Declare variables.

// Declare variables.
//
// hProv:           Handle to a cryptographic service provider (CSP). 
//                  This example retrieves the default provider for  
//                  the PROV_RSA_FULL provider type.  
// hHash:           Handle to the hash object needed to create a hash.
// hKey:            Handle to a symmetric key. This example creates a 
//                  key for the RC4 algorithm.
// hHmacHash:       Handle to an HMAC hash.
// pbHash:          Pointer to the hash.
// dwDataLen:       Length, in bytes, of the hash.
// Data1:           Password string used to create a symmetric key.
// Data2:           Message string to be hashed.
// HmacInfo:        Instance of an HMAC_INFO structure that contains 
//                  information about the HMAC hash.
// 
 
    NTSTATUS    Status; 
     
    BCRYPT_ALG_HANDLE   AlgHandle = NULL; 
    BCRYPT_HASH_HANDLE  HashHandle = NULL; 
     
    PBYTE   Hash = NULL; 
    DWORD   HashLength = 0; 
    DWORD   ResultLength = 0; 
    DWORD   IterationCount = 2; //Perform HMAC computation twice 
    BOOL    IsReusable = TRUE; 
    DWORD   LoopCounter = 0; 

//--------------------------------------------------------------------
// Zero the HMAC_INFO structure and use the SHA1 algorithm for
// hashing.

if((hdr->hmacAlg ==HMAC_ALG_None) || (hdr->hmacAlg>HMAC_ALG_AES_GMAC_128))		/*no HASH being requested or unknown*/
  return(NULL);


if(hdr->hmacAlg<=HMAC_ALG_SHA_256_256)			/* if the signature is for SHA-256		*/	
  { 
    // Open an handle for SHA-256
 
    Status = BCryptOpenAlgorithmProvider( &AlgHandle,                 // Alg Handle pointer 
                                        BCRYPT_SHA256_ALGORITHM,      // Cryptographic Algorithm name (null terminated unicode string)  
                                        NULL,                       // Provider name; if null, the default provider is loaded 
                                        BCRYPT_ALG_HANDLE_HMAC_FLAG); // Flags, Peform HMAC  
    if( ERROR_SUCCESS != Status ) 
    { 
        return(NULL);
    } 
  }
else		/* then we have AES GMAC			*/
 {
    // Open an algorithm handle for AES-GMAC 

    Status = BCryptOpenAlgorithmProvider( 
                                        &AlgHandle,                 // Alg Handle pointer 
                                        BCRYPT_AES_GMAC_ALGORITHM,      // Cryptographic Algorithm name (null terminated unicode string)  
                                        NULL,                       // Provider name; if null, the default provider is loaded 
                                        BCRYPT_ALG_HANDLE_HMAC_FLAG); // Flags, Peform HMAC  

    if( ERROR_SUCCESS != Status ) 
    { 
        return(NULL);
    } 
 }
     
   // Obtain the length of the hash 
 
    Status = BCryptGetProperty( 
                                        AlgHandle,                  // Handle to a CNG object 
                                        BCRYPT_HASH_LENGTH,         // Property name (null terminated unicode string) 
                                        (PBYTE)&HashLength,         // Address of the output buffer which recieves the property value 
                                        sizeof(HashLength),         // Size of the buffer in bytes 
                                        &ResultLength,              // Number of bytes that were copied into the buffer 
                                        0);                         // Flags 
    if( ERROR_SUCCESS != Status ) 
    { 
        BCryptCloseAlgorithmProvider( 
                                        AlgHandle,                  // Handle to the algorithm provider which needs to be closed 
                                        0);                         // Flags 
        return(NULL);
    } 
 
 
    // 
    // Allocate the hash buffer on the heap 
    // 
 
    Hash = (PBYTE)calloc(1, HashLength); 
    if( NULL == Hash ) 
    { 
        BCryptCloseAlgorithmProvider( 
                                        AlgHandle,                  // Handle to the algorithm provider which needs to be closed 
                                        0);                         // Flags 
        return(NULL);
    } 
 
    // 
    // Create a hash 
    // BCRYPT_REUSABLE_HASH_FLAG ensures the reusable hash implementation is used for the specified algorithm. 
    // If the provider does not support it, the entire sequence(BCryptCreateHash, BCryptHashData, BCryptFinishHash and BCryptDestroyHash) should be followed for every HMAC/Hash computation 
    // 
 
	Status = FALSE;
#ifdef  BCRYPT_HASH_REUSABLE_FLAG				/*some older includes don't allow the REUSABLE Flags		*/
    Status = BCryptCreateHash( 
                                        AlgHandle,                  // Handle to an algorithm provider                  
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object 
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer.  
                                        0,                          // Size of the buffer in bytes 
                                        (PBYTE)hdr->key_ptr,        // A pointer to a key to use for the hash or MAC 
                                        hdr->key_len,				// Size of the key in bytes 
                                        BCRYPT_HASH_REUSABLE_FLAG); // Flags  


    if( ERROR_SUCCESS != Status ) 
    { 
#endif
        // 
        // The provider does not support reusable hash implementation 
        // 
 
        IsReusable = FALSE; 
#if 0
        Status = BCryptCreateHash( 
                                        AlgHandle,                  // Handle to an algorithm provider                  
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object 
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer.  
                                        0,                          // Size of the buffer in bytes 
                                        (PBYTE)hdr->key_ptr,        // A pointer to a key to use for the hash or MAC 
                                        hdr->key_len,		    // Size of the key in bytes 
                                        0);                         // Flags 
#endif

        Status = BCryptCreateHash( 
                                        AlgHandle,                  // Handle to an algorithm provider                  
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object 
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer.  
                                        0,                          // Size of the buffer in bytes 
                                        (PBYTE)hdr->pKey,        // A pointer to a key to use for the hash or MAC 
                                        hdr->keyLen,		    // Size of the key in bytes 
                                        0);                         // Flags 

        if( ERROR_SUCCESS != Status ) 
        { 
          BCryptCloseAlgorithmProvider( 
                                        AlgHandle,                  // Handle to the algorithm provider which needs to be closed 
                                        0);                         // Flags 
	    free(Hash);
            return(NULL); 
        }     
#ifdef BCRYPT_HASH_REUSABLE_FLAG
    }   
#endif
 
    do 
    {  
        // 
        // Hash the message(s) 
        // More than one message can be hashed by calling BCryptHashData  
        // 
 
        Status = BCryptHashData( 
                                            HashHandle,             // Handle to the hash or MAC object 
                                            (PBYTE)pData,         // A pointer to a buffer that contains the data to hash 
                                            DataLen,			   // Size of the buffer in bytes 
                                            0);                     // Flags 
        if( ERROR_SUCCESS != Status ) 
        { 
            free(Hash); 
			BCryptDestroyHash(HashHandle);    
			BCryptCloseAlgorithmProvider(AlgHandle, 0);
            return(NULL); 
        } 
 
     
        // 
        // Close the hash 
        // 
 
        Status = BCryptFinishHash( 
                                            HashHandle,             // Handle to the hash or MAC object 
                                            Hash,                   // A pointer to a buffer that receives the hash or MAC value 
                                            HashLength,             // Size of the buffer in bytes 
                                            0);                     // Flags 
        if( ERROR_SUCCESS != Status ) 
        { 
            free(Hash); 
			BCryptDestroyHash(HashHandle);    
			BCryptCloseAlgorithmProvider(AlgHandle, 0);
            return(NULL); 
        } 
 
        if( !IsReusable ) 
        { 
            Status = BCryptDestroyHash(HashHandle);                 // Handle to hash/MAC object which needs to be destroyed 
            if( ERROR_SUCCESS != Status ) 
            { 
            free(Hash); 
			BCryptCloseAlgorithmProvider(AlgHandle, 0);
            return(NULL); 
            } 
            HashHandle = NULL; 

#if 0 
            Status = BCryptCreateHash( 
                                        AlgHandle,                  // Handle to an algorithm provider                  
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object 
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer.  
                                        0,                          // Size of the buffer in bytes 
                                        (PBYTE)HmacKey,             // A pointer to a key to use for the hash or MAC 
                                        sizeof(HmacKey),            // Size of the key in bytes 
                                        0);                         // Flags 
#endif

            Status = BCryptCreateHash( 
                                        AlgHandle,                  // Handle to an algorithm provider                  
                                        &HashHandle,                // A pointer to a hash handle - can be a hash or hmac object 
                                        NULL,                       // Pointer to the buffer that recieves the hash/hmac object; NULL implies provider will allocate and free the buffer.  
                                        0,                          // Size of the buffer in bytes 
                                        (PBYTE)hdr->pKey,        // A pointer to a key to use for the hash or MAC 
                                        hdr->keyLen,		    // Size of the key in bytes 
                                        0);                         // Flags 

            if( ERROR_SUCCESS != Status ) 
            { 
              free(Hash); 
			  BCryptDestroyHash(HashHandle);    
			  BCryptCloseAlgorithmProvider(AlgHandle, 0);
              return(NULL); 
            }   
        } 
 
        LoopCounter++; 
 
    } while( LoopCounter < IterationCount ); 
 
 
    Status = ERROR_SUCCESS; 
 
   
    if( NULL != HashHandle )     
    { 
        BCryptDestroyHash(HashHandle);                              // Handle to hash/MAC object which needs to be destroyed 
    } 
 
    if( NULL != AlgHandle ) 
    { 
        BCryptCloseAlgorithmProvider( 
                                        AlgHandle,                  // Handle to the algorithm provider which needs to be closed 
                                        0);                         // Flags 
	} 
 
    return (Hash); 
}


#endif				/*end of SHA256 HMAC Using Bcrypt */

ST_BOOLEAN usr_create_HMAC( IEC905_SESS_PDU_HDR_INFO *hdr, ST_UCHAR *pData, ST_UINT32 DataLen, ST_UCHAR *pOutHash, ST_UINT32 *enc_len_ptr)
{
	ST_UCHAR *pCreatedHash;

	if (hdr->hmacAlg == HMAC_ALG_None)
		return(FALSE);							/*HMAC not created			*/

	pCreatedHash = create_90_5_HMAC(hdr, pData, DataLen);			/*create the HASH	*/
	if(pCreatedHash == NULL)
		return(FALSE);
	else
	{
		/*have a returned hash and now need to truncate appropriately	*/
		switch(hdr->hmacAlg)
		{
		case HMAC_ALG_SHA_256_80:
			*pOutHash++=10;
			memcpy(pOutHash,pCreatedHash,10);
			*enc_len_ptr = *enc_len_ptr + 12;
			break;

		case HMAC_ALG_AES_GMAC_128:
		case HMAC_ALG_SHA_256_128:
			*pOutHash++=16;
			memcpy(pOutHash,pCreatedHash,16);
			*enc_len_ptr = *enc_len_ptr + 18;
			break;

		case HMAC_ALG_SHA_256_256:
			*pOutHash++=32;
			memcpy(pOutHash,pCreatedHash,32);
			*enc_len_ptr = *enc_len_ptr + 34;
			break;

		case HMAC_ALG_AES_GMAC_64:
			*pOutHash++=8;
			memcpy(pOutHash,pCreatedHash,8);
			*enc_len_ptr = *enc_len_ptr + 11;
			break;
		}

		free(pCreatedHash);
	}
  return(SUCCESS_IEC905);
}

ST_BOOLEAN usr_compare_HMAC( IEC905_SESS_PDU_HDR_INFO *hdr, ST_UCHAR *pData, ST_UINT32 DataLen, ST_UCHAR *pCmpHash)
{
#define MAX_SIZE_HASH_STORAGE 34
	ST_UCHAR hashStorage[MAX_SIZE_HASH_STORAGE];
	ST_UINT cmpResult;
	ST_UINT32 hmac_len;

	memset(hashStorage,0,MAX_SIZE_HASH_STORAGE);
	if(usr_create_HMAC( hdr, pData, DataLen, hashStorage, &hmac_len)!=SUCCESS_IEC905)			/*create a HASH so can compare against what was recieved */
		return(FALSE);

	/*have a returned hash and now need to compare the truncated value*/
	switch(hdr->hmacAlg)
		{
		case HMAC_ALG_SHA_256_80:
			cmpResult = memcmp(hashStorage,pCmpHash,11);
			break;

		case HMAC_ALG_AES_GMAC_128:
		case HMAC_ALG_SHA_256_128:
			cmpResult = memcmp(hashStorage,pCmpHash,17);
			break;

		case HMAC_ALG_SHA_256_256:
			cmpResult = memcmp(hashStorage,pCmpHash,33);
			break;

		case HMAC_ALG_AES_GMAC_64:
			cmpResult = memcmp(hashStorage,pCmpHash,9);
			break;

		default:
		    cmpResult=1;
		    break;
		}


	if(cmpResult==0)
		return(SUCCESS_IEC905);
	else
		return(FAILURE_IEC905);
}


/* the following is a user supplied encryption function	this is a dummy for now		*/
ST_UCHAR *usr_encrypt_payloads(
    IEC905_SESS_PDU_HDR_INFO *hdr,
	ST_UCHAR *unencrypted_buffer,
	ST_UINT32 *payload_len
	)
{
ST_UCHAR *enc_buf_ptr;
ST_UINT32 result_enc_len = *payload_len;

	/* for now, just take the uncrypted buffer and put it into another buffer */
	/* this buffer will be freed by the 90-5 encoder						  */
	enc_buf_ptr = calloc (1,*payload_len);

	/* the following will need to be replaced by an actual encryption call */
	memcpy(enc_buf_ptr,unencrypted_buffer,*payload_len);

	/*after the encryption into the buffer, the payload_length must be updated */
	/* with the resulting encrypted length										*/
	*payload_len = result_enc_len;

	return(enc_buf_ptr);

	
}

/* the following is a user supplied encryption function	this is a dummy for now		*/
ST_UCHAR *usr_decrypt_payloads(
    IEC905_SESS_PDU_HDR_INFO *hdr,
	ST_UCHAR *encrypted_buffer,
	ST_UINT32 *payload_len,
	IEC905_MSG_CNTRL *pKeyInput
	)
{
ST_UCHAR *enc_buf_ptr;
ST_UINT32 result_enc_len = *payload_len;

	/* for now, just take the uncrypted buffer and put it into another buffer */
	/* this buffer will be freed by the 90-5 encoder						  */
	enc_buf_ptr = calloc (1,*payload_len);

	/* the following will need to be replaced by an actual encryption call */
	memcpy(enc_buf_ptr,encrypted_buffer,*payload_len);

	/*after the encryption into the buffer, the payload_length must be updated */
	/* with the resulting encrypted length										*/
	*payload_len = result_enc_len;

	return(enc_buf_ptr);
	
}






  

  
