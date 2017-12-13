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
/* MODULE NAME : iec_glbtypes.h						    */
/*									    */
/* MODULE DESCRIPTION : 						    */
/*									    */
/* GLOBAL FUNCTIONS DEFINED IN THIS MODULE :				    */
/*									    */
/* MODIFICATION LOG :							    */
/*  Date     Who   Rev			Comments			    */
/* --------  ---  ------   -------------------------------------------	    */
/* 08/11/12  HSF	   Modified header to include Apache License	    */
/* 10/04/11  HSF	   Initial revision				    */
/****************************************************************************/

#ifndef IECGBLTYPES_INCLUDED
#define IECGBLTYPES_INCLUDED

//#include "glbopt.h"	/* Global compiler options.			*/

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/************************************************************************/
/************************************************************************/
/* General purpose defines, same for all platforms			*/

#define IEC_TRUE		1
#define IEC_FALSE	0		
#define IEC_SUCCESS 	0
#define IEC_FAILURE 	1
#define IEC_BIG_ENDIAN		0
#define IEC_LITTLE_ENDIAN	1

/* Define used for 'const' modifier 					*/
/* DEBUG: someday if all code is changed to consistently use IEC_CONST,	*/
/*        this define may be replaced with the following:		*/
/* #define IEC_CONST const						*/
#define IEC_CONST 

/************************************************************************/
/************************************************************************/
/************************************************************************/
/************************************************************************/

/* SYSTEM_SEL defines - bit masked					*/
#define	SYSTEM_SEL_MSOFT	0x0001
#define SYSTEM_SEL_SYS_5	0x0020
#define SYSTEM_SEL_SYS_BSD	0x0040
#define SYSTEM_SEL_SYSVXWORKS	0x0800

/* For backwards compatibility only, do not use. Will be deleted soon.	*/
#if !defined(MSOFT)
#define	MSOFT		SYSTEM_SEL_MSOFT       
#endif
#if !defined(SYS_5)
#define	SYS_5		SYSTEM_SEL_SYS_5       
#endif
#if !defined(SYS_BSD)
#define	SYS_BSD		SYSTEM_SEL_SYS_BSD     
#endif
#if !defined(SYSVXWORKS)
#define	SYSVXWORKS	SYSTEM_SEL_SYSVXWORKS  
#endif


/************************************************************************/
/************************************************************************/
/* SYSTEM and CPU select defines. These are based on built in compiler	*/
/* defines which allow automatic detection of the compiler.		*/
/************************************************************************/

/************************************************************************/
/* WINDOWS 95/NT							*/
/************************************************************************/
#if defined(_WIN32)				/* VC++, 32-Bit		*/

#define IEC_BYTE_ORDER	IEC_LITTLE_ENDIAN
#define SYSTEM_SEL   	SYSTEM_SEL_MSOFT
#define IEC_END_STRUCT	

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8     signed char     	
#define ST_INT16    signed short     	
#define ST_INT32    signed long     	
#define ST_INT64    __int64
#define ST_UINT8    unsigned char     	
#define ST_UINT16   unsigned short    	
#define ST_UINT32   unsigned long    	
#define ST_UINT64   unsigned __int64
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _IECTYPES_DEFINED

#endif



/************************************************************************/
/* VXWORKS - VXWORKS on Motorola 680x0 processor			*/
/************************************************************************/

#if defined(VXWORKS) 

#define IEC_BYTE_ORDER	IEC_BIG_ENDIAN
#define SYSTEM_SEL   	SYSTEM_SEL_SYSVXWORKS
#define IEC_END_STRUCT 

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed long     	
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned long    	
#define ST_BOOLEAN  unsigned char		

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif



/************************************************************************/
/* LINUX SYSTEM								*/
/* OR LYNXOS SYSTEM (same types)					*/
/************************************************************************/
#if defined(linux) || defined(__LYNX)

/* NOTE: this may also work for setting IEC_BYTE_ORDER on other		*/
/*       platforms that use the GNU C Library				*/
#include <endian.h>
#if (__BYTE_ORDER ==__LITTLE_ENDIAN)
  #define IEC_BYTE_ORDER	IEC_LITTLE_ENDIAN
#elif (__BYTE_ORDER ==__BIG_ENDIAN)
  #define IEC_BYTE_ORDER	IEC_BIG_ENDIAN
#else
  #error unsupported byte order
#endif

#define IEC_END_STRUCT 

/* We can tolerate machine-dependent sizes for these types		*/
#define ST_CHAR    char	
#define ST_INT     signed int		
#define ST_LONG    signed long int     	
#define ST_UCHAR   unsigned char	
#define ST_UINT    unsigned int		
#define ST_ULONG   unsigned long     	
#define ST_VOID    void      		
#define ST_DOUBLE  double		
#define ST_FLOAT   float		

/* General purpose return code						*/
#define ST_RET signed int		

/* We need specific sizes for these types				*/
#define ST_INT8   signed char     	
#define ST_INT16  signed short     	
#define ST_INT32  signed int			/* was signed long	*/
#define ST_INT64  signed long long
#define ST_UINT8  unsigned char     	
#define ST_UINT16 unsigned short    	
#define ST_UINT32 unsigned int			/* was unsigned long	*/
#define ST_UINT64 unsigned long long
#define ST_BOOLEAN  unsigned char		

/* This define shows that we really have support for 64 bit integers	*/
#define INT64_SUPPORT

/* This define shows that we have supplied all required 		*/
#define _SISCOTYPES_DEFINED

#endif	/* linux	*/

/************************************************************************/
/************************************************************************/
/* Make sure that this module has identified the target system 		*/

#if !defined(_IECTYPES_DEFINED)
#error Warning: System not correctly identified by iec_glbtypes.h
#endif

#if !defined(IEC_BYTE_ORDER)
#error IEC_BYTE_ORDER not defined
#endif



/************************************************************************/
#ifdef __cplusplus
}
#endif


#endif /* #ifndef GBLTYPES_INCLUDED */
