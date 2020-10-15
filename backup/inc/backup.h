/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/backup_new/inc/backup.h,v 1.7 2007/11/10 17:40:05 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: backup.h,v $
 * Revision 1.7  2007/11/10 17:40:05  mikeb
 * Add check database feature
 *
 * Revision 1.6  2007/07/22 01:05:09  mikeb
 * Updates for restore
 *
 * Revision 1.5  2007/07/18 03:55:07  mikeb
 * Fix lock file
 *
 * Revision 1.4  2007/07/17 01:44:17  mikeb
 * Ready to test file get
 *
 * Revision 1.3  2007/07/16 04:16:00  mikeb
 * July 15 updates.  Initial gile get from remote system
 *
 * Revision 1.2  2007/07/15 02:57:22  mikeb
 * Update to gzip data files
 *
 * Revision 1.1.1.1  2007/07/10 05:09:35  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef H_BACKUP
#define H_BACKUP

#define RS_MAX_PATH         2048

#define RS_PROG_NAME        "restore"
#define BK_PROG_NAME        "backup"
#define BK_LOCK_FILE        "backup-lock"
#define BK_LOCK_FILE_GET    "backup-get-lock"

#define CMD_CP              "/bin/cp -p"
#define CMD_MKDIR           "/bin/mkdir"
#define CMD_RMDIR           "/bin/rm -rf"
#define CMD_MV              "/bin/mv"
#define CMD_GZIP            "/bin/gzip"
#define CMD_GUNZIP          "/bin/gzip -d"
// #define CMD_SCP             "/usr/bin/scp -p -B -C -q -c blowfish"
//#define CMD_SCP             "/usr/bin/scp -p -B -C -q"
#define CMD_SCP             "/usr/bin/scp -p -B -C -q -l 8192 -v -o ConnectTimeout=30"
#endif /* H_BACKUP */
