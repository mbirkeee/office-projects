//---------------------------------------------------------------------------
// File: mbSql.cpp
//---------------------------------------------------------------------------
// Date: May 5, 2006
//---------------------------------------------------------------------------
// Description:
//
// Library for MySQL support
//---------------------------------------------------------------------------

#include <stdio.h>
#pragma hdrstop

#include "mysql.h"
#include "errmsg.h"
#include "mbUtils.h"
#include "mbSqlLib.h"
#include "mbSqlFormLogin.h"

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

qHead_t     gMbMySqlQueueHead;
qHead_p     gMbMySql_q;
Boolean_t   gMbMysqlInitFlag = FALSE;
Char_p      gMbMysqlNilString_p = "";
Int32u_t    gMbMysqlConnectionIndex;
Char_p      gMbMysqlUsername_p;
Char_p      gMbMysqlPassword_p;
Char_p      gMbMysqlHost_p;
Char_p      gMbMysqlDatabase_p;
Int32u_t    gMbMysqlPort;
Int32u_t    gMbMysqlConnectionsTotal;
Boolean_t   gMbMysqlTerminateRequestFlag;
Boolean_t   gMbMysqlLogFlag;


Int32s_t    mbSqlConnectionCreate( Boolean_t promptFlag );
Int32s_t    mbSqlConnectionDelete( PmcMbSql_p sql_p );
Int32s_t    mbSqlConnectionPut( PmcMbSql_p sql_p );
PmcMbSql_p  mbSqlConnectionGet( Void_t );

//---------------------------------------------------------------------------
// Initialize the library
//---------------------------------------------------------------------------

Int32s_t    mbSqlInit(  Char_p      host_p,
                        Int32u_t    port,
                        Char_p      database_p,
                        Char_p      username_p,
                        Char_p      password_p,
                        Boolean_t   logSQLFlag )
{
    Int32u_t                rv = MB_SQL_ERR_INVALID;
    Int32s_t                i;
    Boolean_t               prompt = TRUE;

    gMbMysqlHost_p = NIL;
    gMbMysqlUsername_p = NIL;
    gMbMysqlPassword_p = NIL;
    gMbMysqlDatabase_p = NIL;

    gMbMysqlConnectionIndex = 0;
    gMbMysqlConnectionsTotal = 0;
    gMbMysqlTerminateRequestFlag = FALSE;
    gMbMysqlLogFlag = logSQLFlag;

    gMbMySql_q = qInitialize( &gMbMySqlQueueHead );

    // Sanity checks
    if( database_p == NIL )
    {
        mbDlgError( "No MySQL database specified" );
        goto exit;
    }

    if( host_p == NIL )
    {
        mbDlgError( "No MySQL host specified" );
        goto exit;
    }

    if( strlen( database_p ) == 0 )
    {
        mbDlgError( "No MySQL database specified" );
        goto exit;
    }

    if( strlen( host_p ) == 0 )
    {
        mbDlgError( "No MySQL host specified" );
        goto exit;
    }

    if( port == 0 )
    {
        mbDlgError( "No MySQL port specified" );
        goto exit;
    }

    rv = MB_SQL_ERR_ALLOC;

    gMbMysqlPort = port;
    mbMallocStr( gMbMysqlHost_p, host_p );
    mbMallocStr( gMbMysqlDatabase_p, database_p );

    gMbMysqlUsername_p = NIL;
    if( username_p )
    {
        mbMallocStr( gMbMysqlUsername_p, username_p );
    }

    gMbMysqlPassword_p = NIL;
    if( password_p )
    {
        mbMallocStr( gMbMysqlPassword_p, password_p );
        if( strlen( password_p ) ) prompt = FALSE;
    }

    mbLog( "called host: '%s' port: %d database: '%s\n", host_p, port, database_p );

    // Make multiple connections to the SQL database
    for( i = 0 ; i < 5 ; i++ )
    {
        if( ( rv = mbSqlConnectionCreate( prompt ) ) != MB_SQL_ERR_OK ) goto exit;
        prompt = FALSE;
    }

exit:

    gMbMysqlInitFlag = TRUE;
    if( rv != MB_SQL_ERR_OK )
    {
        mbSqlTerminate( );
    }
    return rv;
}

//---------------------------------------------------------------------------
// mbSqlConnectionAdd
//---------------------------------------------------------------------------

Int32s_t    mbSqlConnectionCreate( Boolean_t promptFlag )
{
    PmcMbSql_p              sql_p;
    MbSqlFormLoginInfo_t    info;
    Int32s_t                rv = MB_SQL_ERR_ALLOC;

    if( mbCalloc( sql_p, sizeof( PmcMbSql_t ) ) == NIL ) goto exit;

    if( ( sql_p->mysql_p = mysql_init( NULL ) ) == NIL ) goto exit;

    for( ; ; )
    {
        if( promptFlag )
        {
            TFormLogin              *login_p;

            memset( &info, 0, sizeof( MbSqlFormLoginInfo_t ) );

            if( gMbMysqlDatabase_p )
            {
               strncpy( info.database, gMbMysqlDatabase_p, MB_SQL_MAX_LEN_DATABASE - 1 );
            }

            if( gMbMysqlUsername_p )
            {
                strncpy( info.username, gMbMysqlUsername_p, MB_SQL_MAX_LEN_USERNAME - 1 );
            }

            // Show the login form
            login_p = new TFormLogin( &info );
            login_p->ShowModal( );
            delete login_p;

            if( info.returnCode == MB_BUTTON_CANCEL )
            {
                // User clicked "cancel" or form close
                rv = MB_SQL_ERR_CANCEL;
                goto exit;
            }
            else
            {
                mbFree( gMbMysqlUsername_p );
                mbFree( gMbMysqlPassword_p );
                mbMallocStr( gMbMysqlUsername_p, info.username );
                mbMallocStr( gMbMysqlPassword_p, info.password );
            }
        }

        if( mysql_real_connect( sql_p->mysql_p,
                                gMbMysqlHost_p,
                                gMbMysqlUsername_p,
                                gMbMysqlPassword_p,
                                gMbMysqlDatabase_p,
                                gMbMysqlPort, NULL, 0 ) == NIL )
        {
            Char_t  buf[1024];

            sprintf( buf, "MySQL Error: '%s'", mysql_error( sql_p->mysql_p ) );
            mbDlgInfo( buf );

            rv = MB_SQL_ERR_CONNECT;
            if( promptFlag == FALSE )
            {
                goto exit;
            }
            else
            {
                mbDlgInfo( "Login unsuccessful.\nEnsure the username and password are correct\n" );
            }
        }
        else
        {
            sql_p->index = gMbMysqlConnectionIndex++;
            gMbMysqlConnectionsTotal++;
            mbLog( "Con %d created\n", sql_p->index );

            mysql_query( sql_p->mysql_p, "set sql_mode=''" );

            rv = MB_SQL_ERR_OK;
            mbLockAcquire( gMbMySql_q->lock );
            qInsertLast( gMbMySql_q, sql_p );
            mbLockRelease( gMbMySql_q->lock );
            sql_p = NIL;
            break;
        }
    }
exit:

    if( sql_p ) mbFree( sql_p );
    return rv;
}

//---------------------------------------------------------------------------
// mbSqlConnectionDelete
//---------------------------------------------------------------------------

Int32s_t  mbSqlConnectionDelete( PmcMbSql_p sql_p )
{
    if( sql_p )
    {
        mbLog( "Con %d deleted\n", sql_p->index );
        if( sql_p->result_p )
        {
            mysql_free_result( sql_p->result_p );
        }
        if( sql_p->mysql_p )
        {
            mysql_close( sql_p->mysql_p );
        }
        mbFree( sql_p );
        gMbMysqlConnectionsTotal--;
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// mbSqlConnectionDelete
//---------------------------------------------------------------------------

PmcMbSql_p  mbSqlConnectionGet( Void_t )
{
    PmcMbSql_p      sql_p = NIL;
    Int32s_t        i;

    mbLockAcquire( gMbMySql_q->lock );

    for( i = 0 ; i < 10 ; i++ )
    {
        if( gMbMysqlConnectionsTotal < 5 )
        {
            mbLockRelease( gMbMySql_q->lock );
            mbSqlConnectionCreate( FALSE );
            mbLockAcquire( gMbMySql_q->lock );
        }
    }

    // First, get a connection from the connection pool
    if( !qEmpty( gMbMySql_q ) )
    {
        sql_p = (PmcMbSql_p)qRemoveFirst( gMbMySql_q );
        sql_p->result_p = NIL;
        sql_p->numFields = 0;
    }
    else
    {
        mbDlgExclaim( "Error getting MySQL connection from connection pool" );
    }
    mbLockRelease( gMbMySql_q->lock );
    return sql_p;
}


//---------------------------------------------------------------------------
// mbSqlConnectionDelete
//---------------------------------------------------------------------------

Int32s_t  mbSqlConnectionPut( PmcMbSql_p sql_p )
{
    if( sql_p )
    {
        if( sql_p->result_p )
        {
            mysql_free_result( sql_p->result_p );
            sql_p->result_p = NIL;
        }
        mbLockAcquire( gMbMySql_q->lock );
        qInsertLast( gMbMySql_q, sql_p );
        mbLockRelease( gMbMySql_q->lock );
        return TRUE;
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// mbSqlTerminate
//---------------------------------------------------------------------------

Void_t    mbSqlTerminateRequest(  )
{
    gMbMysqlTerminateRequestFlag = TRUE;
    return;
}

//---------------------------------------------------------------------------
// mbSqlTerminate
//---------------------------------------------------------------------------

Int32s_t    mbSqlTerminate(  )
{
    PmcMbSql_p              sql_p;

    if( gMbMysqlInitFlag == TRUE )
    {
        gMbMysqlInitFlag = FALSE;
        mbLockAcquire( gMbMySql_q->lock );
        while( !qEmpty( gMbMySql_q ) )
        {
            sql_p = (PmcMbSql_p)qRemoveFirst( gMbMySql_q );

            mbSqlConnectionDelete( sql_p );
        }
        mbLockRelease( gMbMySql_q->lock );

        mbFree( gMbMysqlHost_p );
        mbFree( gMbMysqlUsername_p );
        mbFree( gMbMysqlPassword_p );
        mbFree( gMbMysqlDatabase_p );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Class: MbSQL Constuctor
//---------------------------------------------------------------------------
__fastcall MbSQL::MbSQL( Char_p query_p )
{
    Init( );
    Query( query_p );
}
//---------------------------------------------------------------------------
// Class: MbSQL Constuctor
//---------------------------------------------------------------------------
__fastcall MbSQL::MbSQL( )
{
    Init( );
}

//---------------------------------------------------------------------------
// Class: MbSQL Object initialization
//---------------------------------------------------------------------------

Boolean_t __fastcall MbSQL::Init( Void_t )
{
    Boolean_t   result = TRUE;

    if( ( mSql_p = mbSqlConnectionGet( ) ) == NIL )
    {
        result = FALSE;
    }

    return result;
}

//---------------------------------------------------------------------------
// MbSQL::Update
//---------------------------------------------------------------------------

Boolean_t __fastcall MbSQL::Update( Char_p cmd_p )
{
    return Execute( cmd_p, FALSE );
}

//---------------------------------------------------------------------------
// MbSQL::Query()
//---------------------------------------------------------------------------
Boolean_t __fastcall MbSQL::Query( Char_p cmd_p )
{
    return Execute( cmd_p, TRUE );
}

//---------------------------------------------------------------------------
// MbSQL::Execute()
//---------------------------------------------------------------------------
Boolean_t __fastcall MbSQL::Execute( Char_p cmd_p, Boolean_t queryFlag )
{
    Boolean_t   result = FALSE;
    Int32s_t    i;

    try
    {

    // Sanity Check
    if( mSql_p == NIL || cmd_p == NIL ) goto exit;

    // Free previous results if necessary
    if( mSql_p->result_p )
    {
       mysql_free_result( mSql_p->result_p );
       mSql_p->result_p= NIL;
       mSql_p->numFields = 0;
    }

    if( gMbMysqlLogFlag )
    {
        if( strcmp( cmd_p, "show table status" ) != 0 )
        {
            mbLog( "Con %d cmd: '%s'\n", mSql_p->index, cmd_p );
        }
    }

start:
    for( i = 0 ; i < 5 ; i++ )
    {
        if(  gMbMysqlTerminateRequestFlag == TRUE ) return FALSE;

        if( mSql_p == NIL )
        {
            if( ( mSql_p = mbSqlConnectionGet( ) ) == NIL ) goto exit;
        }

        if( i > 0 )
        {
            mbLog( "Attempt: %d Con %d cmd: '%s'\n", i+1, mSql_p->index, cmd_p );
        }

        // Actually send mysql command to server
        if( mysql_query( mSql_p->mysql_p, cmd_p ) != 0 )
        {
            if( gMbMysqlTerminateRequestFlag == TRUE ) return FALSE;

            mbLog( "Con %d mysql_query( %s ) error: %d: %s\n", mSql_p->index, cmd_p,
                mysql_errno( mSql_p->mysql_p ), mysql_error( mSql_p->mysql_p ) );

            if( mysql_errno(  mSql_p->mysql_p ) == CR_COMMANDS_OUT_OF_SYNC )
            {
                // Throw away the connection...
                mbSqlConnectionDelete( mSql_p );
                mSql_p = NIL;
            }
            else
            {
                if( mysql_ping( mSql_p->mysql_p ) == 0 )
                {
                    mbLog( "Con %d reconnected\n", mSql_p->index );
                }
                else
                {
                    mbLog( "Con %d msql_ping( ) failed\n",  mSql_p->index );
                    Sleep( 100 );
                }
            }
        }
        else
        {
            if( gMbMysqlTerminateRequestFlag == TRUE ) return FALSE;

            if( queryFlag )
            {
                if( ( mSql_p->result_p = mysql_store_result( mSql_p->mysql_p ) ) != NIL )
                {
                    mSql_p->numFields = mysql_num_fields( mSql_p->result_p );
                    result = TRUE;
                    break;
                }
                else
                {
                    mbLog( "Con %d mysql_store_result( ) error: %s\n", mSql_p->index, mysql_error( mSql_p->mysql_p ) );
                }
            }
            else
            {
                result = TRUE;
                break;
            }
        }
    }

exit:
    if( result == FALSE )
    {
        mbLog( "Con %d %s() failed: %s\n", mSql_p->index, queryFlag ? "Query" : "Update", cmd_p );
        mbDlgInfo( "Database connection lost. Click OK to reconnect.\n\n(Command: %s)  ", cmd_p );

        goto start;
     }

     }
     catch( Exception &exception )
     {
        mbLog( "EXCEPTION IN MbSQL::Execute()\n" );
     }
    return result;
}


//---------------------------------------------------------------------------
// MbSQL::RowCount()
//---------------------------------------------------------------------------
Int32u_t __fastcall MbSQL::RowCount( Void_t )
{
    Int64u_t    rowCount = 0;
    Int32u_t    row32 = 0;

    if( mSql_p )
    {
        if( mSql_p->result_p )
        {
            rowCount = mysql_num_rows( mSql_p->result_p );
        }
        else
        {
            mbDlgExclaim( "Can't get rows when result_p = NIL" );
        }
    }
    if( rowCount > 0x00000000FFFFFFFFi64 )
    {
        mbDlgExclaim( "Row count > 0xFFFFFFFF\n" );
    }
    else
    {
        row32 = (Int32u_t)rowCount;
    }
    return row32;
}

//---------------------------------------------------------------------------
// MbSQL::DateTimeInt64u
//---------------------------------------------------------------------------

Int64u_t __fastcall MbSQL::DateTimeInt64u( Int32s_t index )
{
    Int64u_t    result = 0i64;

    if( index < 0 ) goto exit;
    if( mSql_p )
    {
        if( mSql_p->row )
        {
            MbDateTime  dateTime;

            dateTime.MysqlStringSet( mSql_p->row[index] );
            result = dateTime.Int64( );
        }
    }
exit:    
    return result;
}

//---------------------------------------------------------------------------
// MbSQL::Int32u()
//---------------------------------------------------------------------------

Int32u_t __fastcall MbSQL::Int32u( Int32s_t index )
{
    Int32u_t    value = 0;
    if( index < 0 ) goto exit;
    if( mSql_p )
    {
        if( mSql_p->row )
        {
            if( mSql_p->row[index] )
            {
                value = strtoul( mSql_p->row[index], NIL, 10 );
            }
        }
    }
exit:
    return value;
}

//---------------------------------------------------------------------------
// MbSQL::Float()
//---------------------------------------------------------------------------

Float_t __fastcall MbSQL::Float( Int32s_t index )
{
    Float_t    value = 0.0;
    if( index < 0 ) goto exit;
    if( mSql_p )
    {
        if( mSql_p->row )
        {
            if( mSql_p->row[index] )
            {
                value = atof( mSql_p->row[index] );
            }
        }
    }
exit:
    return value;
}

//---------------------------------------------------------------------------
// MbSQL::IndexGet()
//---------------------------------------------------------------------------

Int32s_t __fastcall MbSQL::IndexGet( Char_p name_p )
{
    Int32s_t        index = -1;
    Int32s_t        numFields;
    Int32s_t        i;
    MYSQL_FIELD    *field_p;

    if( mSql_p == NIL ) goto exit;

    if( mSql_p->result_p == NIL || name_p == NIL ) goto exit;

    for( i = 0 ; i < mSql_p->numFields ; i++ )
    {
        field_p = mysql_fetch_field_direct( mSql_p->result_p, i );
        if( strcmp( field_p->name, name_p ) == 0 )
        {
            index = i;
            break;
        }
    }
exit:
    return index;
}

//---------------------------------------------------------------------------
// MbSQL::NmString()
//---------------------------------------------------------------------------
Char_p __fastcall MbSQL::NmString( Char_p name_p )
{
    return String( IndexGet( name_p ) );
}

//---------------------------------------------------------------------------
// MbSQL::NmInt32u()
//---------------------------------------------------------------------------
Int32u_t __fastcall MbSQL::NmInt32u( Char_p name_p )
{
    return Int32u( IndexGet( name_p ) );
}

//---------------------------------------------------------------------------
// MbSQL::NmFloat()
//---------------------------------------------------------------------------
Float_t __fastcall MbSQL::NmFloat( Char_p name_p )
{
    return Float( IndexGet( name_p ) );
}

//---------------------------------------------------------------------------
// MbSQL::NmDateTimeInt64u()
//---------------------------------------------------------------------------
Int64u_t __fastcall MbSQL::NmDateTimeInt64u( Char_p name_p )
{
    return DateTimeInt64u( IndexGet( name_p ) );
}

//---------------------------------------------------------------------------
// MbSQL::String()
//---------------------------------------------------------------------------
Char_p __fastcall MbSQL::String( Int32s_t index )
{
    Char_p  return_p = NIL;

    if( index < 0 ) goto exit;
    if( mSql_p )
    {
        if( mSql_p->row )
        {
            return_p = mSql_p->row[index];
        }
    }

exit:
    // Never want to return a NIL pointer
    if( return_p == NIL ) return_p = gMbMysqlNilString_p;
    return return_p;
}

//---------------------------------------------------------------------------
// MbSQL::RowGet()
//---------------------------------------------------------------------------
Boolean_t __fastcall MbSQL::RowGet( Void_t )
{
    Boolean_t   result = FALSE;

    if( mSql_p )
    {
        if( mSql_p->result_p )
        {
            if( ( mSql_p->row = mysql_fetch_row( mSql_p->result_p ) ) != NIL ) result = TRUE;
        }
    }
exit:
    return result;
}

//---------------------------------------------------------------------------
// Class: MbSQL Destructor
//---------------------------------------------------------------------------

__fastcall MbSQL::~MbSQL( )
{
    // Return connection to connection pool
    mbSqlConnectionPut( mSql_p );
    return;
}

//---------------------------------------------------------------------------
// End of Class: MbSQL
//---------------------------------------------------------------------------

