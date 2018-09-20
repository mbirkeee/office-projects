/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/inc/mbLog.h,v 1.1.1.1 2007/07/10 05:06:02 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbLog.h,v $
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef MB_LOG_H
#define MB_LOG_H

Ints_t      mbLogInit( Char_p progName_p, Char_p logDir_p );
Ints_t      mbLog( Char_p  string_p, ... );
Char_p      mbCharTime( Char_p buf_p );
Char_p      mbCharDate( Char_p buf_p );
Int32u_t    mbToday( void );
Ints_t      mbSystem( Char_p cmd_p, Ints_p pid_p );

#endif /* MB_LOG_H */
