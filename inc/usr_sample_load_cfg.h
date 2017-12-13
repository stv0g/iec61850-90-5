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
/* MODULE NAME : usr_sample_load_cfg.h					    */
/*									    */
/* MODULE DESCRIPTION :							    */
/*	Structures and functions declarations for loading configuration	    */
/*	file								    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 12/01/11  HSF	   Initial revision				    */
/****************************************************************************/
#define MAX_ALLOWED_VALUE_SIZE 128
typedef struct subscriber_address{
    char subAddress[MAX_ALLOWED_VALUE_SIZE];
    char srcAddress[MAX_ALLOWED_VALUE_SIZE];
    char dataSetRef[MAX_ALLOWED_VALUE_SIZE];
    int usage;		//SV, GOOSE, or Tunnel
    void *pRxdCntrl;
}SUBSCRIBER_ADDRESS;

typedef struct startup_cfg{
#define INTERFACE_ID_TOKEN_STRING "InterfaceID"

char interaceID[MAX_ALLOWED_VALUE_SIZE];					//pointer to an allocated interface ID string value
#define DEST_IP_ADDRESS_V4_SMV "SMVIPV4Pub"
char destIPAddressSMV[MAX_ALLOWED_VALUE_SIZE];
#define DEST_IP_ADDRESS_V4_GOOSE "GOOSEIPV4Pub"
char destIPAddressGOOSE[MAX_ALLOWED_VALUE_SIZE];
#define DEST_IP_ADDRESS_V4_TUNNEL "TUNNELIPV4Pub"
char destIPAddressTunnell[MAX_ALLOWED_VALUE_SIZE];
#define MAX_ALLOWED_SUBSCRIPTIONS 10
int numCfgSubscriptions;
#define STAT_RESET_TIME_MINUTES "StatResetMinutes"
unsigned long statResetTime;			    //in seconds
#define UDP_SCK_BUF_SETTING "UDPBufSizeK"
unsigned long updScktBufSize;
#define TRANSMISSION_INTERVAL_SETTING "TransIntMsec"
unsigned int transmissionIntervalMsec;
#define LOG_INTERVAL_SETTING "LogIntMin"
unsigned long logIntervalSeconds;
#define STARTUP_DELAY_SETTING "ThreadStrtDelayMsec"
int threadStrtDelay;
#define SUBSCRIPTION_PAIR "IGMPv3Pair"
#define SUBSCRIPTION_PAIR_V2 "IGMPv2Dest"
SUBSCRIBER_ADDRESS subscriptions[MAX_ALLOWED_SUBSCRIPTIONS];
}STARTUP_CFG;

int startup_cfg_read (
	char *startup_cfg_filename,	/* usually "startup.cfg"	*/
	STARTUP_CFG *startup_cfg);