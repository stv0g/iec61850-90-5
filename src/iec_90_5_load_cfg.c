/************************************************************************/
/* SISCO SOFTWARE MODULE HEADER *****************************************/
/************************************************************************/
/*	(c) Copyright Systems Integration Specialists Company, Inc.,	*/
/*	2004-2004 All Rights Reserved					*/
/*									*/
/* MODULE NAME : startup.c						*/
/* PRODUCT(S)  : MMS-EASE-LITE						*/
/*									*/
/* MODULE DESCRIPTION :							*/
/*	Function to read "startup.cfg" input file.			*/
/*									*/
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				*/
/*			startup_cfg_read				*/
/*									*/
/* MODIFICATION LOG :							*/
/*  Date     Who   Rev     Comments					*/
/* --------  ---  ------   -------------------------------------------  */
/* 08/03/07  JRB    04     Allow only comma or tab as delimiters.	*/
/*			   Check for empty tokens.			*/
/*			   Add extra error log messages.		*/
/* 05/03/05  JRB    03	   Replace strtok with get_next_string to handle*/
/*			   extra delimiters and quoted strings.		*/
/*			   Allow blanks, tabs in ANY string, but it	*/
/*			   MUST be enclosed in quotes.			*/
/* 08/20/04  JRB    02     Fix ReportScanRate input error checking	*/
/* 07/07/04  JRB    01     Initial Revision.				*/
/************************************************************************/
#include "iec_glbtypes.h"
#include "iec_sysincs.h"
#include "iec_90_5.h"
#include "iec_90_5_load_cfg.h"



/************************************************************************/
/*			get_next_string					*/
/* Return pointer to next string in input buffer, separated by		*/
/* delimiters (string may be surrounded by "double quotes").		*/
/* ARGUMENTS:								*/
/*   ptrptr - address of current ptr. Current ptr changed by this funct.*/
/*   delimiters - set of delimiter chars (like strtok or strpbrk).	*/
/* RETURNS:	ptr to string found, NULL on error or end of input string.*/
/************************************************************************/
ST_CHAR *get_next_string (ST_CHAR **ptrptr, ST_CHAR *delimiters)
  {
ST_CHAR *curptr;
ST_CHAR *string;
ST_CHAR *next_delimiter;

  /* If (*ptrptr==NULL), can't do anything. Just return NULL.		*/
  /* May often happen if last call failed or hit end of input buffer.	*/
  if (*ptrptr == NULL)
    string = NULL;
  else
    {
    curptr = *ptrptr;	/* copy to local var for readability	*/
    
    /* if next char is double quote, get up to next double quote	*/
    if (*curptr == '\"')
      {
      curptr++;	/* point after starting quote	*/
      string = curptr;
      curptr = strpbrk (curptr, "\"");	/* find ending quote	*/
      /* NOTE: curptr should now point to ending quote, but	*/
      /*       it may be NULL, if ending quote not found.	*/
      if (curptr)
        {
        *curptr = '\0';	/* replace ending quote with NULL	*/
        curptr++;		/* point after ending quote	*/
        }
      if (curptr)
        {	/* make sure next character is delimiter, then skip it.	*/
        next_delimiter = strpbrk (curptr, delimiters);	/* find next delimiter	*/
        if (next_delimiter != curptr)
          {
          printf("get_next_string ERROR: '%c' after ending quote. Expecting delimiter.\n", *curptr);
          string = NULL;	/* error	*/
          curptr = NULL;	/* error	*/
          }
        else
          curptr++;	/* skip this delimiter	*/
        }
      }
    else if (*curptr == '\0')
      {		/* no more strings	*/
      string = NULL;
      curptr = NULL;
      }
    else
      {
      string = curptr;
      curptr = strpbrk (curptr, delimiters);	/* find next delimiter	*/
      /* NOTE: curptr should now point to ending delimiter, but	*/
      /*       it may be NULL, if expected delimiter not found.	*/
      if (curptr)
        {
        *curptr = '\0';		/* replace delimiter with NULL	*/
        curptr++;		/* point after delimiter	*/
        }
      }

    *ptrptr = curptr;	/* NULL on any error OR end of input string	*/
    }
  return (string);	/* return ptr to string (NULL on any error)	*/ 
  }      

ST_VOID strncpy_safe (char *dest, char *src, int max_len)
  {
  strncpy (dest, src, max_len);
  dest[max_len] = '\0';
  }


/************************************************************************/
/*			startup_cfg_read				*/
/* Reads "startup.cfg" input file & fills in STARTUP_CFG struct.	*/
/* RETURNS:	SD_SUCCESS or error code				*/
/************************************************************************/
int startup_cfg_read (
	char *startup_cfg_filename,	/* usually "startup.cfg"	*/
	STARTUP_CFG *startup_cfg)
  {
FILE *in_fp;
ST_CHAR in_buf[256];	/* buffer for one line read from file	*/
ST_CHAR token_buf[256];	/* copy of "in_buf". Modified by parsing code.	*/
ST_CHAR *curptr;		/* ptr to current position in token_buf	*/
char seps[] = ",\t\n";
ST_INT line_num;		/* number of lines in file	*/
ST_RET retcode = SUCCESS_IEC905;
ST_CHAR *parameter_name;		/* first token on line	*/
ST_CHAR *value;			/* 2nd token	*/
int length;
  memset (startup_cfg, 0, sizeof (STARTUP_CFG));	/* CRITICAL: start with clean struct*/
  startup_cfg->numCfgSubscriptions=0;			//for tracking purposes

  in_fp = fopen (startup_cfg_filename, "r");
  if (in_fp == NULL)
    {
    printf("Error opening input file '%s'\n", startup_cfg_filename);
    return (FAILURE_IEC905);
    }

  /* Read one line at a time from the input file	*/
  line_num = 0;
  while (fgets (in_buf, sizeof(in_buf) - 1, in_fp) != NULL)
    {
    //DEBUG: if last char in "in_buf" is not '\n', then complete line was not read.
    //  Should we try to handle that case?
    line_num++;

    /* Copy the input buffer to "token_buf". This code modifies the 
     * copied buffer (token_buf). Keep input buffer (in_buf) intact.
     */
    strcpy (token_buf, in_buf);

    curptr = token_buf;	/* init "curptr"	*/
    /* First token must be "ParameterName".	*/
    parameter_name = get_next_string (&curptr, seps);


    /* If NULL, this is empty line. If first char is '#', this is comment line.*/
    if (parameter_name == NULL || parameter_name[0] == '#')
      continue;		/* Ignore empty lines & comment lines	*/
    if (parameter_name [0] == '\0')
      {
      /* First token is empty. This is probably empty line.	*/
      /* Ignore this line, but log error if more tokens found. 	*/
      if ((value = get_next_string (&curptr, seps)) != NULL)
        printf("Input ignored because first token is empty at line %d in '%s'. Second token='%s'\n",
                    line_num, startup_cfg_filename, value);
      continue;
      }

    /* Second token must be "Value".	*/
    value          = get_next_string (&curptr, seps);

    if (value && value [0] != '\0')
      {
      if (stricmp (parameter_name, INTERFACE_ID_TOKEN_STRING) == 0)
        strncpy_safe (startup_cfg->interaceID, value, MAX_ALLOWED_VALUE_SIZE);
      else if (stricmp (parameter_name, DEST_IP_ADDRESS_V4_SMV) == 0)
	strncpy_safe (startup_cfg->destIPAddressSMV, value, MAX_ALLOWED_VALUE_SIZE);
      else if (stricmp (parameter_name, DEST_IP_ADDRESS_V4_GOOSE) == 0)
	strncpy_safe (startup_cfg->destIPAddressGOOSE, value, MAX_ALLOWED_VALUE_SIZE);
      else if (stricmp (parameter_name, DEST_IP_ADDRESS_V4_TUNNEL ) == 0)
	strncpy_safe (startup_cfg->destIPAddressTunnell, value, MAX_ALLOWED_VALUE_SIZE);
      else if (stricmp (parameter_name, STAT_RESET_TIME_MINUTES ) == 0)
	startup_cfg->statResetTime = atol(value) * 60;
      else if (stricmp (parameter_name, UDP_SCK_BUF_SETTING ) == 0)
	startup_cfg->updScktBufSize = atoi(value) * 1024;
      else if (stricmp (parameter_name, TRANSMISSION_INTERVAL_SETTING) == 0)
	startup_cfg->transmissionIntervalMsec = atoi(value);
      else if (stricmp (parameter_name, LOG_INTERVAL_SETTING) == 0)
	startup_cfg->logIntervalSeconds = atoi(value)*60;
      else if (stricmp (parameter_name, STARTUP_DELAY_SETTING) == 0)
	startup_cfg->threadStrtDelay = atoi(value);
      else if (stricmp (parameter_name, SUBSCRIPTION_PAIR ) == 0)
	  {
	  if(startup_cfg->numCfgSubscriptions<MAX_ALLOWED_SUBSCRIPTIONS-1)
	    {
	      //then we have room, get the subscription address
	      strncpy_safe (startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].subAddress, value, MAX_ALLOWED_VALUE_SIZE);

	      //now get the src address
	      value          = get_next_string (&curptr, seps);
	      if(value!=NULL)
	        strncpy_safe (startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].srcAddress, value, MAX_ALLOWED_VALUE_SIZE);

	      //now get the dataSetRef
	      value          = get_next_string (&curptr, seps);
	      if(value!=NULL)
	        strncpy_safe (startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].dataSetRef, value, MAX_ALLOWED_VALUE_SIZE);

	      //now get the usage type (SV, GOOSE, TUN)
	      value          = get_next_string (&curptr, seps);
	      if(value!=NULL)
	      {
		if(!strcmp("SV",value))
		  startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].usage = IEC_KEY_USAGE_TYPE_SV;
		else if(!strcmp("GOOSE",value))
		  startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].usage = IEC_KEY_USAGE_TYPE_GOOSE;
		else if(!strcmp("TUN",value))
		  startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].usage = IEC_KEY_USAGE_TYPE_TUNNEL;
		else
		  printf("Error on line %d in file %s, Usage type must be SV, GOOSE, or TUN\n",line_num, startup_cfg_filename);
	      }
	    ++startup_cfg->numCfgSubscriptions;
	    }
	  }
#ifndef _WIN32
      else if (stricmp (parameter_name, SUBSCRIPTION_PAIR_V2 ) == 0)
	  {
	  if(startup_cfg->numCfgSubscriptions<MAX_ALLOWED_SUBSCRIPTIONS-1)
	    {
	      //then we have room, get the subscription address
	      strncpy_safe (startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].subAddress, value, MAX_ALLOWED_VALUE_SIZE);

	      //now get the dataSetRef
	      value          = get_next_string (&curptr, seps);
	      if(value!=NULL)
	        strncpy_safe (startup_cfg->subscriptions[startup_cfg->numCfgSubscriptions].dataSetRef, value, MAX_ALLOWED_VALUE_SIZE);
	   ++startup_cfg->numCfgSubscriptions;
	    }

	  }
#endif
      else
        {
        printf("Unrecognized ParameterName '%s' at line %d in '%s'.\n",
                    parameter_name, line_num, startup_cfg_filename);
        retcode = FAILURE_IEC905;
        }
      }
    else
      {
      printf("Invalid input at line %d in '%s'. Must contain ParameterName and Value.\n", line_num, startup_cfg_filename);
      printf ("%s\n", in_buf);
      retcode = FAILURE_IEC905;
      }

    /* If ANYTHING failed so far, stop looping.	*/
    if (retcode)
      {
      printf ("Error may be caused by extra delimiter in input treated as empty field\n");
      break;	/* get out of loop	*/
      }
    }	/* end main "while" loop	*/

  fclose (in_fp);
  return (retcode);
  }
