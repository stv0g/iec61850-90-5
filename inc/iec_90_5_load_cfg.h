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