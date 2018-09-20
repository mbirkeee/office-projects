/******************************************************************************
 * Copyright (c) 2008, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/backup_new/inc/restore.h,v 1.1 2008/04/06 14:16:07 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: restore.h,v $
 * Revision 1.1  2008/04/06 14:16:07  mikeb
 * New file
 *
 *
 *-----------------------------------------------------------------------------
 */

#ifndef H_RESTORE
#define H_RESTORE

/* Function prototypes */
Int32s_t check( Char_p backupDir_p, Int32u_t historyFlag, Int32u_t historyDate );
Int32s_t checkListAppend( Char_p file_p, qHead_p test_q, Int32u_t historyFlag, Int32u_t historyDate );
Int32s_t filesDelete( Char_p backupDir_p, Char_p deleteFile_p );
Int32s_t benchmark( Char_p dir_p, Boolean_t writeFlag );
Int32s_t historyAppend( Char_p file_p, qHead_p history_q );

#endif /* RESTORE */
