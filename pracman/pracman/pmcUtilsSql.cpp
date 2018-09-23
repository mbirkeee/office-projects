//---------------------------------------------------------------------------
// MySQL utility functions
//---------------------------------------------------------------------------
// (c) 2001-2008 Michael A. Bree
//---------------------------------------------------------------------------

// Platform includes
#include <stdio.h>
#include <vcl.h>
#include <time.h>
#pragma hdrstop

// Library includes
#include "mbUtils.h"

// Program includes
#include "pmcUtils.h"
#include "pmcUtilsSql.h"
#include "pmcInitialize.h"
#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcFormPickList.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// Get the most recent patient history
//---------------------------------------------------------------------------
Char_p pmcSqlPatHistoryGet( Int32u_t patientId, Int32u_t type, Int32u_p date_p, Int32u_p time_p )
{
    qHead_t         historyQueue;
    qHead_p         history_q;
    mbStrList_p     entry_p;
    Char_p          return_p = NIL;

    // Initialize history queue.
    history_q = qInitialize( &historyQueue );

    // Get list of histories
    if( pmcSqlPatHistoryListGet( patientId, type, history_q, FALSE ) > 0 )
    {
        entry_p = qLast( history_q, mbStrList_p );
        if( pmcSqlPatHistoryEntryGet( entry_p ) != TRUE )
        {
            mbDlgError( "Failed to get patient history" );
            goto exit;
        }

        if( entry_p->str_p == NIL )
        {
            mbDlgError( "History is NIL\n" );
        }
        mbMallocStr( return_p, entry_p->str_p );
        if( date_p ) *date_p = entry_p->handle;
        if( time_p ) *time_p = entry_p->handle2;
    }

exit:
    mbStrListFree( history_q );
    return return_p;
}

//---------------------------------------------------------------------------
// Get the patient history if necessary.  We use an mbStrList_t "entry_p".
// The history record id is assumed to be in entry_p->handle.  This history
// string is written into entry_p->str_p and the modification time string
// is written into entry_p->str2_p.  Memory for each string is allocated.
//
// The history get functions are written this way to allow the histories to
// be retrieved on demand, rather than all at the start.
//---------------------------------------------------------------------------
Int32u_t pmcSqlPatHistoryEntryGet( mbStrList_p entry_p )
{
    Int32u_t        result = FALSE;
    MbSQL           sql;
    MbDateTime      dateTime;
    MbString        cmd = MbString();

    // Sanity check
    if( entry_p == NIL ) goto exit;

    if( entry_p->handle == 0 )
    {
        // This is an empty entry.  Add empty stings to it if necessary
        if( entry_p->str_p == NIL )
        {
            mbMallocStr( entry_p->str_p,  "" );
        }
        if( entry_p->str2_p == NIL )
        {
            mbMallocStr( entry_p->str2_p, "" );
        }
        goto exit;
    }

    // Already have the history in this entry
    if( entry_p->str_p != NIL )
    {
        // Have already obtained this entry
        result = TRUE;
        goto exit;
    }

    // Format SQL command
    mbSprintf( &cmd, "select %s, %s from %s where %s=%lu",
        PMC_SQL_PAT_HISTORY_DATA,
        PMC_SQL_FIELD_MODIFIED,
        PMC_SQL_TABLE_PAT_HISTORY,
        PMC_SQL_FIELD_ID, entry_p->handle );

    // Execute the command
    sql.Query( cmd.get() );

    if( sql.RowCount( ) != 1 )
    {
        mbDlgError( "Error reading patient history" );
        goto exit;
    }
    sql.RowGet( );

    if( sql.String( 0 ) )
    {
        mbMallocStr( entry_p->str_p, sql.String( 0 ) );

        // Get the modify time
        dateTime.SetDateTime64( sql.DateTimeInt64u( 1 ) );
        mbSprintf( &cmd, "%s %s", dateTime.MDY_DateString( ), dateTime.HM_TimeString( ) );

        mbMallocStr( entry_p->str2_p, cmd.get() );
        entry_p->handle = dateTime.Date( );
        entry_p->handle2 = dateTime.Time( );
    }
    result = TRUE;

exit:

    return result;
}

//---------------------------------------------------------------------------
// Get list of available patient histories.  The actual history contents
// are not retrieved here; rather they are retrieved on demand.  The addFlag
// indicates that an empty entry should be added if none are found
//---------------------------------------------------------------------------
Int32u_t pmcSqlPatHistoryListGet( Int32u_t patientId, Int32u_t type, qHead_p history_q, Boolean_t addFlag )
{
    Int32u_t        count = 0;
    MbSQL           sql;
    MbString        cmd = MbString();
    mbStrList_p     entry_p;

    // Format SQL command
    mbSprintf( &cmd, "select %s from %s "
                     "where %s=%lu and %s=%lu and %s=%lu "
                     "order by %s",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_PAT_HISTORY,
        PMC_SQL_FIELD_PATIENT_ID,   patientId,
        PMC_SQL_FIELD_TYPE,         type,
        PMC_SQL_FIELD_NOT_DELETED,  PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE,
        PMC_SQL_FIELD_ID );

    // Execute the command
    sql.Query( cmd.get() );

    // Loop through the results; store only the IDs
    while( sql.RowGet() )
    {
        mbCalloc( entry_p, sizeof(mbStrList_t) );
        entry_p->handle = sql.Int32u( 0 );
        qInsertLast( history_q, entry_p );
        count++;
    }

    if( count == 0 && addFlag == TRUE )
    {
        // Insert an empty entry
        mbCalloc( entry_p, sizeof(mbStrList_t) );
        qInsertLast( history_q, entry_p );
        count++;
    }
    return count;
}

//---------------------------------------------------------------------------
// Put specified patient history string into the database in a new record
//---------------------------------------------------------------------------

Int32u_t pmcSqlPatHistoryPut( Int32u_t patientId, Int32u_t type, Char_p history_p )
{
    Int32u_t    recordId = 0;

    if( patientId == 0 || history_p == NULL ) goto exit;

    // Fisrt, must create a new record in the database
    if( ( recordId = pmcSqlRecordCreate( PMC_SQL_TABLE_PAT_HISTORY, NIL ) ) == 0 )
    {
        goto exit;
    }

    // Put the patient id into the record
    pmcSqlExecInt( PMC_SQL_TABLE_PAT_HISTORY,
                   PMC_SQL_FIELD_PATIENT_ID,
                   patientId,
                   recordId );

    // Put the type into the record
    pmcSqlExecInt( PMC_SQL_TABLE_PAT_HISTORY,
                   PMC_SQL_FIELD_TYPE,
                   type,
                   recordId );

    // Put the actual history into the record
    pmcSqlExecString( PMC_SQL_TABLE_PAT_HISTORY,
                      PMC_SQL_PAT_HISTORY_DATA,
                      history_p,
                      recordId );

    // Now make record visible to others
    pmcSqlRecordUndelete( PMC_SQL_TABLE_PAT_HISTORY, recordId );

exit:
    return recordId;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
Int64u_t    pmcSqlTableUpdateTimeGet
(
    Char_p  requestedTable_p
)
{
    Char_p      table_p;
    Int64u_t    result = 0;
    MbSQL       sql;

    if( sql.Query( "show table status" ) )
    {
        while( sql.RowGet( ) )
        {
            if( ( table_p = sql.String( 0 ) ) != NIL )
            {
                if( strcmp( table_p, requestedTable_p ) == 0 )
                {
                    result = sql.DateTimeInt64u( 12 );
                    break;
                }
            }
        }
    }

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlExecInt
(
    Char_p      table_p,
    Char_p      field_p,
    Int32u_t    value,
    Int32u_t    id
)
{
    Char_p      cmd_p = NIL;
    Int32u_t    result = FALSE;
    MbSQL       sql;

    if( table_p == NIL || field_p == NIL ) goto exit;

    if( mbMalloc( cmd_p, 256 ) == NIL ) goto exit;

    sprintf( cmd_p, "update %s set %s=%d where id=%ld",
        table_p, field_p, value, id );

    result = sql.Update( cmd_p );

exit:
    mbFree( cmd_p );

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------


Int32u_t        pmcSqlExecString
(
    Char_p      table_p,
    Char_p      field_p,
    Char_p      string_p,
    Int32u_t    id
)
{
    Char_p      cmd_p = NIL;
    Char_p      string2_p = NIL;
    Int32u_t    result = FALSE;
    Ints_t      len;
    MbSQL       sql;

    if( table_p == NIL || field_p == NIL || string_p == NIL ) goto exit;

    len = strlen( string_p );

    if( mbMalloc( cmd_p, len + 100 ) == NIL ) goto exit;
    if( mbMalloc( string2_p, len + 1 ) == NIL ) goto exit;

    strcpy( string2_p, string_p );
    pmcStringStripDoubleQuotes( string2_p );

    sprintf( cmd_p, "update %s set %s=\"%s\" where %s=%ld",
        table_p, field_p, string2_p, PMC_SQL_FIELD_ID, id );

    result = sql.Update( cmd_p );

exit:
    mbFree( cmd_p );
    mbFree( string2_p );

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlExecDate
(
    Char_p      table_p,
    Char_p      field_p,
    Int32u_t    value,
    Int32u_t    id
)
{
    Char_p      cmd_p = NIL;
    Int32u_t    result = FALSE;
    MbSQL       sql;

    if( table_p == NIL || field_p == NIL ) goto exit;

    if( mbMalloc( cmd_p, 512 ) == NIL ) goto exit;

    sprintf( cmd_p, "update %s set %s=%d where id=%ld",
        table_p, field_p, value, id );

    result = sql.Update( cmd_p );

exit:

    mbFree( cmd_p );
    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlExecTime
(
    Char_p      table_p,
    Char_p      field_p,
    Int32u_t    value,
    Int32u_t    id
)
{
    Char_p      buf_p = NIL;
    Int32u_t    result = FALSE;
    MbSQL       sql;

    if( table_p == NIL || field_p == NIL ) goto exit;

    if( mbMalloc( buf_p, 512 ) == NIL ) goto exit;

    sprintf( buf_p, "update %s set %s=%d where id=%ld",
        table_p, field_p, value, id );

    result = sql.Update( buf_p );

exit:

    mbFree( buf_p );
    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlRecordDeleted
(
    Char_p      table_p,
    Int32u_t    id
)
{
    Char_p      buf_p = NIL;
    Int32u_t    deleted = FALSE;
    Int32u_t    notDeleted;
    Int32u_t    count;

    if( table_p == NIL ) goto exit;
    mbMalloc( buf_p, 256 );

    sprintf( buf_p, "select %s from %s where %s=%ld",
             PMC_SQL_FIELD_NOT_DELETED,
             table_p,
             PMC_SQL_FIELD_ID, id );

    notDeleted = pmcSqlSelectInt( buf_p, &count );

    if( count == 0 )
    {
        deleted = TRUE;
    }
    else if( count == 1 )
    {
        if( notDeleted == PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE ) deleted = TRUE;
    }
    else
    {
        mbDlgDebug(( "Error: record not found\n" ));
    }

exit:
    mbFree( buf_p );
    return deleted;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlRecordDelete
(
    Char_p      table_p,
    Int32u_t    id
)
{
    return pmcSqlRecordDeleteUndelete( table_p, id, TRUE );
}
//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlRecordUndelete
(
    Char_p      table_p,
    Int32u_t    id
)
{
    return pmcSqlRecordDeleteUndelete( table_p, id, FALSE );
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlRecordDeleteUndelete
(
    Char_p      table_p,
    Int32u_t    id,
    bool        delFlag
)
{
    Int32u_t    result = TRUE;
    Int32u_t    delValue;
    Int32u_t    curValue;
    MbSQL       sql;
    MbString    cmd;

    if( table_p == NIL ) goto exit;

    // delFlag == TRUE   -  deleted      - not_deleted = 0
    // delFlag == FALSE  -  not deleted  - not_deleted = 1
    delValue = delFlag ? PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE : PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE;
    curValue = delFlag ? PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE  : PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE;

    mbSprintf( &cmd, "update %s set %s=%ld where id=%ld and %s=%ld",
        table_p,
        PMC_SQL_FIELD_NOT_DELETED, delValue, id,
        PMC_SQL_FIELD_NOT_DELETED, curValue );

    sql.Update( cmd.get( ) );

exit:
    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t        pmcSqlExec
(
    Char_p      cmd_p
)
{
    MbSQL       sql;
    if( sql.Update( cmd_p ) == TRUE )
    {
        return MB_RET_OK;
    }
    return MB_RET_ERROR;
}

//---------------------------------------------------------------------------
// Function: pmcSqlMatchCount
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32u_t        pmcSqlMatchCount
(
    Char_p      cmd_p
)
{
    Int32u_t    rowCount = 0;
    MbSQL       sql;

    if( cmd_p == 0 ) return 0;

    sql.Query( cmd_p );
    rowCount = sql.RowCount( );

    return rowCount;
}

//---------------------------------------------------------------------------
// Function: pmcSqlSelectInt
//---------------------------------------------------------------------------
// Description:
//
// Execute a select statement that should return a single int
//---------------------------------------------------------------------------

Int32u_t  pmcSqlSelectInt
(
    Char_p      cmd_p,
    Int32u_p    count_p     // Indicate how many matches were found in database
)
{
    Int32u_t    result = 0;
    Int32u_t    rowCount = 0;
    MbSQL       sql;

    if( sql.Query( cmd_p ) == FALSE ) goto exit;

    if( ( rowCount = sql.RowCount( ) ) != 1 ) goto exit;

    if( sql.RowGet( ) )
    {
        result = sql.Int32u( 0 );
    }

exit:
    if( count_p == NIL )
    {
        if( rowCount != 1 )
        {
            mbDlgExclaim( "Cmd '%s' expected 1 result; got %ld", cmd_p, rowCount );
        }
    }
    else
    {
        *count_p = rowCount;
    }
    return result;
}

//---------------------------------------------------------------------------
// Function: pmcSqlSelectString
//---------------------------------------------------------------------------
// Description:
//
// Execute a select statement that should return a single string
//---------------------------------------------------------------------------

Char_p          pmcSqlSelectStr
(
    Char_p      cmd_p,
    Char_p      result_p,
    Ints_t      resultLen,
    Int32u_p    count_p
)
{
    MbSQL       sql;
    Int32u_t    rowCount = 0;

    if( result_p == NIL )
    {
        mbDlgDebug(( "called with NIL pointer" ));
        goto exit;
    }
    *result_p = NULL;

    if( cmd_p == NIL ) goto exit;

    if( sql.Query( cmd_p ) == FALSE ) goto exit;

    while( sql.RowGet() )
    {
        if( rowCount == 0 )
        {
            if( sql.String( 0 ) )
            {
                strncpy( result_p, sql.String( 0 ), resultLen );
            }
        }
        rowCount++;
    }

exit:
    // Indicate how many results obtained
    if( count_p == NIL )
    {
        if( rowCount != 1 )
        {
            mbDlgExclaim( "Cmd '%s' expected 1 result; got %ld", cmd_p, rowCount );
        }
    }
    else
    {
        *count_p = rowCount;
    }
    return result_p;
}

//---------------------------------------------------------------------------
// Function: pmcSqlRecordCreate
//---------------------------------------------------------------------------
// Description:
//
// This function creates a new record in the specified table
//---------------------------------------------------------------------------

Int32u_t    pmcSqlRecordCreate
(
    Char_p      table_p,
    MbSQL      *sqlIn_p
)
{
    Int32u_t    id = 0;
    Boolean_t   lockTableFlag = FALSE;
    MbSQL      *sql_p;
    MbString    buf;

    if( sqlIn_p )
    {
        sql_p = sqlIn_p;
    }
    else
    {
        sql_p = new MbSQL();
    }

    // NOTE: Must do all SQL commands with same connection (i.e., sql object)
    pmcSuspendPollInc( );

    if( sqlIn_p == NIL )
    {
        lockTableFlag = TRUE;
        mbSprintf( &buf, "lock tables %s write", table_p );
        sql_p->Update( buf.get() );
    }
    else
    {
        // Table must already be locked
    }

    // First, get the max table id
    mbSprintf( &buf, "select max(%s) from %s where %s>0",
        PMC_SQL_FIELD_ID, table_p, PMC_SQL_FIELD_ID );

    sql_p->Query( buf.get() );
    while( sql_p->RowGet() )
    {
        id = sql_p->Int32u( 0 );
    }

    // Increment id
    id++;

    // Now create the new record
    mbSprintf( &buf, "insert into %s (%s,%s) values (%ld, now() )",
            table_p,
            PMC_SQL_FIELD_ID,
            PMC_SQL_FIELD_CREATED,
            id );

    if( sql_p->Update( buf.get() ) == TRUE )
    {
        mbLog( "Created new record id: %u in table: %s\n", id, table_p );
    }
    else
    {
        mbLog( "Create record id: %u in table: %s failed\n", id, table_p );
        id = 0;
    }

    if( lockTableFlag == TRUE )
    {
        mbSprintf( &buf, "unlock tables" );
        sql_p->Update( buf.get() );
    }

    pmcSuspendPollDec( );

    if( sqlIn_p == NIL )
    {
        delete sql_p;
    }
    return id;
}

//---------------------------------------------------------------------------
// Function: pmcSqlRecordLock
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32u_t pmcSqlEchoLock( Int32u_t id, Char_p name_p )
{
    Int32u_t    returnCode = FALSE;
    Int32s_t    result;

    if( pmcSqlRecordLock( PMC_SQL_TABLE_ECHOS, id, TRUE ) )
    {
        returnCode = TRUE;
        goto exit;
    }

    result = mbDlgYesNo( "Echo Locked: %s\n\n"
                         "This echo is locked for editing or viewing by another user.\n\n"
                         "To unlock this echo, click Yes.\n"
                         "To leave this echo locked, click No.\n\n"
                         "WARNING:\n\n"
                         "Unlocking a locked echo can cause data corruption if it is truly being\n"
                         "used by another user.  Unlock only if you're sure it is safe to do so.", name_p );

    if( result == MB_BUTTON_YES )
    {
        pmcSqlRecordUnlockForce( PMC_SQL_TABLE_ECHOS, id );
        if( pmcSqlRecordLock( PMC_SQL_TABLE_ECHOS, id, TRUE ) == FALSE )
        {
            mbDlgInfo( "Unlock failed." );
        }
        else
        {
            returnCode = TRUE;
        }
    }

exit:
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcSqlRecordLock
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32u_t    pmcSqlRecordLock
(
    Char_p      table_p,
    Int32u_t    id,
    Int32u_t    notDeletedFlag
)
{
    Int32u_t    lockValue;
    Int32u_t    result = FALSE;
    MbSQL       sql;
    Char_t      buf[256];

    pmcSuspendPollInc( );

    // First, lock the tables
    sprintf( buf, "lock tables %s write", table_p );
    sql.Update( buf );

    // Next, read back the lock value
    sprintf( buf, "select %s from %s where %s=%ld",
        PMC_SQL_FIELD_LOCK, table_p, PMC_SQL_FIELD_ID, id );

    sql.Query( buf );
    while( sql.RowGet() )
    {
        lockValue = sql.Int32u( 0 );
    }

    if( lockValue == 0 )
    {
        sprintf( buf, "update %s set %s=%ld where %s=%ld and %s=0",
             table_p,
             PMC_SQL_FIELD_LOCK, pmcSqlLockValue,
             PMC_SQL_FIELD_ID, id,
             PMC_SQL_FIELD_LOCK );

        if( notDeletedFlag == TRUE )
        {
            // Only lock record if not deleted
            Char_t  buf2[32];
            sprintf( buf2, " and %s=%ld", PMC_SQL_FIELD_NOT_DELETED, PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );
            strcat( buf, buf2 );
        }

        sql.Update( buf );

        // Next, read back the lock value.. this is a sanity check
        sprintf( buf, "select %s from %s where %s=%ld",
            PMC_SQL_FIELD_LOCK, table_p, PMC_SQL_FIELD_ID, id );

        sql.Query( buf );
        while( sql.RowGet() )
        {
            lockValue = sql.Int32u( 0 );
        }
    }
    else
    {
        lockValue = 0;
    }

    sprintf( buf, "unlock tables" );
    sql.Update( buf );
    pmcSuspendPollDec( );

    result = ( lockValue == pmcSqlLockValue ) ? TRUE : FALSE;

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcSqlRecordUnlock
(
    Char_p      table_p,
    Int32u_t    id
)
{
    Char_t      buf[128];
    Int32u_t    lockValue;
    Int32u_t    result;
    MbSQL       sql;

    pmcSuspendPollInc( );

    sprintf( buf, "lock tables %s write", table_p );
    sql.Update( buf );

    sprintf( buf, "update %s set %s=0 where %s=%ld and %s=%ld",
        table_p,
        PMC_SQL_FIELD_LOCK,
        PMC_SQL_FIELD_ID, id,
        PMC_SQL_FIELD_LOCK, pmcSqlLockValue  );

    sql.Update( buf );

    // Next, read back the lock value
    sprintf( buf, "select %s from %s where %s=%ld",
        PMC_SQL_FIELD_LOCK, table_p, PMC_SQL_FIELD_ID, id );

    sql.Query( buf );
    while( sql.RowGet() )
    {
        lockValue = sql.Int32u( 0 );
    }

    sprintf( buf, "unlock tables" );
    sql.Update( buf );

    pmcSuspendPollDec( );

    if( lockValue == 0 )
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
        mbLog( "Cannot unlock record %ld in table %s (locked by another client).", id, table_p );
    }

    return result;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32u_t    pmcSqlRecordUnlockForce
(
    Char_p      table_p,
    Int32u_t    id
)
{
    Char_t      buf[128];
    Int32u_t    lockValue;
    Int32u_t    result;

    sprintf( buf, "update %s set %s=0 where %s=%ld",
        table_p,
        PMC_SQL_FIELD_LOCK,
        PMC_SQL_FIELD_ID, id );

    pmcSqlExec( buf );

    // Next, read back the lock value the max table id
    sprintf( buf, "select %s from %s where %s=%ld",
        PMC_SQL_FIELD_LOCK, table_p, PMC_SQL_FIELD_ID, id );

    lockValue = pmcSqlSelectInt( buf, NIL );

    if( lockValue == 0 )
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
        mbLog( "Cannot unlock record %ld in table %s (locked by another client).", id, table_p );
    }
    return result;
}

//---------------------------------------------------------------------------
// Function: pmcSqlRecordPurge
//---------------------------------------------------------------------------
// Description:
//
// This function completely removes the record from the database.  No
// checking is done.  It is for debugging only and should not be available
// in production versions.
//---------------------------------------------------------------------------

Int32u_t    pmcSqlRecordPurge
(
    Char_p      table_p,
    Int32u_t    id
)
{
    Char_t      buf[1024];

    // First, get the max table id
    sprintf( buf, "delete from %s where %s=%ld", table_p, PMC_SQL_FIELD_ID, id );

    pmcSqlExec( buf );

    mbDlgExclaim( "Purged record id %ld from table '%s'", id, table_p );

    return id;
}

//---------------------------------------------------------------------------
// Function: pmcSqlSonographerDetailsGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t  pmcSqlSonographerDetailsGet
(
    Int32u_t                id,
    PmcSqlSonographer_p     sono_p
)
{
    Char_p          buf_p  = NIL;
    Int32s_t        returnCode = FALSE;
    MbSQL           sql;

    if( mbMalloc( buf_p, 128 ) == NIL ) goto exit;
    if( sono_p == NIL ) goto exit;

    // Format SQL command
    sprintf( buf_p, "select %s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,            // 0
        PMC_SQL_FIELD_FIRST_NAME,           // 1

        PMC_SQL_TABLE_SONOGRAPHERS,
        PMC_SQL_FIELD_ID, id );

    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        strncpy( sono_p->lastName,    sql.String( 0 ), PMC_MAX_NAME_LEN );
        strncpy( sono_p->firstName,   sql.String( 1 ), PMC_MAX_NAME_LEN );
    }

    sono_p->id = id;

    // Sanity Check
    if( sql.RowCount( ) != 1 )
    {
        mbDlgDebug(( "Error locating sonographer  id %ld in database (records: %u)", id, sql.RowCount( ) ));
    }
    else
    {
        returnCode = TRUE;
    }

exit:
    mbFree( buf_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcSqlAppDetailsGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t    pmcSqlAppDetailsGet
(
    Int32u_t        id,
    PmcSqlApp_p     app_p
)
{
    Char_p          buf_p = NIL;
    Int32s_t        returnCode = FALSE;
    Int32u_t        notDeleted;
    MbSQL           sql;

#if PMC_APP_HISTORY_CREATE
    Variant         varTime;
    MDateTime      *dateTime_p;

    dateTime_p = new MDateTime( );
#endif

    if( mbMalloc( buf_p, 1024 ) == NIL ) goto exit;
    if( app_p == NIL ) goto exit;
    memset( app_p, 0, sizeof( PmcSqlApp_t ) );
    if( id == 0 ) goto exit;

    //                       0  1  2  3  4  5  6  7  8  9
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s, "
                           "%s,%s,%s,%s,%s,%s,%s,%s "
                    "from %s where %s=%ld",
        PMC_SQL_FIELD_PATIENT_ID,                   // 0
        PMC_SQL_APPS_FIELD_START_TIME,              // 1
        PMC_SQL_APPS_FIELD_DURATION,                // 2
        PMC_SQL_FIELD_PROVIDER_ID,                  // 3
        PMC_SQL_FIELD_DATE,                         // 4
        PMC_SQL_APPS_FIELD_TYPE,                    // 5
        PMC_SQL_APPS_FIELD_CONF_PHONE_DATE,         // 6
        PMC_SQL_APPS_FIELD_CONF_PHONE_TIME,         // 7
        PMC_SQL_APPS_FIELD_CONF_PHONE_ID,           // 8
        PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,        // 9
        PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,        // 10
        PMC_SQL_APPS_FIELD_CONF_LETTER_ID,          // 11
        PMC_SQL_APPS_FIELD_COMPLETED,               // 12
        PMC_SQL_APPS_FIELD_REFERRING_DR_ID,         // 13
        PMC_SQL_FIELD_NOT_DELETED,                  // 14
        PMC_SQL_FIELD_CREATED,                      // 15
        PMC_SQL_APPS_FIELD_COMMENT_IN,              // 16
        PMC_SQL_APPS_FIELD_COMMENT_OUT,             // 17

        PMC_SQL_TABLE_APPS,
        PMC_SQL_FIELD_ID, id );

    if( sql.Query( buf_p ) == FALSE ) goto exit;
    if( sql.RowCount( ) != 1 ) goto exit;
    if( sql.RowGet( ) == FALSE ) goto exit;

    app_p->patientId        = sql.Int32u(0);
    app_p->startTime        = sql.Int32u(1);
    app_p->duration         = sql.Int32u(2);
    app_p->providerId       = sql.Int32u(3);
    app_p->date             = sql.Int32u(4);
    app_p->type             = sql.Int32u(5);
    app_p->confPhoneDate    = sql.Int32u(6);
    app_p->confPhoneTime    = sql.Int32u(7);
    app_p->confPhoneId      = sql.Int32u(8);
    app_p->confLetterDate   = sql.Int32u(9);
    app_p->confLetterTime   = sql.Int32u(10);
    app_p->confLetterId     = sql.Int32u(11);
    app_p->completed        = sql.Int32u(12);
    app_p->referringDrId    = sql.Int32u(13);
    notDeleted              = sql.Int32u(14);
    app_p->deleted = ( notDeleted == PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE ) ? FALSE : TRUE;

#if PMC_APP_HISTORY_CREATE
        varTime =  pmcDataSet_p->Fields->Fields[15]->AsVariant;
        dateTime_p->Update( &varTime );
        app_p->createDate = dateTime_p->DateInt;
        app_p->createTime = dateTime_p->TimeInt;
#endif
    strncpy( app_p->commentIn, sql.String( 16 ), PMC_MAX_COMMENT_LEN );
    strncpy( app_p->commentOut, sql.String( 17 ), PMC_MAX_COMMENT_LEN );

    returnCode = TRUE;

exit:
    if( returnCode != TRUE )
    {
       mbDlgDebug(( "Error locating patient id %ld in database (records: %u)", id, sql.RowCount( ) ));
    }

    mbFree( buf_p );

#if PMC_APP_HISTORY_CREATE
    delete dateTime_p;
#endif

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcSqlPatientDetailsGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t    pmcSqlPatientDetailsGet
(
    Int32u_t        id,
    PmcSqlPatient_p pat_p
)
{
    Char_p          buf_p = NIL;
    Int32s_t        returnCode = MB_RET_ERR;
    Boolean_t       localFlag;
    Char_p          phoneHome_p;
    Char_p          phoneDay_p;
    MbSQL           sql;

    if( mbMalloc( buf_p, 512 ) == NIL ) goto exit;

    // Zero out the structure
    if( pat_p ) memset( pat_p, 0, sizeof( PmcSqlPatient_t ) );

    if( id == 0 || pat_p == NIL ) goto exit;

    // Format SQL command    0  1  2  3  4  5  6  7  8  9 10
    sprintf( buf_p, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_TITLE,                    // 2
        PMC_SQL_PATIENTS_FIELD_PHN,             // 3
        PMC_SQL_PATIENTS_FIELD_PHN_PROV,        // 4
        PMC_SQL_PATIENTS_FIELD_DATE_OF_BIRTH,   // 5
        PMC_SQL_PATIENTS_FIELD_GENDER,          // 6
        PMC_SQL_PATIENTS_FIELD_HOME_PHONE,      // 7
        PMC_SQL_PATIENTS_FIELD_WORK_PHONE,      // 8
        PMC_SQL_PATIENTS_FIELD_REFERRING_DR_ID, // 9
        PMC_SQL_PATIENTS_FIELD_DATE_OF_DEATH,   // 10
        PMC_SQL_TABLE_PATIENTS,
        PMC_SQL_FIELD_ID, id );


    if( sql.Query( buf_p ) == FALSE ) goto exit;
    if( sql.RowCount() != 1 ) goto exit;
    if( sql.RowGet( ) == FALSE ) goto exit;

    strncpy( pat_p->lastName,  sql.String( 0 ), PMC_MAX_NAME_LEN );
    strncpy( pat_p->firstName, sql.String( 1 ), PMC_MAX_NAME_LEN );
    strncpy( pat_p->title,     sql.String( 2 ), PMC_MAX_TITLE_LEN );
    strncpy( pat_p->phn,       sql.String( 3 ), PMC_MAX_PHN_LEN );
    strncpy( pat_p->phnProv,   sql.String( 4 ), PMC_MAX_PROV_LEN );

    pat_p->birthDate = sql.Int32u( 5 );
    pat_p->gender    = sql.Int32u( 6 );

    phoneHome_p = sql.String( 7 );
    phoneDay_p  = sql.String( 8 );

    pat_p->refDrId      = sql.Int32u( 9 );
    pat_p->deceasedDate = sql.Int32u( 10 );
    pat_p->id = id;
    
    returnCode = MB_RET_OK;

    pmcPhoneFormat( phoneHome_p, pat_p->areaHome, pat_p->phoneHome, &localFlag );
    if( !localFlag && strlen( pat_p->phoneHome ) )
    {
        sprintf( pat_p->formattedPhoneHome, "(%s) %s", pat_p->areaHome, pat_p->phoneHome );
    }
    else
    {
        sprintf( pat_p->formattedPhoneHome, "%s", pat_p->phoneHome );
    }

    pmcPhoneFormat( phoneDay_p, pat_p->areaDay, pat_p->phoneDay, &localFlag );
    if( !localFlag && strlen( pat_p->phoneDay ) )
    {
        sprintf( pat_p->formattedPhoneDay, "(%s) %s", pat_p->areaDay, pat_p->phoneDay );
    }
    else
    {
        sprintf( pat_p->formattedPhoneDay, "%s", pat_p->phoneDay );
    }

exit:

    if( returnCode != MB_RET_OK && id != 0 )
    {
        mbDlgDebug(( "Error locating patient id %ld in database (records: %u)", id, sql.RowCount( ) ));
    }
    mbFree( buf_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:   pmcSqlEchoDetailsFree
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------
Int32s_t  pmcSqlEchoDetailsFree( PmcSqlEchoDetails_p echo_p )
{
    if( echo_p )
    {
        mbFree( echo_p->comment_p       );
        mbFree( echo_p->studyName_p     );
        mbFree( echo_p->text_p          );
        mbFree( echo_p->imageQuality_p  );
        mbFree( echo_p->indication_p    );
        mbFree( echo_p->mv_reg_p        );
        mbFree( echo_p->mv_sten_p       );
        mbFree( echo_p->av_reg_p        );
        mbFree( echo_p->av_sten_p       );
        mbFree( echo_p->pv_reg_p        );
        mbFree( echo_p->tv_reg_p        );
        mbFree( echo_p->tv_sten_p       );
        mbFree( echo_p->rhythm_p        );
        mbFree( echo_p );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// This should be part of an echo class, like the new Patient class
//---------------------------------------------------------------------------
PmcSqlEchoDetails_p pmcSqlEchoDetailsGet( Int32u_t echoId, Boolean_t subDetailsFlag )
{
    PmcSqlEchoDetails_p     echo_p = NIL;
    Int32s_t                returnCode = FALSE;
    Int32u_t                count;
    Boolean_t               gotLock = FALSE;
    Boolean_t               createNew = FALSE;
    MbSQL                   sql;
    Int32u_t                patId;
    MbString                cmd;
    MbCursor                cursor = MbCursor( crHourGlass );

    mbCalloc( echo_p, sizeof( PmcSqlEchoDetails_t ));

    if( echo_p == NIL ) goto exit;

    echo_p->echoId = echoId;

start:

    // First, get details from the echo structure
    //                        0  1  2  3  4  5  6
    mbSprintf( &cmd, "select %s,%s,%s,%s,%s,%s,%s from %s where %s=%ld"
        ,PMC_SQL_FIELD_PATIENT_ID       // 0
        ,PMC_SQL_ECHOS_FIELD_NAME       // 1
        ,PMC_SQL_ECHOS_FIELD_COMMENT    // 2
        ,PMC_SQL_FIELD_PROVIDER_ID      // 3
        ,PMC_SQL_FIELD_SONOGRAPHER_ID   // 4
        ,PMC_SQL_FIELD_DATE             // 5
        ,PMC_SQL_ECHOS_FIELD_READ_DATE  // 6

        ,PMC_SQL_TABLE_ECHOS
        ,PMC_SQL_FIELD_ID,  echoId );

    if( sql.Query( cmd.get() ) == FALSE ) goto exit;

    if( sql.RowCount() != 1 ) goto exit;

    while( sql.RowGet() )
    {
        echo_p->patient.id      = sql.Int32u( 0 );
        echo_p->sono.id         = sql.Int32u( 4 );
        echo_p->studyDate       = sql.Int32u( 5 );
        echo_p->readDate        = sql.Int32u( 6 );
        echo_p->provider.id     = sql.Int32u( 3 );

        mbMallocStr( echo_p->studyName_p,   sql.String( 1 ) );
        mbMallocStr( echo_p->comment_p,     sql.String( 2 ) );
    }

    // Next get echo_details record
    mbSprintf( &cmd, "select %s from %s where %s=%lu",
        PMC_SQL_FIELD_ID,
        PMC_SQL_TABLE_ECHO_DETAILS,
        PMC_SQL_FIELD_ECHO_ID, echoId );

    echo_p->detailsId = pmcSqlSelectInt( cmd.get(), &count );

    if( count == 0 )
    {
        // Attempt to lock the echo
        if( gotLock == FALSE )
        {
            if( pmcSqlEchoLock( echoId, echo_p->studyName_p ) == FALSE )
            {
                mbDlgError( "Failed to get echo details record\n" );
                goto exit;
            }
        }

        gotLock = TRUE;

        if( echo_p->readDate == PMC_ECHO_STATUS_NO_ECHO )
        {
            createNew = TRUE;
        }
        else
        {
            mbSprintf( &cmd, "select %s,%s,%s,%s from %s where %s=%lu and %s=%lu order by id desc",
                PMC_SQL_FIELD_ID,
                PMC_SQL_ECHOS_FIELD_NAME,
                PMC_SQL_FIELD_PATIENT_ID,
                PMC_SQL_ECHOS_FIELD_COMMENT,
                PMC_SQL_TABLE_ECHOS,

                PMC_SQL_ECHOS_FIELD_READ_DATE,  PMC_ECHO_STATUS_NO_ECHO,
                PMC_SQL_FIELD_NOT_DELETED,      PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE
             );

            if( sql.Query( cmd.get() ) == FALSE ) goto exit;

            if( sql.RowCount() == 0 )
            {
                // There are no "pre-echos... just go ahead and create a new report
                createNew = TRUE;
            }
            else
            {
                qHead_t             strQueue;
                qHead_p             str_q;
                Int32u_t            id;
                PmcSqlPatient_t     pat;
                mbStrList_p         str_p;
                Boolean_t           found;
                Char_t              temp[512];
                Char_t              reportName[256];
                Char_t              echoName[256];
                Char_p              comment_p;

                str_q = qInitialize( &strQueue );

                if( echo_p->studyName_p )
                {
                    sprintf( echoName, "%s", echo_p->studyName_p );
                }
                else
                {
                    sprintf( echoName, "ID %u\n", echoId );
                }

                mbStrListAddNew( str_q, "New Report", NIL, 0, 0 );

                while( sql.RowGet() )
                {
                    id = sql.Int32u( 0 );
                    patId = sql.Int32u( 2 );

                    if( strlen( sql.String( 1 ) ) )
                    {
                        sprintf( reportName, "%s", sql.String( 1 ) );
                    }
                    else
                    {
                        sprintf( reportName, "ID: %u", id );
                    }

                    if( patId )
                    {
                        pmcSqlPatientDetailsGet( patId, &pat );
                        sprintf( temp, "%s (%s %s)", reportName, pat.firstName, pat.lastName );
                    }
                    else
                    {
                        sprintf( temp, "%s", reportName );
                    }
                    mbStrListAddNew( str_q, temp, sql.String( 3 ), id, 0 );
                }

                sprintf( temp, "Choose report for echo: %s", echoName );
                id = pmcFormPickList( temp, str_q );

                if( id == 0 )
                {
                    sprintf( temp, "Create new report for echo %s?", echoName );
                    comment_p = gPmcEmptyString_p;
                }
                else
                {
                    found = FALSE;
                    qWalk( str_p, str_q, mbStrList_p )
                    {
                        if( str_p->handle == id )
                        {
                            found = TRUE;
                            comment_p = str_p->str2_p;
                            break;
                        }
                    }
                    if( found )
                    {
                        sprintf( temp, "Assign report '%s'\nto echo '%s?'", str_p->str_p, echoName );
                    }
                    else
                    {
                        mbDlgError( "Did not find report!\n" );
                        mbStrListFree( str_q );
                        goto exit;
                    }
                }

                if( mbDlgOkCancel( temp ) == MB_BUTTON_OK )
                {
                    if( id == 0 )
                    {
                        mbStrListFree( str_q );
                        createNew = TRUE;
                        goto create;
                    }
                }
                else
                {
                    mbStrListFree( str_q );
                    goto exit;
                }

                if( pmcSqlEchoLock( id, echo_p->studyName_p ) == FALSE )
                {
                    mbDlgError( "Failed to lock echo report\n" );
                    mbStrListFree( str_q );
                    goto exit;
                }

                // Get the patient ID
                mbSprintf( &cmd, "select %s from %s where %s=%lu",
                    PMC_SQL_FIELD_PATIENT_ID,
                    PMC_SQL_TABLE_ECHOS,
                    PMC_SQL_FIELD_ID, id );

                patId = pmcSqlSelectInt( cmd.get(), &count );
                if( count != 1 )
                {
                    mbDlgError( "Failed to get patient ID\n" );
                    goto exit;
                }

                mbSprintf( &cmd, "select %s from %s where %s=%lu",
                    PMC_SQL_FIELD_ID,
                    PMC_SQL_TABLE_ECHO_DETAILS,
                    PMC_SQL_FIELD_ECHO_ID, id );

                echo_p->detailsId = pmcSqlSelectInt( cmd.get(), &count );
                if( count != 1 )
                {
                    mbDlgError( "Failed to get patient ID\n" );
                    goto exit;
                }

                mbSprintf( &cmd, "update %s set %s=%u, %s=\"%s\" where %s=%u",
                    PMC_SQL_TABLE_ECHOS,
                    PMC_SQL_FIELD_PATIENT_ID,       patId,
                    PMC_SQL_ECHOS_FIELD_COMMENT,    comment_p,
                    PMC_SQL_FIELD_ID,               echoId );

                sql.Update( cmd.get() );

                mbSprintf( &cmd, "update %s set %s=%lu where %s=%lu",
                     PMC_SQL_TABLE_ECHOS,
                     PMC_SQL_FIELD_NOT_DELETED,      PMC_SQL_FIELD_NOT_DELETED_FALSE_VALUE,
                     PMC_SQL_FIELD_ID,                 id );

                sql.Update( cmd.get() );

                mbSprintf( &cmd, "update %s set %s=%lu where %s=%lu",
                     PMC_SQL_TABLE_ECHO_DETAILS,
                     PMC_SQL_FIELD_ECHO_ID,         echoId,
                     PMC_SQL_FIELD_ID,              echo_p->detailsId );

                 sql.Update( cmd.get() );
                 mbStrListFree( str_q );
                 goto start;
            }
        }

create:

        if( createNew == TRUE )
        {
            // Create a new details record for this echo
            if( ( echo_p->detailsId = pmcSqlRecordCreate( PMC_SQL_TABLE_ECHO_DETAILS, NIL ) ) == 0 )
            {
                mbDlgError( "Failed to create new echo details record\n" );
                goto exit;
            }

            // Must set the echo ID in the new record
            if( pmcSqlExecInt( PMC_SQL_TABLE_ECHO_DETAILS, PMC_SQL_FIELD_ECHO_ID, echoId, echo_p->detailsId ) == FALSE )
            {
                mbDlgError( "Failed to set echo ID in new record\n" );
                goto exit;
            }

            // Must set the echo ID in the new record
            if( pmcSqlExecInt( PMC_SQL_TABLE_ECHO_DETAILS, PMC_SQL_FIELD_ECHO_ID, echoId, echo_p->detailsId ) == FALSE )
            {
                mbDlgError( "Failed to set echo ID in new record\n" );
                goto exit;
            }

            //                                  0     1     2     3     4     5     6     7    8      9
            mbSprintf( &cmd, "update %s set %s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                                          ",%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                                          ",%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                                          ",%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f,%s=%f"
                            " where %s=%lu"

                    ,PMC_SQL_TABLE_ECHO_DETAILS
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_RV       ,PMC_FLOAT_NOT_SET  // 0
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_AA       ,PMC_FLOAT_NOT_SET  // 1
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LA       ,PMC_FLOAT_NOT_SET  // 2
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVED     ,PMC_FLOAT_NOT_SET  // 3
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVES     ,PMC_FLOAT_NOT_SET  // 4
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_SEPT     ,PMC_FLOAT_NOT_SET  // 5
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_PW       ,PMC_FLOAT_NOT_SET  // 6
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_MI       ,PMC_FLOAT_NOT_SET  // 7
                    ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVEF     ,PMC_FLOAT_NOT_SET  // 8

                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_MRJA     ,PMC_FLOAT_NOT_SET  //  9
                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_VA       ,PMC_FLOAT_NOT_SET  // 10
                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_PEV      ,PMC_FLOAT_NOT_SET  // 11
                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_PAV      ,PMC_FLOAT_NOT_SET  // 12
                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_MG       ,PMC_FLOAT_NOT_SET  // 13
                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_IVRT     ,PMC_FLOAT_NOT_SET  // 14
                    ,PMC_SQL_ECHO_DETAILS_FIELD_MV_EDT      ,PMC_FLOAT_NOT_SET  // 15

                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_AJL      ,PMC_FLOAT_NOT_SET  // 16
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_APHT     ,PMC_FLOAT_NOT_SET  // 17
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_MV       ,PMC_FLOAT_NOT_SET  // 18
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_PG       ,PMC_FLOAT_NOT_SET  // 19
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_MG       ,PMC_FLOAT_NOT_SET  // 20
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_LD       ,PMC_FLOAT_NOT_SET  // 21
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_LV       ,PMC_FLOAT_NOT_SET  // 22
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_VTI      ,PMC_FLOAT_NOT_SET  // 23
                    ,PMC_SQL_ECHO_DETAILS_FIELD_AV_VA       ,PMC_FLOAT_NOT_SET  // 24

                    ,PMC_SQL_ECHO_DETAILS_FIELD_PV_VEL      ,PMC_FLOAT_NOT_SET  // 25
                    ,PMC_SQL_ECHO_DETAILS_FIELD_PV_PAT      ,PMC_FLOAT_NOT_SET  // 26
                    ,PMC_SQL_ECHO_DETAILS_FIELD_PV_GRAD     ,PMC_FLOAT_NOT_SET  // 27
                    ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_SYS     ,PMC_FLOAT_NOT_SET  // 28
                    ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_DIA     ,PMC_FLOAT_NOT_SET  // 29
                    ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_AR      ,PMC_FLOAT_NOT_SET  // 30
                    ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_LAA     ,PMC_FLOAT_NOT_SET  // 31

                    ,PMC_SQL_ECHO_DETAILS_FIELD_RATE        ,PMC_FLOAT_NOT_SET  // 32
                    ,PMC_SQL_ECHO_DETAILS_FIELD_TV_TRJA     ,PMC_FLOAT_NOT_SET  // 33
                    ,PMC_SQL_ECHO_DETAILS_FIELD_TV_RVSP     ,PMC_FLOAT_NOT_SET  // 34
                    ,PMC_SQL_ECHO_DETAILS_FIELD_TV_EV       ,PMC_FLOAT_NOT_SET  // 35
                    ,PMC_SQL_ECHO_DETAILS_FIELD_TV_VA       ,PMC_FLOAT_NOT_SET  // 36

                    ,PMC_SQL_FIELD_ID                       ,echo_p->detailsId );

            sql.Update( cmd.get() );
        }
    }
    else if( count > 1 )
    {
        mbDlgError( "Unexptected count returned\n" );
        goto exit;
    }

    mbSprintf( &cmd, "select %s,%s,%s from %s where %s=%ld"

        ,PMC_SQL_ECHO_DETAILS_NOTES
        ,PMC_SQL_ECHO_DETAILS_INDICATION
        ,PMC_SQL_ECHO_DETAILS_IMAGE_QUALITY

        ,PMC_SQL_TABLE_ECHO_DETAILS
        ,PMC_SQL_FIELD_ID,   echo_p->detailsId );

    if( sql.Query( cmd.get() ) == FALSE ) goto exit;

    if( sql.RowCount() != 1 ) goto exit;

    while( sql.RowGet() )
    {
        mbMallocStr( echo_p->text_p,            sql.String( 0 ) );
        mbMallocStr( echo_p->indication_p,      sql.String( 1 ) );
        mbMallocStr( echo_p->imageQuality_p,    sql.String( 2 ) );
    }

    // Now must read data from record
    //                        0  1  2  3  4  5  6  7  8  9
    mbSprintf( &cmd, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s"
                           ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s"
                           ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s"
                           ",%s,%s,%s,%s,%s,%s,%s,%s,%s,%s"
                           ",%s,%s,%s,%s,%s,%s"
                           " from %s where %s=%ld"

        ,PMC_SQL_FIELD_REFERRING_ID                     // 0
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_RV               // 1
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_AA               // 2
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LA               // 3
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVED             // 4
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVES             // 5
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_SEPT             // 6
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_PW               // 7
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_MI               // 8
        ,PMC_SQL_ECHO_DETAILS_FIELD_CD_LVEF             // 9

        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_REG              // 10
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_STEN             // 11
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_MRJA             // 12
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_VA               // 13
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_PEV              // 14
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_PAV              // 15
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_MG               // 16
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_IVRT             // 17
        ,PMC_SQL_ECHO_DETAILS_FIELD_MV_EDT              // 18

        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_REG              // 19
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_STEN             // 20
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_AJL              // 21
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_APHT             // 22
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_MV               // 23
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_PG               // 24
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_MG               // 25
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_LD               // 26
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_LV               // 27
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_VTI              // 28
        ,PMC_SQL_ECHO_DETAILS_FIELD_AV_VA               // 29

        ,PMC_SQL_ECHO_DETAILS_FIELD_PV_REG              // 30
        ,PMC_SQL_ECHO_DETAILS_FIELD_PV_VEL              // 31
        ,PMC_SQL_ECHO_DETAILS_FIELD_PV_PAT              // 32
        ,PMC_SQL_ECHO_DETAILS_FIELD_PV_GRAD             // 33
        ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_SYS             // 34
        ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_DIA             // 35
        ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_AR              // 36
        ,PMC_SQL_ECHO_DETAILS_FIELD_PVF_LAA             // 37

        ,PMC_SQL_ECHO_DETAILS_FIELD_TV_REG              // 38
        ,PMC_SQL_ECHO_DETAILS_FIELD_TV_STEN             // 39
        ,PMC_SQL_ECHO_DETAILS_FIELD_TV_TRJA             // 40
        ,PMC_SQL_ECHO_DETAILS_FIELD_TV_RVSP             // 41
        ,PMC_SQL_ECHO_DETAILS_FIELD_TV_EV               // 42
        ,PMC_SQL_ECHO_DETAILS_FIELD_TV_VA               // 43

        ,PMC_SQL_ECHO_DETAILS_FIELD_RATE                // 44
        ,PMC_SQL_ECHO_DETAILS_FIELD_RHYTHM              // 45

        ,PMC_SQL_TABLE_ECHO_DETAILS
        ,PMC_SQL_FIELD_ID,   echo_p->detailsId );

    if( sql.Query( cmd.get() ) == FALSE ) goto exit;

    if( sql.RowCount() != 1 ) goto exit;

    while( sql.RowGet() )
    {
        echo_p->referring.id   =  sql.Int32u( 0 );

        echo_p->cd_rv    = sql.Float( 1 );
        echo_p->cd_aa    = sql.Float( 2 );
        echo_p->cd_la    = sql.Float( 3 );
        echo_p->cd_lved  = sql.Float( 4 );
        echo_p->cd_lves  = sql.Float( 5 );
        echo_p->cd_sept  = sql.Float( 6 );
        echo_p->cd_pw    = sql.Float( 7 );
        echo_p->cd_mi    = sql.Float( 8 );
        echo_p->cd_lvef  = sql.Float( 9 );

        mbMallocStr( echo_p->mv_reg_p,      sql.String( 10 ) );
        mbMallocStr( echo_p->mv_sten_p,     sql.String( 11 ) );

        echo_p->mv_mrja  = sql.Float( 12 );
        echo_p->mv_va    = sql.Float( 13 );
        echo_p->mv_pev   = sql.Float( 14 );
        echo_p->mv_pav   = sql.Float( 15 );
        echo_p->mv_mg    = sql.Float( 16 );
        echo_p->mv_ivrt  = sql.Float( 17 );
        echo_p->mv_edt   = sql.Float( 18 );

        mbMallocStr( echo_p->av_reg_p,      sql.String( 19 ) );
        mbMallocStr( echo_p->av_sten_p,     sql.String( 20 ) );

        echo_p->av_ajl   = sql.Float( 21 );
        echo_p->av_apht  = sql.Float( 22 );
        echo_p->av_mv    = sql.Float( 23 );
        echo_p->av_pg    = sql.Float( 24 );
        echo_p->av_mg    = sql.Float( 25 );
        echo_p->av_ld    = sql.Float( 26 );
        echo_p->av_lv    = sql.Float( 27 );
        echo_p->av_vti   = sql.Float( 28 );
        echo_p->av_va    = sql.Float( 29 );

        mbMallocStr( echo_p->pv_reg_p,      sql.String( 30 ) );

        echo_p->pv_vel   = sql.Float( 31 );
        echo_p->pv_pat   = sql.Float( 32 );
        echo_p->pv_grad  = sql.Float( 33 );
        echo_p->pvf_sys  = sql.Float( 34 );
        echo_p->pvf_dia  = sql.Float( 35 );
        echo_p->pvf_ar   = sql.Float( 36 );
        echo_p->pvf_laa  = sql.Float( 37 );

        mbMallocStr( echo_p->tv_reg_p,   sql.String( 38 ) );
        mbMallocStr( echo_p->tv_sten_p,  sql.String( 39 ) );

        echo_p->tv_trja     = sql.Float( 40 );
        echo_p->tv_rvsp     = sql.Float( 41 );
        echo_p->tv_ev       = sql.Float( 42 );
        echo_p->tv_va       = sql.Float( 43 );

        echo_p->rate        = sql.Float( 44 );
        mbMallocStr( echo_p->rhythm_p, sql.String( 45 ) );
    }

    if( echo_p->sono.id && subDetailsFlag == TRUE )
    {
        if( pmcSqlSonographerDetailsGet( echo_p->sono.id, &echo_p->sono ) == FALSE ) goto exit;
    }
    if( echo_p->patient.id && subDetailsFlag == TRUE )
    {
        if( pmcSqlPatientDetailsGet( echo_p->patient.id, &echo_p->patient ) == FALSE ) goto exit;

        if( echo_p->referring.id == 0 &&  echo_p->patient.refDrId != 0 )
        {
            // Set referring dr to patient's default referring dr
            echo_p->referring.id = echo_p->patient.refDrId;
        }
    }
    if( echo_p->referring.id && subDetailsFlag )
    {
        if( pmcSqlDoctorDetailsGet( echo_p->referring.id, &echo_p->referring ) == FALSE ) goto exit;
    }
    if( echo_p->provider.id && subDetailsFlag )
    {
        if( pmcSqlProviderDetailsGet( echo_p->provider.id, &echo_p->provider ) == FALSE ) goto exit;
    }

    // If creating a new details record, and echo is new (i.e., read date 0), then set status to "in progress"
    if( echo_p->readDate == 0 )
    {
        echo_p->readDate = PMC_ECHO_STATUS_IN_PROGRESS;
        pmcSqlExecInt( PMC_SQL_TABLE_ECHOS, PMC_SQL_ECHOS_FIELD_READ_DATE, echo_p->readDate, echoId );
    }

    returnCode = TRUE;

exit:

    if( gotLock ) pmcSqlRecordUnlock( PMC_SQL_TABLE_ECHOS, echoId );

    if( returnCode != TRUE )
    {
        pmcSqlEchoDetailsFree( echo_p );
        echo_p = NIL;
    }
    return echo_p;
}

//---------------------------------------------------------------------------
// Function: pmcSqlDoctorDetailsGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t    pmcSqlDoctorDetailsGet
(
    Int32u_t        id,
    PmcSqlDoctor_p  dr_p
)
{
    Char_p          buf_p = NIL;
    Int32s_t        returnCode = FALSE;
    MbSQL           sql;
    MbString        buf = MbString();

    if( dr_p == NIL ) goto exit;
    memset( dr_p, 0, sizeof( PmcSqlDoctor_t ) );
    if( id == 0 ) goto exit;

    if( mbCalloc( buf_p, 1024 ) == NIL ) goto exit;

    // Format SQL command
    //                        0  1  2  3  4  5  6  7  8  9 10 11 12 13
    mbSprintf( &buf, "select %s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_PROVINCE,                 // 2
        PMC_SQL_DOCTORS_FIELD_OTHER_NUMBER,     // 3
        PMC_SQL_DOCTORS_FIELD_DOCTOR_NUMBER,    // 4
        PMC_SQL_DOCTORS_FIELD_CANCER_CLINIC,    // 5
        PMC_SQL_DOCTORS_FIELD_ON_MSP_LIST,      // 6
        PMC_SQL_DOCTORS_FIELD_WORK_PHONE,       // 7
        PMC_SQL_DOCTORS_FIELD_WORK_FAX,         // 8
        PMC_SQL_FIELD_ADDRESS1,                 // 9
        PMC_SQL_FIELD_ADDRESS2,                 // 10
        PMC_SQL_FIELD_CITY,                     // 11
        PMC_SQL_FIELD_COUNTRY,                  // 12
        PMC_SQL_FIELD_POSTAL_CODE,              // 13

        PMC_SQL_TABLE_DOCTORS,
        PMC_SQL_FIELD_ID, id );

    if( sql.Query( buf.get() ) == FALSE ) goto exit;
    if( sql.RowCount( ) != 1 ) goto exit;
    if( sql.RowGet( ) == FALSE ) goto exit;

    strncpy( dr_p->lastName,    sql.String(  0 ), PMC_MAX_NAME_LEN );
    strncpy( dr_p->firstName,   sql.String(  1 ), PMC_MAX_NAME_LEN );
    strncpy( dr_p->province,    sql.String(  2 ), PMC_MAX_PROV_LEN );
    strncpy( dr_p->otherNumber, sql.String(  3 ), PMC_MAX_OTHER_NUMBER_LEN );
    strncpy( dr_p->phone,       sql.String(  7 ), PMC_MAX_PHONE_LEN );
    strncpy( dr_p->fax,         sql.String(  8 ), PMC_MAX_PHONE_LEN );
    strncpy( dr_p->address1,    sql.String(  9 ), PMC_MAX_ADDRESS_LEN);
    strncpy( dr_p->address2,    sql.String( 10 ), PMC_MAX_ADDRESS_LEN );
    strncpy( dr_p->city,        sql.String( 11 ), PMC_MAX_CITY_LEN );
    strncpy( dr_p->country,     sql.String( 12 ), PMC_MAX_COUNTRY_LEN );
    strncpy( dr_p->postalCode,  sql.String( 13 ), PMC_MAX_POSTAL_CODE_LEN );

    dr_p->mspNumber     = sql.Int32u( 4 );
    dr_p->cancerClinic  = ( sql.Int32u( 5 ) > 0 ) ? TRUE : FALSE;
    dr_p->mspActive     = sql.Int32u( 6 );
    dr_p->id            = id;

    pmcPhoneFormatString( dr_p->phone, dr_p->formattedPhone );
    pmcPhoneFormatString( dr_p->fax, dr_p->formattedFax );

    sprintf( dr_p->address3, "%s, %s  %s", dr_p->city, dr_p->province, dr_p->postalCode );

    // Tighten up the address (makes for cleaner display in letters
    if( strlen( dr_p->address1 ) == 0 && strlen( dr_p->address2 ) > 0 )
    {
        strcpy(  dr_p->address1,  dr_p->address2 );
        strcpy(  dr_p->address2,  dr_p->address3 );
        *dr_p->address3 = 0;
    }

    if( strlen( dr_p->address2 ) == 0 && strlen( dr_p->address3 ) > 0 )
    {
        strcpy(  dr_p->address2,  dr_p->address3 );
        *dr_p->address3 = 0;
    }

    returnCode = TRUE;

exit:

    if( returnCode != TRUE && id != 0 )
    {
        mbDlgDebug(( "Error locating doctor id %ld in database (records: %u)", id, sql.RowCount( ) ));
    }

    return returnCode;
}

//---------------------------------------------------------------------------
// Function: pmcSqlProviderDetailsGet
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t    pmcSqlProviderDetailsGet
(
    Int32u_t            id,
    PmcSqlProvider_p    provider_p
)
{
    MbString            buf;
    Int32s_t            returnCode = FALSE;
    MbSQL               sql;

    if( id == 0 ) goto exit;
    if( provider_p == NIL ) goto exit;
    memset( provider_p, 0, sizeof( PmcSqlProvider_t ) );

    // Format SQL command
    mbSprintf( &buf, "select %s,%s,%s,%s,%s from %s where %s=%ld",
        PMC_SQL_FIELD_LAST_NAME,                // 0
        PMC_SQL_FIELD_FIRST_NAME,               // 1
        PMC_SQL_FIELD_DESC,                     // 2
        PMC_SQL_PROVIDERS_FIELD_PROVIDER_NUMBER,// 3
        PMC_SQL_PROVIDERS_FIELD_CLINIC_NUMBER,  // 4
        PMC_SQL_TABLE_PROVIDERS,
        PMC_SQL_FIELD_ID, id );

    if( sql.Query( buf.get( ) ) == FALSE ) goto exit;
    if( sql.RowCount( ) != 1 ) goto exit;
    if( sql.RowGet( ) == FALSE ) goto exit;

    strncpy( provider_p->lastName, sql.String( 0 ), PMC_MAX_NAME_LEN );
    strncpy( provider_p->firstName, sql.String( 1 ), PMC_MAX_NAME_LEN );
    strncpy( provider_p->description, sql.String( 2 ), PMC_MAX_DESC_LEN );

    provider_p->billingNumber = sql.Int32u( 3 );
    provider_p->clinicNumber = sql.Int32u( 4 );

    provider_p->id = id;

    returnCode = TRUE;

exit:

    if( returnCode != TRUE )
    {
        mbDlgDebug(( "Error locating provider id %ld in database (records: %d)", id, sql.RowCount( ) ));
    }
    return returnCode;
}








