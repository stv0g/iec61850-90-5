/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2011-2011, All Rights Reserved					*/
/*									*/
/* MODULE NAME : iec_90_5_key_store.c						*/
/* PRODUCT(S)  : MMS-EASE Lite						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	Decoder for IEC 61850-90-5 Session/CLTP Protocol				*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*	NONE								*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who			Comments			*/
/* --------  ---  ------   -------------------------------------------	*/
/* 10/04/11  HSF	   Initial revision				*/
/************************************************************************/
#include "glbtypes.h"
#include "sysincs.h"
#include "mem_chk.h"
#include "time.h"
#include <conio.h>
#include "iec_90_5.h"

static ST_UCHAR test_key[] = {
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88,
	0x01,0x33,0x34,0x35,0x36,0x37,0x01,0x33,0x34,0x35,0x36,0x37,0x38,0x66,0x77,0x88};

static ST_VOID unit_test_key_payloads()
{
  #define MAX_PAYLOAD_TEST_ARRAY_SIZE 2000
  IEC905_MSG_CNTRL *test_array[MAX_PAYLOAD_TEST_ARRAY_SIZE];				//array used to test 1000 additions and removals
  int i;
  IEC_COMM_ADDRESS keyAddress;
  char DatSetRef[129];
  IEC905_MSG_CNTRL *pTest;
  IEC905_MSG_CNTRL *pTestRet;
  ST_BOOLEAN result;
  unsigned long key_id=0;
  int ret_val;
  unsigned char ipV6Address[SIZEOF_IPV6_ADDRESS];
 IEC905_KEY_INFO *retKeyValue;
 

  //test for all different being able to be added and removed
  for(i = 0;i<MAX_PAYLOAD_TEST_ARRAY_SIZE;i++)
  {
   ++key_id;
   sprintf(DatSetRef,"SISCO_TEST/DataSet%03d",i);

   keyAddress.lenOfAddress = sizeof(int);
   if(i%2 == 0)
      {
      keyAddress.typeOfAddress = IP_V4_ADDRESS_TYPE;
      keyAddress.lenOfAddress = 4;
      keyAddress.pAddress = (ST_UCHAR *)&i;
      }
   else
     { 
     keyAddress.typeOfAddress = IP_V6_ADDRESS_TYPE;
     keyAddress.lenOfAddress = SIZEOF_IPV6_ADDRESS;
     memcpy(ipV6Address,(char *)&i, sizeof(unsigned long));
     keyAddress.pAddress = ipV6Address;
     }
   test_array[i] = iec905_create_msg_cntrl_tx( IEC_KEY_USAGE_TYPE_GOOSE, &keyAddress,NULL, DatSetRef);
   if(test_array[i]==NULL)
     {
     //then we have an error
      printf("Error Payload Test - Phase 1\n");
      return; 
     }
    if((retKeyValue=iec905_add_current_key(test_array[i],KEY_TYPE_AES_128 ,sizeof(test_key),test_key,(unsigned char *)&key_id,1))==NULL)
	printf("Add Key Error\n");
    ++key_id;
     if((retKeyValue=iec905_add_next_key(test_array[i],KEY_TYPE_AES_128 ,sizeof(test_key),test_key,(unsigned char *)&key_id,1))==NULL)
      printf("Add Next Key Error\n");
  }
  


 
//test the management function

  Sleep(1100);
  for(i = 0;i<MAX_PAYLOAD_TEST_ARRAY_SIZE;i++)
    {
    ret_val = iec905_manage_keys(test_array[i]);
    }


  for(i = 0;i<MAX_PAYLOAD_TEST_ARRAY_SIZE;i++)
    {
    if(ret_val = iec905_manage_keys(test_array[i])==SUCCESS_IEC905)
            printf("Error Payload Test - Phase 1b\n");
    }


 //now that we have the array, try to add something in the middle and make sure new payloads don't get created

  pTest = test_array[500];
  pTestRet = iec905_create_msg_cntrl_tx( pTest->keyUsageType, &pTest->keyAddress, NULL,pTest->pDataSetRef);
  if(pTest != pTestRet)
    printf("Error Payload Test - Phase 2a\n");
  else
    {
    ++key_id;
    iec905_add_current_key(pTest,KEY_TYPE_AES_128 ,sizeof(test_key),test_key,(unsigned char *)&key_id,20); //test for adding an existing key
    }

  pTest = test_array[501];
  pTestRet = iec905_create_msg_cntrl_tx( pTest->keyUsageType, &pTest->keyAddress, NULL,pTest->pDataSetRef);
  if(pTest != pTestRet)
    printf("Error Payload Test - Phase 2b\n");
 

  //now want to delete from the rear of the array  (Phase 3)
  for(i = 0;i<MAX_PAYLOAD_TEST_ARRAY_SIZE;i++)
    {
      pTest = test_array[MAX_PAYLOAD_TEST_ARRAY_SIZE-i-1];
      if((result = iec905_destroy_msg_cntrl (pTest))==FALSE)
	printf("Error Payload Test - Phase 3\n");
      test_array[MAX_PAYLOAD_TEST_ARRAY_SIZE-i-1]=NULL;
    }


  //now want to test deleting from the beginning (Phase 4)
#if 0
  for(i = 0;i<MAX_PAYLOAD_TEST_ARRAY_SIZE;i++)
  {
   sprintf(DatSetRef,"SISCO_TEST/DataSet%03d",i);
   keyAddress.pAddress = (ST_UCHAR *)&i;
   keyAddress.lenOfAddress = sizeof(int);
   if(i%2 == 0)
      keyAddress.typeOfAddress = IP_V4_ADDRESS_TYPE;
   else
     keyAddress.typeOfAddress = IP_V6_ADDRESS_TYPE;
    keyAddress.lenOfAddress =4;
    keyAddress.pAddress = (char *)&key_id;
   test_array[i] = create_KeyPayload_tx( IEC_KEY_USAGE_TYPE_GOOSE, &keyAddress, NULL, DatSetRef);	
   if(test_array[i]==NULL)
     {
     //then we have an error
      printf("Error Payload Test - Phase 4a\n");
      return; 
     }
  }

  for(i = 0;i<MAX_PAYLOAD_TEST_ARRAY_SIZE;i++)
    {
      pTest = test_array[i];
      if((result = iec905_destroy_msg_cntrl (pTest))==FALSE)
	printf("Error Payload Test - Phase 4b\n");
      test_array[i] = NULL;
    }
#endif

}

ST_VOID repeat_unit_test_key_payloads(int num_repeats)
{
    int i;
    for(i=0; i<num_repeats;i++)
      {
      unit_test_key_payloads();
      printf("Completed Key Payload test iteration = %05d\n",i);
      }
}

//Now unit test code for IGMP stuff

ST_VOID unit_test_igmp(SOCKET rxSocket)
{
int i=1;
int j;
char multiAddress[32];
char srcAddress[32];
char dataSetRef[129];
IEC905_MSG_CNTRL *ret[254];
   

//test IGMP V3 
    sprintf(multiAddress,"224.0.0.%d",i);
    sprintf(srcAddress,"192.168.1.%d",i);
    sprintf(dataSetRef,"SISCO/DataSEt%03d",i);
    ret[0] =iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,IP_V4_ADDRESS_TYPE,multiAddress,srcAddress,dataSetRef,rxSocket);
    if(ret[0]==NULL)
      printf("Error IGMP Key test phase 1a");      

     ret[0]=iec905_igmpv3_group_destroy(ret[0],rxSocket);
     if(ret[0]!=NULL)
      printf("Error IGMP Key test phase 1b");


//test IGMP V2
    ret[0] =iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,IP_V4_ADDRESS_TYPE,multiAddress,NULL,dataSetRef,rxSocket);
    if(ret[0]==NULL)
      printf("Error IGMP Key test phase 2a");      

     ret[0]=iec905_igmpv3_group_destroy(ret[0], rxSocket);
     if(ret[0]!=NULL)
      printf("Error IGMP Key test phase 2b");

//multiple IGMP  V3 enrolls 
    for(j=0;j<254;j++)
    {
      sprintf(multiAddress,"224.0.0.%d",j);
      sprintf(srcAddress,"192.168.1.%d",j);
      sprintf(dataSetRef,"SISCO/DataSEt%03d",j);
      ret[j] =iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,IP_V4_ADDRESS_TYPE,multiAddress,srcAddress,dataSetRef,rxSocket);
      if(ret[j]==NULL)
        printf("Error IGMP Key test phase 3a");    
    }

//Now multiple destroys
    for(j=0;j<254;j++)
    {
      ret[j]=iec905_igmpv3_group_destroy(ret[j], rxSocket);
      if(ret[j]!=NULL)
        printf("Error IGMP Key test phase 3b");    
    }

//now do the same test with IGMP V2
    for(j=0;j<254;j++)
    {
      sprintf(multiAddress,"224.0.0.%d",j);
      sprintf(dataSetRef,"SISCO/DataSEt%03d",j);
      ret[j] =iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,IP_V4_ADDRESS_TYPE,multiAddress,NULL,dataSetRef,rxSocket);
      if(ret[j]==NULL)
        printf("Error IGMP Key test phase 4a");    
    }

//Now multiple destroys
    for(j=0;j<254;j++)
    {
      ret[j]=iec905_igmpv3_group_destroy(ret[j], rxSocket);
      if(ret[j]!=NULL)
        printf("Error IGMP Key test phase 4b");    
    }

//now do 1/2 of V3 and V2
    for(j=0;j<254;j++)
    {
      sprintf(multiAddress,"224.0.0.%d",j);
      sprintf(srcAddress,"192.168.1.%d",j);
      sprintf(dataSetRef,"SISCO/DataSEt%03d",j);
      if(j%2!=0)
        ret[j] =iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,IP_V4_ADDRESS_TYPE,multiAddress,NULL,dataSetRef,rxSocket);
      else
        ret[j] =iec905_igmpv3_group_enroll(IEC_KEY_USAGE_TYPE_SV,IP_V4_ADDRESS_TYPE,multiAddress,srcAddress,dataSetRef,rxSocket);
      if(ret[j]==NULL)
        printf("Error IGMP Key test phase 5a");    
    }

    for(j=0;j<254;j++)
    {
      ret[j]=iec905_igmpv3_group_destroy(ret[j],rxSocket);
      if(ret[j]!=NULL)
        printf("Error IGMP Key test phase 5b");    
    }

}


ST_VOID repeat_unit_test_igmp(int num_repeats, SOCKET rxSocket)
{
    int i;
    for(i=0; i<num_repeats;i++)
      {
      unit_test_igmp(rxSocket);
      printf("Completed IGMP test iteration = %05d\n",i);
      }
}
