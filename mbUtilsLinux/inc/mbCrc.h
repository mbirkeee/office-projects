/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/inc/mbCrc.h,v 1.1.1.1 2007/07/10 05:06:02 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbCrc.h,v $
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef MBCRC_H
#define MBCRC_H

#include "mbTypes.h"

/*  CRC Polynomial */
/* x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0 */

#define MB_CRC_POLYNOMIAL 0x04C11DB7L

/******************************************************************************
 * Function Prototypes
 *-----------------------------------------------------------------------------
 */

Int32u_t        mbCrcFile
(
    Char_p      file_p,
    Int32u_p    size_p
);

Char_p          mbMD5File
(
    Char_p      fileName_p,
    Int8u_p     result_p,
    Char_p      str_p,
    Int32u_p    size_p
);

#endif /* MBCRC_H */
