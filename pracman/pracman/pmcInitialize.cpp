//---------------------------------------------------------------------------
// File:    pmcInitialize.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    April 4, 2001
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

#include <stdio.h>
#include <vcl.h>
#include <time.h>
#include <dir.h>
#include <process.h>
#include <Filectrl.hpp>
#pragma hdrstop

#include "mbUtils.h"
#include "mbSqlLib.h"

#include "pmcUtils.h"
#include "pmcInitialize.h"
#include "pmcMainForm.h"
#include "pmcGlobals.h"
#include "pmcDocumentEditForm.h"
#include "pmcDocumentListForm.h"
#include "pmcClaimEditForm.h"
#include "pmcBatchImportForm.h"
#include "pmcProviderView.h"
#include "pmcFormPickList.h"

#pragma package(smart_init)

extern void pmcTableStatusUpdate( Char_p tableName_p );

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcInitialize
(
    void
)
{
    Char_p          fileName_p;
    Char_p          buf_p;
    Char_p          buf2_p;
    FILE           *fp;
    Ints_t          len, i;
    AnsiString      str = "";
    Int32s_t        returnCode = FALSE;
    Int32s_t        rv;
    Boolean_t       mallocTrack = FALSE;
    qHead_t         mysqlQueue;
    qHead_p         mysql_q;
    pmcMysqlItem_p  mysql_p = NIL;
    Char_t          winUserName[128];
    Int32u_t        bufSize;

    mysql_q = qInitialize( &mysqlQueue );

#if PMC_TRACK_MALLOC
    mallocTrack = TRUE;
#endif

    pmcSystemId = 1;
    pmcStartTime = (Int32u_t)time( 0 );
    randomize( );
    pmcSqlLockValue = rand( );

    pmcDBConfig_q = NIL;
    pmcDBMed_q = NIL;
    
    pmcIcdContainsOhs = FALSE;
    pmcFeeContainsOhs = FALSE;

    fp = fopen( "malloc_track", "r" );
    if( fp  )
    {
        mallocTrack = TRUE;
        fclose( fp );
        fp = NIL;
    }

    // Initialize memory management functions. TRUE/FALSE: keep list of chunks
    mbMallocInit( mallocTrack );

    // Initialize the dialog system. TRUE/FALSE: log the dialogs
    mbDlgInit( PMC_NAME, TRUE );

    // Initialize CRC table
    mbCrcInit( );

    // Initialize high precision timer
    mbMsec( );

    // Initialize queue of local telephone exchanges
    gLocalExchange_q      = qInitialize( &gLocalExchange );

    // List of patient history types
    gPatHistoryType_q     = qInitialize( &gPatHistoryTypeQueue );

    // Initialze list of MCIB fees
    pmcFee_q                = qInitialize( &pmcFeeQueue    );
    pmcOldFee_q             = qInitialize( &pmcOldFeeQueue );

    // Initialze list of ICD codes
    pmcIcd_q                = qInitialize( &pmcIcdQueueHead );

    // Initialize list of "single" ICD codes
    pmcIcdSingle_q          = qInitialize( &pmcIcdSingleQueueHead );

    // Initialze pick list queue
    pmcPickListCfg_q        = qInitialize( &pmcPickListCfgQueueHead );

    // Initialize queue of EXP codes
    pmcExp_q                = qInitialize( &pmcExpQueueHead );

    // Initialize the queue of cut appointments
    pmcAppointCutBuf_q      = qInitialize( &pmcAppointCutBufHead );

    // Queue of providers to show in provider view
    pmcProviderView_q       = qInitialize( &pmcProviderViewListHead );


    // Initialize the queue of doctor records
    pmcDocName_q            = qInitialize( &pmcDocNameQueue );
    pmcDocNum_q             = qInitialize( &pmcDocNumQueue );
    pmcDocPhone_q           = qInitialize( &pmcDocPhoneQueue );
    pmcDocDelete_q          = qInitialize( &pmcDocDeleteQueue );

    // Initialize the queue of patient records
    pmcPatName_q            = qInitialize( &pmcPatNameQueue );
    pmcPatDelete_q          = qInitialize( &pmcPatDeleteQueue );
    pmcPatPhone_q           = qInitialize( &pmcPatPhoneQueue );
    pmcPatPhn_q             = qInitialize( &pmcPatPhnQueue );

    // Initialize queue of providers
    pmcProvider_q           = qInitialize( &pmcProviderListHead );

    // Initialize list of created documents.  This list is used to delete
    // created documents that are not edited in any way.
    pmcCreatedDocument_q    = qInitialize( &pmcCreatedDocumentQueueHead );

    // List of documents
    pmcDocument_q           = qInitialize( &pmcDocumentQueue );
    pmcDocumentDeleted_q    = qInitialize( &pmcDocumentDeletedQueue );

    // List of non-modal patient form windows
    pmcPatientForm_q        = qInitialize( &pmcPatientFormQueueHead );
    mbLockInit( pmcPatientFormListLock );

#if PMC_AVAIL_TIME
    // Initialize queue of available times
    pmcAvailTime_q      = qInitialize( &pmcAvailTimeListHead );
#endif

    mbLockInit( pmcTestLock );

    mbLockInit( pmcPatListLock );
    mbLockInit( pmcDocListLock );

    mbMalloc( pmcBuf1_p, 2048 );
    mbMalloc( pmcBuf2_p, 2048 );

    mbMalloc( buf_p, 512 );
    mbMalloc( buf2_p, 512 );
    mbMalloc( fileName_p, 256 );

    mbMalloc( pmcDefaultIcdCode_p, 256 );

    mbMalloc( pmcClaimsin_p,  128 );
    mbMalloc( pmcInfo_p,      128 );
    mbMalloc( pmcValidrpt_p,  128 );
    mbMalloc( pmcNotice_p ,   128 );
    mbMalloc( pmcReturns_p,   128 );

    pmcHomeDir[0] = 0;
    getcwd( pmcHomeDir, 256 );

    // Load the property file
    if( pmcHomeDir ) sprintf( fileName_p, "%s\\", pmcHomeDir );

    // Prepend user name to prefs file (someday replace prefs with some
    // sort of registry
    if( GetUserName( winUserName, &bufSize ) == TRUE )
    {
        strcat( fileName_p, winUserName );
        strcat( fileName_p, "_" );
    }

    strcat( fileName_p, PMC_PROPERTY_FILE );
    mbPropertyInit( fileName_p );
    mbPropertyLoad( );

    // Load the initializtion file
    if( pmcHomeDir ) sprintf( fileName_p, "%s\\", pmcHomeDir );
    strcat( fileName_p, PMC_INIT_FILE );

    fp = fopen( fileName_p, "r" );
    if( fp == NIL )
    {
        mbDlgExclaim( "Could not open initialization file '%s'", fileName_p );
        goto exit;
    }

    // Look for hostname and log directory first
    while( fgets( buf_p, 512, fp ) != 0 )
    {
        mbStrClean( buf_p, buf2_p, FALSE );
        len = strlen( buf2_p );
        // Look for "comments". (Cannot accept a # character anywhere)
        for( i = 0 ; i < len ; i++ )
        {
            if( *(buf2_p+i) == '#') *(buf2_p+i) = 0;
        }

        if( mbStrPos( buf2_p, pmcCfg[CFG_HOSTNAME].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_HOSTNAME].key_p );
            mbMalloc( pmcCfg[CFG_HOSTNAME].str_p, strlen( buf2_p+len ) + 1 );
            mbStrClean( buf2_p+len, pmcCfg[CFG_HOSTNAME].str_p, TRUE );
            pmcCfg[CFG_HOSTNAME].init = TRUE;
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_LOG_DIR].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_LOG_DIR].key_p );
            mbMalloc( pmcCfg[CFG_LOG_DIR].str_p, strlen( buf2_p+len ) + 1 );
            mbStrClean( buf2_p+len, pmcCfg[CFG_LOG_DIR].str_p, FALSE );
            pmcCfg[CFG_LOG_DIR].init = TRUE;
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_DESC].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_DATABASE_DESC].key_p );
            mbCalloc( mysql_p, sizeof(pmcMysqlItem_t) );
            qInsertLast( mysql_q, mysql_p );
            mbStrClean( buf2_p + len, mysql_p->desc, TRUE );
            mysql_p->local = TRUE;
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_MYSQL].key_p ) == 0 )
        {
           len = strlen( pmcCfg[CFG_DATABASE_MYSQL].key_p );
           if( mysql_p ) mbStrClean( buf2_p + len, mysql_p->mysql, TRUE );
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_HOST].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_DATABASE_HOST].key_p );
            if( mysql_p )  mbStrClean( buf2_p + len, mysql_p->host, TRUE );
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_PORT].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_DATABASE_PORT].key_p );
            if( mysql_p ) mysql_p->port = atol( mbStrClean( buf2_p+len, NIL, TRUE ) );
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_USERNAME].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_DATABASE_USERNAME].key_p );
            if( mysql_p )  mbStrClean( buf2_p+len, mysql_p->username, TRUE );
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_PASSWORD].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_DATABASE_PASSWORD].key_p );
            if( mysql_p ) mbStrClean( buf2_p+len, mysql_p->password, TRUE );
        }
        else if( mbStrPos( buf2_p, pmcCfg[CFG_DATABASE_LOCAL].key_p ) == 0 )
        {
            len = strlen( pmcCfg[CFG_DATABASE_LOCAL].key_p );
            if( mysql_p )
            {
                mbStrToUpper( buf2_p+len );
                mysql_p->local = ( mbStrPos( buf2_p+len, "TRUE" ) >= 0 ) ? TRUE : FALSE;
            }
        }
    }

    rewind( fp );

    // Initialize logging system
    mbLogInit( pmcCfg[CFG_HOSTNAME].str_p, pmcCfg[CFG_LOG_DIR].str_p, pmcSqlLockValue );
    mbLog( "Initializing...\n" );

    while( fgets( buf_p, 512, fp ) != 0 )
    {
        mbStrClean( buf_p, buf2_p, FALSE );
        len = strlen( buf2_p );
        // Look for "comments". (Cannot accept a # character anywhere)
        for( i = 0 ; i < len ; i++ )
        {
            if( *(buf2_p+i) == '#') *(buf2_p+i) = 0;
        }

        nbDlgDebug(( "Read line '%s'", buf2_p ));

        // Check for config items in new config array
        for( i = 0 ; i < CFG_INVALID ; i++ )
        {
            // The keyword must be at the start of the line (any leading
            // white space will have been stripped
            if( mbStrPos( buf2_p, pmcCfg[i].key_p ) == 0 )
            {
                if( pmcCfg[i].init == FALSE )
                {
                    len = strlen( pmcCfg[i].key_p );
                    mbMalloc( pmcCfg[i].str_p, strlen( buf2_p+len ) + 1 );
                    mbStrClean( buf2_p+len, pmcCfg[i].str_p, FALSE );
                    pmcCfg[i].init = TRUE;

                    if( pmcCfg[i].type == PMC_CFG_TYPE_INT )
                    {
                        pmcCfg[i].value = atol( pmcCfg[i].str_p );
                    }
                    if( pmcCfg[i].type == PMC_CFG_TYPE_BOOLEAN )
                    {
                        mbStrToUpper( pmcCfg[i].str_p );
                        pmcCfg[i].value = ( mbStrPos( pmcCfg[i].str_p, "TRUE" ) >= 0 ) ? TRUE : FALSE;
                    }
                    nbDlgDebug(( "Got '%s' value %ld for '%s'\n", pmcCfg[i].str_p, pmcCfg[i].value, pmcCfg[i].key_p ));
               }
                else
                {
                    if(    i != CFG_HOSTNAME
                        && i != CFG_LOG_DIR
                        && i != CFG_DATABASE_DESC
                        && i != CFG_DATABASE_MYSQL
                        && i != CFG_DATABASE_HOST
                        && i != CFG_DATABASE_PORT
                        && i != CFG_DATABASE_USERNAME
                        && i != CFG_DATABASE_PASSWORD
                        && i != CFG_DATABASE_LOCAL  )
                    {
                        mbDlgExclaim( "Duplicate config entry for '%s'\n", pmcCfg[i].key_p );
                    }
                }
            }
        }

        // The following items must be handled specially
        if( mbStrPos( buf2_p, PMC_INIT_PICKLIST_ICD ) >= 0 )
        {
            len = strlen( PMC_INIT_PICKLIST_ICD );
            pmcPickListCfgAdd( buf2_p+len, PMC_PICKLIST_CFG_ICD );
        }

        if( mbStrPos( buf2_p, PMC_INIT_PICKLIST_MSP_SERVICE ) >= 0 )
        {
            len = strlen( PMC_INIT_PICKLIST_MSP_SERVICE );
            pmcPickListCfgAdd( buf2_p+len, PMC_PICKLIST_CFG_MSP_SERVICE );
        }

        if( mbStrPos( buf2_p, PMC_INIT_PICKLIST_MSP_HOSPITAL ) >= 0 )
        {
            len = strlen( PMC_INIT_PICKLIST_MSP_HOSPITAL );
            pmcPickListCfgAdd( buf2_p+len, PMC_PICKLIST_CFG_MSP_HOSPITAL );
        }
    }

    // Check validity of configured items
    for( i = 0 ; i < CFG_INVALID ; i++ )
    {
        if( pmcCfg[i].init == FALSE && pmcCfg[i].required == TRUE )
        {
            mbDlgExclaim( "Required configuration file field '%s' not detected.", pmcCfg[i].key_p );
            goto exit;
        }
        if( pmcCfg[i].init == TRUE && pmcCfg[i].type == PMC_CFG_TYPE_DIR )
        {
            if( !DirectoryExists( pmcCfg[i].str_p ) )
            {
                mbDlgExclaim( "Could not access configured directory '%s'.", pmcCfg[i].str_p );
                goto exit;
            }
        }
        if( pmcCfg[i].init == TRUE && pmcCfg[i].type == PMC_CFG_TYPE_FILE )
        {
            if( !FileExists( pmcCfg[i].str_p ) )
            {
                mbDlgExclaim( "Could not access configured file '%s'.", pmcCfg[i].str_p );
                goto exit;
            }
        }
    }

    if( strlen( pmcCfg[CFG_AREA_CODE].str_p ) != 3 )
    {
        mbDlgExclaim( "Invalid local area code '%s'", pmcCfg[CFG_AREA_CODE].str_p );
    }

    if( pmcCfg[CFG_HOSTNAME].str_p == NIL )
    {
        mbDlgExclaim( "No host name defined!" );
        mbMalloc( pmcCfg[CFG_HOSTNAME].str_p, 32 );
        sprintf( pmcCfg[CFG_HOSTNAME].str_p, "hostname" );
    }


    if( mysql_q->size > 1 )
    {
        qHead_t     strQueue;
        qHead_p     str_q;

        str_q = qInitialize( &strQueue );
        qWalk( mysql_p, mysql_q, pmcMysqlItem_p )
        {
            mbStrListAddNew( str_q, mysql_p->desc, NIL, (Int32u_t)mysql_p, 0 );
        }
        mysql_p = (pmcMysqlItem_p)pmcFormPickList( "Practice Manager - Select Database", str_q );
        mbStrListFree( str_q );
    }
    else
    {
        mysql_p = qFirst( mysql_q, pmcMysqlItem_p );
    }

    if( mysql_p == NIL )
    {
        mbDlgError( "Error selecting MySQL database" );
        goto exit;
    }

    // Transfer selected database parameters into global config variables
    mbRemallocStr( pmcCfg[CFG_DATABASE_DESC].str_p,     mysql_p->desc );
    mbRemallocStr( pmcCfg[CFG_DATABASE_MYSQL].str_p,    mysql_p->mysql );
    mbRemallocStr( pmcCfg[CFG_DATABASE_HOST].str_p,     mysql_p->host );
    mbRemallocStr( pmcCfg[CFG_DATABASE_PASSWORD].str_p, mysql_p->password );
    mbRemallocStr( pmcCfg[CFG_DATABASE_USERNAME].str_p, mysql_p->username );
    pmcCfg[CFG_DATABASE_PORT].value = mysql_p->port;

    // Free the queue of MySQL databases
    while( !qEmpty( mysql_q ) )
    {
        mysql_p = (pmcMysqlItem_p)qRemoveFirst( mysql_q );
        mbFree( mysql_p );
    }

    // Default Colors
    for( Ints_t i = 0 ; i < PMC_COLOR_COUNT ; i++ )
    {
        pmcColor[i] = pmcDefaultColors[i];
    }

    // Read the MSP files
    if( !pmcInitFeesFileReadOld( pmcCfg[CFG_FEES_FILE_OLD].str_p ) ) goto exit;
    if( !pmcInitFeesFileRead( pmcCfg[CFG_FEES_FILE].str_p ) ) goto exit;

    pmcInitFeeFilesMerge( );

    if( !pmcInitIcdFileRead( pmcCfg[CFG_ICD_FILE].str_p ) ) goto exit;
    if( !pmcInitExpFileRead( pmcCfg[CFG_EXP_FILE].str_p ) ) goto exit;

    mbLog( "Fee cutover date: %d\n", pmcCfg[CFG_CUTOVER_DATE].value );

    {
        // Clean out the temporary directory
        qHead_t                 fileHead;
        qHead_p                 file_q;
        mbFileListStruct_p      file_p;

        file_q = qInitialize( &fileHead );

        mbFileListGet( pmcCfg[CFG_TEMP_DIR].str_p, "*.*", file_q, FALSE );

        qWalk( file_p, file_q, mbFileListStruct_p )
        {
             unlink( file_p->fullName_p );
        }
        mbFileListFree( file_q );
    }

    pmcNewPatForm = pmcCfg[CFG_NEW_PAT_FORM_FLAG].value;

    if( pmcCfg[CFG_ECHO_MIN_MB_FREE].value == 0 ) pmcCfg[CFG_ECHO_MIN_MB_FREE].value = 0;
    pmcEchoMinBytesFree  = (Int64u_t) pmcCfg[CFG_ECHO_MIN_MB_FREE].value;
    pmcEchoMinBytesFree *= (Int64u_t)1048576;

    // Initialize my SQL library
    if( ( rv = mbSqlInit( pmcCfg[CFG_DATABASE_HOST].str_p,
                          pmcCfg[CFG_DATABASE_PORT].value,
                          pmcCfg[CFG_DATABASE_MYSQL].str_p,
                          pmcCfg[CFG_DATABASE_USERNAME].str_p,
                          pmcCfg[CFG_DATABASE_PASSWORD].str_p,
                          pmcCfg[CFG_LOG_SQL].value ) ) != MB_SQL_ERR_OK )
    {
        if( rv == MB_SQL_ERR_CONNECT )
        {
            mbDlgExclaim( "Cound not connect to MySQL database" );
        }
        goto exit;
    }

    // Initialize our "pracman registry"
    pmcRegInit( pmcCfg[CFG_DATABASE_HOST].str_p, pmcCfg[CFG_DATABASE_USERNAME].str_p );

    returnCode = TRUE;

    pmcButtonRect_p = NIL;
    pmcButtonCanvas_p = NIL;
    pmcButtonString_p = NIL;

    // Set up default echo sort modes
    pmcEchoSortMode             = PMC_SORT_ID_DESCENDING;
    pmcEchoSortReadDateMode     = PMC_SORT_DATE_ASCENDING;
    pmcEchoSortStudyNameMode    = PMC_SORT_DESC_DESCENDING;
    pmcEchoSortPatNameMode      = PMC_SORT_NAME_DESCENDING;
    pmcEchoSortStudyDateMode    = PMC_SORT_STUDY_DATE_ASCENDING;
    pmcEchoSortIddMode          = PMC_SORT_ID_DESCENDING;
    pmcEchoSortCommentMode      = PMC_SORT_COMMENT_DESCENDING;

    pmcEchoFilterBackedUpChecked    = TRUE;
    pmcEchoFilterNotBackedUpChecked = TRUE;
    pmcEchoFilterOnlineChecked      = TRUE;
    pmcEchoFilterOfflineChecked     = TRUE;
    pmcEchoFilterReadChecked        = FALSE;
    pmcEchoFilterNotReadChecked     = TRUE;

    pmcEchoReadBackItemIndex = 1;

    // Get the config list stored in the database
    pmcTableStatusUpdate( PMC_SQL_TABLE_CONFIG );
    returnCode = pmcDBConfigInit( TRUE );

    // Get list of local telephone exchanges.  Must happen after pmcDBConfigInit
    pmcLocalExchangeInit( );

    pmcPatHistoryTypeInit( );

    // Get med list from database
 //   pmcTableStatusUpdate( PMC_SQL_TABLE_MED_LIST );
 //   returnCode = pmcDBMedListInit( TRUE );

    pmcDocumentListInit( );


    for( Ints_t i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        mbMalloc( pmcDefaultClaimCode_p[i], 32 );
        *pmcDefaultClaimCode_p[i] = 0;
    }

exit:

    if( fp ) fclose( fp );
    mbFree( fileName_p );
    mbFree( buf_p );
    mbFree( buf2_p );

    return returnCode;
}

#if 0
// OBSOLETE

//---------------------------------------------------------------------------
// Function:  pmcDBConfigFree
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
Int32s_t pmcDBConfigFree( void )
{
    pmcDBConfig_p           item_p;

    if( pmcDBConfig_q != NIL )
    {
        while( !qEmpty( pmcDBConfig_q ) )
        {
            item_p = (pmcDBConfig_p)qRemoveFirst( pmcDBConfig_q );
            mbFree( item_p->value_p );
            mbFree( item_p );
        }
    }

    return TRUE;
}
#endif

//---------------------------------------------------------------------------
// Function:  pmcDBMedListFreeEntry
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
Int32s_t pmcDBMedListFreeEnfry( pmcMedEntry_p item_p )
{
    if( item_p )
    {
        mbFree( item_p->generic_p );
        mbFree( item_p->brand_p );
        mbFree( item_p->class_p );
        mbFree( item_p );
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcDBMedListFree
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
Int32s_t pmcDBMedListFree( qHead_p queue_p )
{
    pmcMedEntry_p           item_p;

    if( queue_p )
    {
        while( !qEmpty( queue_p ) )
        {
            item_p = (pmcMedEntry_p)qRemoveFirst( queue_p );
            pmcDBMedListFreeEnfry( item_p );
        }
    }
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  pmcDBMedListInit
//---------------------------------------------------------------------------
// Description:
//
// Read med list from the database
//---------------------------------------------------------------------------
Int32s_t pmcDBMedListInit( Boolean_t forceFlag )
{
    MbSQL                   sql;
    Char_p                  buf_p;
    Int32s_t                returnCode = FALSE;
    pmcMedEntry_p           item_p, item2_p;
    Boolean_t               match;
    static Int64u_t         modifyTime = 0;
    qHead_t                 tempQueue;
    qHead_p                 temp_q;
    qHead_t                 temp2Queue;
    qHead_p                 temp2_q;

    temp_q = qInitialize( &tempQueue );
    temp2_q = qInitialize( &temp2Queue );

    if( forceFlag == FALSE )
    {
        if( modifyTime >= pmcPollTableModifyTime[PMC_TABLE_INDEX_CONFIG]  ) return TRUE;
    }
    modifyTime = pmcPollTableModifyTime[PMC_TABLE_INDEX_CONFIG];

    mbMalloc( buf_p, 1024 );

    if( buf_p == NIL ) goto exit;

    pmcDBMedListFree( pmcDBMed_q );

    if( pmcDBMed_q == NIL )
    {
        pmcDBMed_q = qInitialize( &pmcDBMedQueue );
    }

    sprintf( buf_p, "select %s,%s,%s from %s where %s>0"
        ,PMC_SQL_FIELD_ID
        ,PMC_SQL_MED_LIST_FIELD_GENERIC
        ,PMC_SQL_MED_LIST_FIELD_BRAND

        ,PMC_SQL_TABLE_MED_LIST
        ,PMC_SQL_FIELD_ID );

    sql.Query( buf_p );

    while( sql.RowGet( ) )
    {
        mbCalloc( item_p, sizeof( pmcMedEntry_t ));

        mbMallocStr( item_p->generic_p, sql.String( 1 ) );
        mbMallocStr( item_p->brand_p, sql.String( 2 ) );

        qInsertLast( temp_q, item_p );
    }

    // OK, got all items.... now create new list from generic and brand names

    mbLog( "original size: %lu\n", temp_q->size );
    qWalk( item_p, temp_q, pmcMedEntry_p )
    {
        match = FALSE;
        qWalk( item2_p, temp2_q, pmcMedEntry_p )
        {
            if( strcmp( item_p->generic_p, item2_p->brand_p ) == 0 )
            {
                match = TRUE;
                break;
            }
        }

        if( match == TRUE )
        {
            mbLog( "already have GENERIC '%s'\n", item_p->generic_p );
        }
        else
        {
            mbCalloc( item2_p, sizeof( pmcMedEntry_t ) );
            mbMallocStr( item2_p->brand_p, item_p->generic_p );
            mbMallocStr( item2_p->generic_p, "GENERIC" );
            qInsertLast( temp2_q, item2_p );
        }
    }
    mbLog( "size after generics : %lu\n", pmcDBMed_q->size );

    qWalk( item_p, temp_q, pmcMedEntry_p )
    {
        match = FALSE;
        qWalk( item2_p, temp2_q, pmcMedEntry_p )
        {
            if( strcmp( item_p->brand_p, item2_p->brand_p ) == 0 )
            {
                match = TRUE;
                break;
            }
        }

        if( match == TRUE )
        {
            mbLog( "already have BRAND '%s'\n", item_p->generic_p );
        }
        else
        {
            mbCalloc( item2_p, sizeof( pmcMedEntry_t ) );
            mbMallocStr( item2_p->brand_p, item_p->brand_p );
            mbMallocStr( item2_p->generic_p, item_p->generic_p );
            qInsertLast( temp2_q, item2_p );
        }
    }

    /* Sort alphabetically */
    while( !qEmpty( temp2_q ) )
    {
        item_p = (pmcMedEntry_p)qRemoveFirst( temp2_q );

        match = FALSE;
        qWalk( item2_p, pmcDBMed_q, pmcMedEntry_p )
        {
            if( strcmp( item_p->brand_p, item2_p->brand_p ) < 0 )
            {
                qInsertBefore( pmcDBMed_q, item2_p, item_p );
                match = TRUE;
                break;
            }
        }

        if( match == FALSE )
        {
            qInsertLast( pmcDBMed_q, item_p );
        }
    }
    mbLog( "Final size: %lu\n", pmcDBMed_q->size );

    returnCode = TRUE;

exit:

    pmcDBMedListFree( temp_q );
    pmcDBMedListFree( temp2_q );
    return returnCode;
}


//---------------------------------------------------------------------------
// Function:  pmcDBConfigInit
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------
Int32s_t pmcDBConfigInit( Boolean_t forceFlag )
{
    MbSQL                   sql;
    Char_p                  buf_p;
    Int32s_t                returnCode = FALSE;
    mbStrList_p             item_p, item2_p;
    Boolean_t               added;
    static Int64u_t         modifyTime = 0;

    if( forceFlag == FALSE )
    {
        if( modifyTime >= pmcPollTableModifyTime[PMC_TABLE_INDEX_CONFIG]  ) return TRUE;
    }
    modifyTime = pmcPollTableModifyTime[PMC_TABLE_INDEX_CONFIG];

    mbMalloc( buf_p, 1024 );

    if( buf_p == NIL ) goto exit;


 //   pmcDBConfigFree( );

    if( pmcDBConfig_q == NIL )
    {
        pmcDBConfig_q = qInitialize( &pmcDBConfigQueue );
    }

    mbStrListFree( pmcDBConfig_q );

    sprintf( buf_p, "select %s,%s,%s,%s from %s where %s>0"
        ,PMC_SQL_CONFIG_FIELD_VAL_STR_1
        ,PMC_SQL_CONFIG_FIELD_VAL_STR_2
        ,PMC_SQL_CONFIG_FIELD_VAL_INT_1
        ,PMC_SQL_CONFIG_FIELD_VAL_INT_2

        ,PMC_SQL_TABLE_CONFIG
        ,PMC_SQL_FIELD_ID );

    sql.Query( buf_p );

    while( sql.RowGet( ) )
    {
 //       mbCalloc( item_p, sizeof( pmcDBConfig_t ));

        item_p = mbStrListItemAlloc( sql.String( 0 ), sql.String( 1 ), sql.Int32u( 2 ), sql.Int32u( 3 ) );

 //       item_p->type    = sql.Int32u( 2 );
//        item_p->order   = sql.Int32u( 3 );

 //       mbMallocStr( item_p->value_p, sql.String( 0 ) );

        added = FALSE;
        qWalk( item2_p, pmcDBConfig_q, mbStrList_p )
        {
            // handle contains the type
            if( item_p->handle < item2_p->handle )
            {
                qInsertBefore( pmcDBConfig_q, item2_p, item_p );
                added = TRUE;
                break;
            }
            if( item_p->handle == item2_p->handle )
            {
                // Handle2 contains the pick order
                if( item_p->handle2 < item2_p->handle2 )
                {
                    qInsertBefore( pmcDBConfig_q, item2_p, item_p );
                    added = TRUE;
                    break;
                }
            }
        }
        if( !added )
        {
            qInsertLast( pmcDBConfig_q, item_p );
        }
    }

    qWalk( item_p, pmcDBConfig_q, mbStrList_p )
    {
        mbLog( "Type: %lu order: %lu value '%s'\n", item_p->handle, item_p->handle2, item_p->str_p );
    }
    returnCode = TRUE;

exit:
    mbFree( buf_p );
    return returnCode;
}

//---------------------------------------------------------------------------
// Function:    pmcInitFeeFilesMerge
//---------------------------------------------------------------------------
// Description: This function adds the old fee values to the fee list.
//
//---------------------------------------------------------------------------

Int32s_t    pmcInitFeeFilesMerge( )
{
    pmcFeeStruct_p  fee_p, fee2_p;
    Boolean_t       match;

   // MAB:20020325: This section of code adds the old fees to the list
    // of fees.  This will allow us to automatically change the fee
    // according to the date of service

    qWalk( fee_p, pmcFee_q, pmcFeeStruct_p )
    {
        match = FALSE;
        qWalk( fee2_p, pmcOldFee_q, pmcFeeStruct_p )
        {
            if( strcmp( fee_p->code, fee2_p->code ) == 0 )
            {
                match = TRUE;
                break;
            }
        }
        if( match )
        {
            fee_p->oldFeeHigh = fee2_p->feeHigh;
            fee_p->oldFeeLow  = fee2_p->feeLow;
            if(    ( fee_p->feeHigh != fee_p->oldFeeHigh )
                || ( fee_p->feeLow  != fee_p->oldFeeLow  ) )
            {
                //mbLog( "Fee code %s differs: High: %d %d Low: %d %d\n", fee_p->code,
                //    fee_p->feeHigh, fee_p->oldFeeHigh, fee_p->feeLow, fee_p->oldFeeLow );
            }
            qRemoveEntry( pmcOldFee_q, &fee2_p->linkage );
            mbFree( fee2_p );
        }
        else
        {
            // mbLog( "New fee %s\n", fee_p->code );
        }
    }

    // mbLog( "The following %d old fees do not appear in the new list\n", pmcOldFee_q->size );
    while( !qEmpty( pmcOldFee_q ) )
    {
        fee_p = (pmcFeeStruct_p)qRemoveFirst( pmcOldFee_q );

        // mbLog( "Code '%s' fee '%d'\n", fee_p->code, fee_p->feeHigh );

        fee_p->oldFeeHigh = fee_p->feeHigh;
        fee_p->oldFeeLow  = fee_p->feeLow;
        fee_p->feeHigh = 0;
        fee_p->feeLow  = 0;

        // Put the obsolete fees into the new fee queue
        qInsertLast( pmcFee_q, fee_p );
    }
    // MAB:20020325: End of section merging old and new fees.
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------

#if 0
void pmcMedListEntryFree( pmcMedEntry_p   med_p )
{
    if( med_p )
    {
        mbFree( med_p->brand_p );
        mbFree( med_p->gen_p );
        mbFree( med_p->class_p );
        mbFree( med_p );
    }
    return;
}

#endif

#if 0
void pmcMedListEntryAdd
(
    qHead_p queue_p,
    Char_p  brand_p,
    Char_p  gen_p,
    Char_p  class_p
)
{
    pmcMedEntry_p      med_p, med2_p;
    Char_t             temp[1024];
    Boolean_t          added;

    mbMalloc( med_p, sizeof( pmcMedEntry_t ) );
    mbMallocStr( med_p->brand_p,    mbStrClean( brand_p, temp, TRUE ) );
    mbMallocStr( med_p->gen_p,      mbStrClean( gen_p, temp, TRUE ) );
    mbMallocStr( med_p->class_p,    mbStrClean( class_p, temp, TRUE ) );

    added = FALSE;
    qWalk( med2_p, queue_p, pmcMedEntry_p )
    {
        if( strcmp( med_p->brand_p, med2_p->brand_p ) == 0 )
        {
            /* Found a duplicate entry */
            added = TRUE;
            pmcMedListEntryFree( med_p );
            break;
        }

        if( strcmp( med_p->brand_p, med2_p->brand_p ) < 0 )
        {
            qInsertBefore( queue_p, med2_p, med_p );
            added = TRUE;
            break;
        }
    }

    if( added == FALSE )
    {
        qInsertLast( queue_p, med_p );
    }
    return;
}
#endif

Int32s_t pmcInitFormularyFileRead( Char_p fileName_p )
{
    mbDlgInfo( "Compiled out" );
    return TRUE;
#if 0
    FILE           *in_p;
    Char_p          buf1_p;
    Char_p          buf2_p;
    Char_p          class_p;
    Char_p          generic_p;
    Char_p          brand_p;
    Char_p          temp_p;
    Boolean_t       newGenericFlag = FALSE;
    Int32s_t        i;
    Int32s_t        pos;
    qHead_p         queue_p;
    qHead_t         queueHead;
    pmcMedEntry_p   med_p;

    Char_p          maker[] =
    {
         "NOVO-"
        ,"APO-"
        ,"PMS-"
        ,"RATIO-"
        ,"GEN-"
        ,"DOM-"
        ,"SANDOZ "
        ,"NU-"
        ,"CO "
        ,"TARO-"
        ,"RAN-"
        ,""
    };

    queue_p = qInitialize( &queueHead );

    mbMalloc( buf1_p,       256 );
    mbMalloc( buf2_p,       256 );
    mbMalloc( class_p,      128 );
    mbMalloc( generic_p,    128 );
    mbMalloc( brand_p,      128 );

    if( buf1_p == NIL || buf2_p == NIL ) goto exit;

    in_p = fopen( fileName_p, "r" );

    if( in_p == NIL )
    {
        mbDlgExclaim( "Could not open fee description file '%s'.", fileName_p );
        goto exit;
    }

    while( fgets( buf2_p, 1024, in_p ) != NULL )
    {
        // Clean line
        mbStrClean( buf2_p, buf1_p, TRUE );
        if( strlen( buf1_p ) < 4 ) continue;

        if( *buf1_p == '2' )
        {
            mbStrClean( ( buf1_p + 10 ), class_p, TRUE );
//            mbLog( "Got Class: '%s'\n", class_p );
            continue;
       }
        else if( *buf1_p == '4' )
        {
            if( *( buf1_p + 1 ) == '1' )
            {
                mbStrClean( ( buf1_p + 28 ), generic_p, TRUE );
//                mbLog( "Got Generic Drug: '%s'\n", generic_p );

                if( newGenericFlag == TRUE )
                {
//                    mbLog( "Got new generic '%s' without a brand!!!\n\]n", generic_p );
                    pmcMedListEntryAdd( pmcMed_q, generic_p, generic_p, class_p );

                }
                newGenericFlag = TRUE;
                continue;
            }

        }
        else if( *buf1_p == '6' )
        {
            newGenericFlag = FALSE;
            *( buf1_p + 66 ) = 0;
            mbStrClean( ( buf1_p + 40 ), brand_p, TRUE );

            temp_p = brand_p;
            for( i = 0 ; ; i++ )
            {
                if( strlen( maker[i] ) == 0 ) break;

                if( mbStrPos( brand_p, maker[i] ) == 0 )
                {
//                    mbLog( "Found maker in brand '%s'\n", brand_p );
                    temp_p = brand_p + strlen( maker[i] );
//                    *(brand_p + strlen( maker[i] )) = 0;
                    break;
                }
            }
            if( ( pos = mbStrPos( temp_p, "(EDS" ) ) >= 0 )
            {
               *(temp_p + pos) = 0;
            }

//            mbLog( "'%s' Gen: '%s' Class: '%s'\n",
//                brand_p, generic_p, class_p );

            /* Allocate a structure for the new entry */
            pmcMedListEntryAdd( pmcMed_q, temp_p, generic_p, class_p );

            continue;
        }

        // mbLog( "Ignore line: '%s'\n", buf1_p );
    }

    /* Temporary code to add some initial medications to the database */
    {
        MbSQL       sql;
        Int32u_t    id;
        Char_p      cmd_p;

        mbMalloc( cmd_p, 4096);

        qWalk( med_p, pmcMed_q, pmcMedEntry_p )
        {
            mbLog( "'%s' '%s' '%s'\n", med_p->brand_p, med_p->gen_p, med_p->class_p );

            id = pmcSqlRecordCreate( PMC_SQL_TABLE_MED_LIST, NIL );

            sprintf( cmd_p, "update %s set %s=\"%s\",%s=\"%s\",%s=\"%s\" where %s=%lu"
                ,PMC_SQL_TABLE_MED_LIST

                ,PMC_SQL_MED_LIST_FIELD_NAME    , med_p->gen_p
                ,PMC_SQL_MED_LIST_FIELD_BRAND   , med_p->brand_p
                ,PMC_SQL_MED_LIST_FIELD_CLASS   , med_p->class_p

                ,PMC_SQL_FIELD_ID               , id );

            sql.Update( cmd_p );
        }
        mbFree( cmd_p );
    }

exit:
    mbFree( buf1_p );
    mbFree( buf2_p );
    mbFree( class_p );
    mbFree( generic_p );
    mbFree( brand_p );

    return TRUE;
#endif
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
// SASKATCHEWAN SERVICE CODES AND FEES FILE FORMAT
//
// FIELD NAME               FROM   TO    LEN   EXPLANATION/VALUE
// ----------------------  ----  ----  -----  -----------------
// Service Code               1     4     4    Three numerics followed by one alpha
//
// Filler                     5     5     1    Space
//
// High Fee                   6    13     8    ZZZZ9.99
//
// Filler                    14    14     1    Space
//
// Low Fee                   15    22     8    ZZZZ9.99 if applicable
//
// Filler                    23    23     1    Space
//
// Service Classification    24    25     2    Spaces - Not classified as a procedure
//                                              ' D'  - Diagnostic Procedure
//                                              ' V'  - Visit/Consultation
//                                              ' 0'  - 0 day surgery
//                                              '10'  - 10 day surgery
//                                              '42'  - 42 day surgery
//
// Filler                    26    26     1    Space
//
// Add on Indicator          27    27     1    Space - Not an add on procedure
//                                             'A'  - This is an add on procedure and should
//                                                    always be submitted at 100%
//
// Filler                    28    28     1    Space
//
// Multiple Unit Indicator   29    29     1    Space - Multiple units are NOT allowed
//                                              'U'  - Multiple units are allowed
//
// Filler                    30    30     1    Space
//
// Fee Determinant           31    31     1    'G' - Only one fee billed by a GP.
//                                             'S' - Only one fee billed by a specialist.
//                                             'E' - The one fee can be billed by a
//                                                   specialist or GP.
//                                             'D' - The specialist bills the high fee and
//                                                   the GP or someone with special training
//                                                   bills the low fee.
//                                             'H' - The specialist bills the high or low fee.
//                                                   The high fee is referred and the low fee
//                                                   is unreferred.
//                                             'W' - Billed by any physician with ultrasound
//                                                   entitlement.  The high fee represents
//                                                   the total fee and the low fee is the
//                                                   interpretation fee.
//                                             'X' - Billed only by radiologists.  The high
//                                                   fee represents the total fee and the low
//                                                   fee is the interpretation fee.
//                                             'C' - Chiropractor and optometrist codes.
//                                                   The high fee for 1U, 5U and 6U is payable
//                                                   for those on special programs and the low
//                                                   fee is for regular Saskatchewan
//                                                   beneficiaries.
//                                                   The high fee for chiropractor x-ray codes
//                                                   of 32U to 72U represents the total fee
//                                                   and the low fee is the interpretation fee.
//
// --- Don't bother reading remaining fields ---
//
// Filler                    32    50    19    Spaces
//
// Anaesthesia-Ind           51    51     1    'L' - Low Complexity.
//                                             'M' - Medium Complexity.
//                                             'H' - High Complexity.
//
//---------------------------------------------------------------------------

Int32s_t            pmcInitFeesFileRead( Char_p fileName_p )
{
    FILE           *in_p;
    Char_p          buf1_p;
    Char_p          buf2_p;
    pmcFeeStruct_p  fee_p, fee2_p;
    Int32u_t        i, j;
    Ints_t          len;
    Int32u_t        duplicateCount = 0;
    Int32u_t        duplicateFlag;
    Int32u_t        add;
    Int32u_t        index = 0;
    Boolean_t       gotLowerCase = FALSE;
    Boolean_t       gotUpperCase = FALSE;
    Int32s_t        returnCode = FALSE;
    qHead_p         fee_q;

    fee_q = pmcFee_q;

    mbMalloc( buf1_p, 1024 );
    mbMalloc( buf2_p, 1024 );

    // Attempt to open fees file
    in_p = fopen( fileName_p, "r" );

    if( in_p == NIL )
    {
        mbDlgExclaim( "Could not open fee description file '%s'.", fileName_p );
        goto exit;
    }

    while( fgets( buf2_p, 1024, in_p ) != NULL )
    {
        // Clean line
        mbStrClean( buf2_p, buf1_p, TRUE );
        if( strlen( buf1_p ) < 4 ) continue;

        //mbLog( "Read line '%s' len (%d)\n", buf1_p, strlen( buf1_p ) );

        mbMalloc( fee_p, sizeof( pmcFeeStruct_t ) );
        memset( fee_p, 0 , sizeof( pmcFeeStruct_t ) );

        fee_p->addOn = FALSE;
        fee_p->multiple = FALSE;
        fee_p->classification = PMC_FEE_CLASS_NONE;

        // First Get the fee code
        len = strlen( buf1_p );
        if( len >= 4 )
        {
            memcpy( fee_p->code, buf1_p, 4 );
            add = TRUE;
            if( mbStrPos( "0", fee_p->code ) >= 0 )
            {
                mbDlgDebug(( "Code has an OH in '%s'", fee_p->code ));
                pmcFeeContainsOhs = TRUE;
            }

            // Check for case insensitivity
            for( i = 0 ; i < 4 ; i++ )
            {
                if( fee_p->code[i] >= 'A' && fee_p->code[i] <= 'Z' ) gotUpperCase = TRUE;
                if( fee_p->code[i] >= 'a' && fee_p->code[i] <= 'z' ) gotLowerCase = TRUE;
            }
        }
        else
        {
            add = FALSE;
        }

        // Next get the high fee
        if( len  >= 13 )
        {
            for( i = 5 , j = 0 ; i < 13 ; i++ )
            {
                if( *(buf1_p + i ) >= '0' && *(buf1_p + i) <= '9' )
                {
                    *( buf2_p + j ) = *( buf1_p + i );
                    j++;
                }
            }
            *( buf2_p + j ) = 0;
            fee_p->feeHigh = atol( buf2_p );
        }

        // Next get low fee
        if( len  >= 22 )
        {
            for( i = 14 , j = 0 ; i < 22 ; i++ )
            {
                if( *(buf1_p + i ) >= '0' && *(buf1_p + i) <= '9' )
                {
                    *( buf2_p + j ) = *( buf1_p + i );
                    j++;
                }
             }
             *( buf2_p + j ) = 0;
             fee_p->feeLow = atol( buf2_p );
        }

        // Service Classification
        if( len  >= 25 )
        {
            for( i = 23 , j = 0 ; i < 25 ; i++ )
            {
                *( buf2_p + j ) = *( buf1_p + i );
                j++;
            }
            *( buf2_p + j ) = 0;

            if( strcmp( buf2_p, " D" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DIAGNOSTIC;
            }
            else if( strcmp( buf2_p, " 0" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DAY_SURGERY_0;
            }
            else if( strcmp( buf2_p, "10" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DAY_SURGERY_10;
            }
            else if( strcmp( buf2_p, "42" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DAY_SURGERY_42;
            }
        }

        // Add on indicator
        if( len >= 27 )
        {
            if( *( buf1_p + 26 ) == 'A' ) fee_p->addOn = TRUE;
        }

         // Multiple unit indicator
        if( len >= 29 )
        {
            if( *( buf1_p + 28 ) == 'U' ) fee_p->multiple = TRUE;
        }

        fee_p->determinant = PMC_FEE_DET_INVALID;
        // Read the Fee determinant
        if( len  >= 31 )
        {
            if( *( buf1_p + 30 ) == 'G' ) fee_p->determinant = PMC_FEE_DET_GP_ONE_FEE;
            if( *( buf1_p + 30 ) == 'S' ) fee_p->determinant = PMC_FEE_DET_SP_ONE_FEE;
            if( *( buf1_p + 30 ) == 'E' ) fee_p->determinant = PMC_FEE_DET_SP_OR_GP_ONE_FEE;
            if( *( buf1_p + 30 ) == 'D' ) fee_p->determinant = PMC_FEE_DET_SP_OR_GP;
            if( *( buf1_p + 30 ) == 'H' ) fee_p->determinant = PMC_FEE_DET_SP;
            if( *( buf1_p + 30 ) == 'W' ) fee_p->determinant = PMC_FEE_DET_ULTRASOUND;
            if( *( buf1_p + 30 ) == 'X' ) fee_p->determinant = PMC_FEE_DET_RADIOLOGIST;
            if( *( buf1_p + 30 ) == 'C' ) fee_p->determinant = PMC_FEE_DET_CHIROPRACTOR;
        }

#if 0
        // Sanity Check - it looks like every fee code must have a determinant.
        // Check that that is indeed the case
        if( fee_p->determinant == PMC_FEE_DET_INVALID )
        {
            mbDlgDebug(( "Invalid fee record read from fee file: '%s', Det: %c\n",
                buf1_p, *( buf1_p + 30 ) ));
        }
#endif
        if( add == TRUE )
        {
            // Look for duplicate entries
            duplicateFlag = FALSE;
            qWalk( fee2_p, fee_q, pmcFeeStruct_p )
            {
                if( mbStrPosLen( fee2_p->code, fee_p->code ) == 0 )
                {
                    duplicateCount++;
                    duplicateFlag = TRUE;
                    mbDlgDebug(( "Found duplicate fee code" ));
                    break;
                }
            }
        }
        if( duplicateFlag == TRUE || add == FALSE )
        {
            mbFree( fee_p );
        }
        else
        {
            qInsertLast( fee_q, fee_p );
            fee_p->index = ++index;
        }
    }

    returnCode = TRUE;

exit:

    // Set Fee case sensitivity
    pmcFeeCaseSensitive = FALSE;
    if( gotUpperCase && gotLowerCase )
    {
        pmcFeeCaseSensitive = TRUE;
    }

    mbLog( "Read %ld fee codes from fee file '%s' (%ld duplicates)\n",
        fee_q->size, fileName_p, duplicateCount );

    if( in_p ) fclose( in_p );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
//
// SASKATCHEWAN SERVICE CODES AND FEES FILE FORMAT
//
// FIELD NAME               FROM   TO    LEN   EXPLANATION/VALUE
// ----------------------  ----  ----  -----  -----------------
// Service Code               1     4     4    Three numerics followed by one alpha
//
// Filler                     5     5     1    Space
//
// High Fee                   6    13     8    ZZZZ9.99
//
// Filler                    14    14     1    Space
//
// Low Fee                   15    22     8    ZZZZ9.99 if applicable
//
// Filler                    23    23     1    Space
//
// Service Classification    24    25     2    Spaces - Not classified as a procedure
//                                              ' D'  - Diagnostic Procedure
//                                              ' V'  - Visit/Consultation
//                                              ' 0'  - 0 day surgery
//                                              '10'  - 10 day surgery
//                                              '42'  - 42 day surgery
//
// Filler                    26    26     1    Space
//
// Add on Indicator          27    27     1    Space - Not an add on procedure
//                                             'A'  - This is an add on procedure and should
//                                                    always be submitted at 100%
//
// Filler                    28    28     1    Space
//
// Multiple Unit Indicator   29    29     1    Space - Multiple units are NOT allowed
//                                              'U'  - Multiple units are allowed
//
// Filler                    30    30     1    Space
//
// Fee Determinant           31    31     1    'G' - Only one fee billed by a GP.
//                                             'S' - Only one fee billed by a specialist.
//                                             'E' - The one fee can be billed by a
//                                                   specialist or GP.
//                                             'D' - The specialist bills the high fee and
//                                                   the GP or someone with special training
//                                                   bills the low fee.
//                                             'H' - The specialist bills the high or low fee.
//                                                   The high fee is referred and the low fee
//                                                   is unreferred.
//                                             'W' - Billed by any physician with ultrasound
//                                                   entitlement.  The high fee represents
//                                                   the total fee and the low fee is the
//                                                   interpretation fee.
//                                             'X' - Billed only by radiologists.  The high
//                                                   fee represents the total fee and the low
//                                                   fee is the interpretation fee.
//                                             'C' - Chiropractor and optometrist codes.
//                                                   The high fee for 1U, 5U and 6U is payable
//                                                   for those on special programs and the low
//                                                   fee is for regular Saskatchewan
//                                                   beneficiaries.
//                                                   The high fee for chiropractor x-ray codes
//                                                   of 32U to 72U represents the total fee
//                                                   and the low fee is the interpretation fee.
//
// --- Don't bother reading remaining fields ---
//
// Filler                    32    50    19    Spaces
//
// Anaesthesia-Ind           51    51     1    'L' - Low Complexity.
//                                             'M' - Medium Complexity.
//                                             'H' - High Complexity.
//
//---------------------------------------------------------------------------

Int32s_t            pmcInitFeesFileReadOld( Char_p fileName_p )
{
    FILE           *in_p;
    Char_p          buf1_p;
    Char_p          buf2_p;
    pmcFeeStruct_p  fee_p, fee2_p;
    Int32u_t        i, j;
    Ints_t          len;
    Int32u_t        duplicateCount = 0;
    Int32u_t        duplicateFlag;
    Int32u_t        add;
    Int32u_t        index = 0;
    Boolean_t       gotLowerCase = FALSE;
    Boolean_t       gotUpperCase = FALSE;
    Int32s_t        returnCode = FALSE;
    qHead_p         fee_q;

    fee_q = pmcOldFee_q;

    mbMalloc( buf1_p, 1024 );
    mbMalloc( buf2_p, 1024 );

    // Attempt to open fees file
    in_p = fopen( fileName_p, "r" );

    if( in_p == NIL )
    {
        mbDlgExclaim( "Could not open fee description file '%s'.", fileName_p );
        goto exit;
    }

    while( fgets( buf2_p, 1024, in_p ) != NULL )
    {
        // Clean line
        mbStrClean( buf2_p, buf1_p, TRUE );
        if( strlen( buf1_p ) < 4 ) continue;

        //printf( "Read line '%s' len (%d)\n", buf1_p, strlen( buf1_p ) );

        mbMalloc( fee_p, sizeof( pmcFeeStruct_t ) );
        memset( fee_p, 0 , sizeof( pmcFeeStruct_t ) );

        fee_p->addOn = FALSE;
        fee_p->multiple = FALSE;
        fee_p->classification = PMC_FEE_CLASS_NONE;

        // First Get the fee code
        len = strlen( buf1_p );
        if( len >= 4 )
        {
            memcpy( fee_p->code, buf1_p, 4 );
            add = TRUE;
            if( mbStrPos( "0", fee_p->code ) >= 0 )
            {
                mbDlgDebug(( "Code has an OH in '%s'", fee_p->code ));
                pmcFeeContainsOhs = TRUE;
            }

            // Check for case insensitivity
            for( i = 0 ; i < 4 ; i++ )
            {
                if( fee_p->code[i] >= 'A' && fee_p->code[i] <= 'Z' ) gotUpperCase = TRUE;
                if( fee_p->code[i] >= 'a' && fee_p->code[i] <= 'z' ) gotLowerCase = TRUE;
            }
        }
        else
        {
            add = FALSE;
        }

        // Next get the high fee
        if( len  >= 13 )
        {
            for( i = 5 , j = 0 ; i < 13 ; i++ )
            {
                if( *(buf1_p + i ) >= '0' && *(buf1_p + i) <= '9' )
                {
                    *( buf2_p + j ) = *( buf1_p + i );
                    j++;
                }
            }
            *( buf2_p + j ) = 0;
            fee_p->feeHigh = atol( buf2_p );
        }

        // Next get low fee
        if( len  >= 22 )
        {
            for( i = 14 , j = 0 ; i < 22 ; i++ )
            {
                if( *(buf1_p + i ) >= '0' && *(buf1_p + i) <= '9' )
                {
                    *( buf2_p + j ) = *( buf1_p + i );
                    j++;
                }
             }
             *( buf2_p + j ) = 0;
             fee_p->feeLow = atol( buf2_p );
        }

        // Service Classification
        if( len  >= 25 )
        {
            for( i = 23 , j = 0 ; i < 25 ; i++ )
            {
                *( buf2_p + j ) = *( buf1_p + i );
                j++;
            }
            *( buf2_p + j ) = 0;

            if( strcmp( buf2_p, " D" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DIAGNOSTIC;
            }
            else if( strcmp( buf2_p, " 0" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DAY_SURGERY_0;
            }
            else if( strcmp( buf2_p, "10" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DAY_SURGERY_10;
            }
            else if( strcmp( buf2_p, "42" ) == 0 )
            {
                fee_p->classification = PMC_FEE_CLASS_DAY_SURGERY_42;
            }
        }

        // Add on indicator
        if( len >= 27 )
        {
            if( *( buf1_p + 26 ) == 'A' ) fee_p->addOn = TRUE;
        }

         // Multiple unit indicator
        if( len >= 29 )
        {
            if( *( buf1_p + 28 ) == 'U' ) fee_p->multiple = TRUE;
        }

        fee_p->determinant = PMC_FEE_DET_INVALID;
        // Read the Fee determinant
        if( len  >= 31 )
        {
            if( *( buf1_p + 30 ) == 'G' ) fee_p->determinant = PMC_FEE_DET_GP_ONE_FEE;
            if( *( buf1_p + 30 ) == 'S' ) fee_p->determinant = PMC_FEE_DET_SP_ONE_FEE;
            if( *( buf1_p + 30 ) == 'E' ) fee_p->determinant = PMC_FEE_DET_SP_OR_GP_ONE_FEE;
            if( *( buf1_p + 30 ) == 'D' ) fee_p->determinant = PMC_FEE_DET_SP_OR_GP;
            if( *( buf1_p + 30 ) == 'H' ) fee_p->determinant = PMC_FEE_DET_SP;
            if( *( buf1_p + 30 ) == 'W' ) fee_p->determinant = PMC_FEE_DET_ULTRASOUND;
            if( *( buf1_p + 30 ) == 'X' ) fee_p->determinant = PMC_FEE_DET_RADIOLOGIST;
            if( *( buf1_p + 30 ) == 'C' ) fee_p->determinant = PMC_FEE_DET_CHIROPRACTOR;
        }

#if 0
        // Sanity Check - it looks like every fee code must have a determinant.
        // Check that that is indeed the case
        if( fee_p->determinant == PMC_FEE_DET_INVALID )
        {
            mbDlgDebug(( "Invalid fee record read from fee file\n" ));
        }
#endif
        if( add == TRUE )
        {
            // Look for duplicate entries
            duplicateFlag = FALSE;
            qWalk( fee2_p, fee_q, pmcFeeStruct_p )
            {
                if( mbStrPosLen( fee2_p->code, fee_p->code ) == 0 )
                {
                    duplicateCount++;
                    duplicateFlag = TRUE;
                    mbDlgDebug(( "Found duplicate fee code" ));
                    break;
                }
            }
        }
        if( duplicateFlag == TRUE || add == FALSE )
        {
            mbFree( fee_p );
        }
        else
        {
            qInsertLast( fee_q, fee_p );
            fee_p->index = ++index;
        }
    }

    returnCode = TRUE;

exit:

    // Set Fee case sensitivity
    pmcFeeCaseSensitive = FALSE;
    if( gotUpperCase && gotLowerCase )
    {
        pmcFeeCaseSensitive = TRUE;
    }

    mbLog( "Read %ld fee codes from fee file '%s' (%ld duplicates)\n",
        fee_q->size, fileName_p, duplicateCount );

    if( in_p ) fclose( in_p );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}


//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//---------------------------------------------------------------------------

Int32s_t        pmcShutdown
(
    void
)
{
    Int32u_t                i;
    pmcLocalExchange_p      ex_p;
    pmcFeeStruct_p          fee_p;
    pmcIcdStruct_p          icd_p;
    pmcExpStruct_p          exp_p;
    pmcPickListCfg_p        pickList_p;

    mbSqlTerminate( );

    // Delete list of local phone exchanges
    for( ; ; )
    {
        if( qEmpty( gLocalExchange_q ) ) break;
        ex_p = (pmcLocalExchange_p)qRemoveFirst( gLocalExchange_q );
        mbFree( ex_p );
    }

    // Delete list of pat history types
    mbStrListFree( gPatHistoryType_q );

    // Delete list of fees
    for( ; ; )
    {
        if( qEmpty( pmcFee_q ) ) break;

        fee_p = (pmcFeeStruct_p)qRemoveFirst( pmcFee_q );
        mbFree( fee_p );
    }

    for( ; ; )
    {
        if( qEmpty( pmcIcd_q ) ) break;

        icd_p = (pmcIcdStruct_p)qRemoveFirst( pmcIcd_q );
        mbFree( icd_p->description_p );
        mbFree( icd_p );
    }

    for( ; ; )
    {
        if( qEmpty( pmcIcdSingle_q ) ) break;

        icd_p = (pmcIcdStruct_p)qRemoveFirst( pmcIcdSingle_q );
        mbFree( icd_p->description_p );
        mbFree( icd_p );
    }

    for( ; ; )
    {
        if( qEmpty( pmcPickListCfg_q ) ) break;

        pickList_p = (pmcPickListCfg_p)qRemoveFirst( pmcPickListCfg_q );
        mbFree( pickList_p->string_p );
        mbFree( pickList_p );
    }

    for( ; ; )
    {
        if( qEmpty( pmcExp_q ) ) break;

        exp_p = (pmcExpStruct_p)qRemoveFirst( pmcExp_q );
        mbFree( exp_p->string_p );
        mbFree( exp_p );
    }

    // Free items in config list
    for( i = 0 ; i < CFG_INVALID ; i++ )
    {
        if( pmcCfg[i].str_p ) mbFree( pmcCfg[i].str_p );
        pmcCfg[i].str_p = NIL;
        pmcCfg[i].init = FALSE;
    }

    mbFree( pmcClaimsin_p );
    mbFree( pmcInfo_p );
    mbFree( pmcValidrpt_p );
    mbFree( pmcNotice_p  );
    mbFree( pmcReturns_p );

    mbFree( pmcBuf1_p );
    mbFree( pmcBuf2_p );

    mbFree( pmcDefaultIcdCode_p );
    
    // Save the properties
    mbPropertySave( );
    mbPropertyClose( );

    // Free provider list queue
    while( !qEmpty( pmcProviderView_q ) )
    {
        pmcProviderViewList_p providerView_p;

        providerView_p = (pmcProviderViewList_p)qRemoveFirst( pmcProviderView_q );
        mbFree( providerView_p );
    }

    // Free the list of config items
    mbStrListFree( pmcDBConfig_q );

    // Terminate the registry
    pmcRegClose( );

    if( mbMallocChunks() != 0 )
    {
       mbLog( "Allocated memory chunks on shutdown: %d", mbMallocChunks() );
    }

    if( mbObjectCountGet() != 0 )
    {
        mbDlgInfo( "ObjectCount: %u", mbObjectCountGet() );
    }

    for( Ints_t i = 0 ; i < PMC_CLAIM_COUNT ; i++ )
    {
        mbFree( pmcDefaultClaimCode_p[i] );
    }

    mbMallocDump( );
    mbDlgTerminate( );
    mbLogTerminate( );

    return TRUE;
}

//---------------------------------------------------------------------------
// Function: pmcInitExpFileRead( )
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

Int32s_t            pmcInitExpFileRead( Char_p fileName_p )
{
    FILE           *in_p;
    Char_p          buf1_p;
    Char_p          buf2_p;
    pmcExpStruct_p  exp_p, exp2_p;
    Int32u_t        duplicateCount = 0;
    Boolean_t       duplicateFlag;
    Ints_t          len;
    Int32s_t        returnCode = FALSE;

    mbMalloc( buf1_p,  4096 );
    mbMalloc( buf2_p,  1024 );

    // Attempt to open ICD file
    in_p = fopen( fileName_p, "r" );
    if( in_p == NIL )
    {
        mbDlgExclaim( "Could not open EXP file '%s'.", fileName_p );
        goto exit;
    }

    exp_p = NIL;
    sprintf( buf1_p, "" );

    while( fgets( buf2_p, 1024, in_p ) != NULL )
    {
        len = strlen( buf2_p );

        if(    ( len == 3 )
            && (*buf2_p >= 'A' && *buf2_p <= 'Z' )
            && (*(buf2_p+1) >= 'A' && *(buf2_p+1) <= 'Z' ) )
        {
            // Found a new EXP code
            duplicateFlag = FALSE;

            // First, see if we should add an existing element to the list
            if( exp_p )
            {
                mbLockAcquire( pmcExp_q->lock );
                qInsertLast( pmcExp_q, exp_p );

                // Get length of EXP string
                len =  strlen( buf1_p );
                if( len )
                {
                    mbMalloc( exp_p->string_p, len + 1 );
                    strcpy( exp_p->string_p, buf1_p );
                }
                sprintf( buf1_p, "" );
                mbLockRelease( pmcExp_q->lock );
                // Testing pmcExpBox( exp_p->code );
                exp_p = NIL;
            }

            mbLockAcquire( pmcExp_q->lock );
            qWalk( exp2_p, pmcExp_q, pmcExpStruct_p )
            {
                if( ( len = mbStrPos( buf2_p, exp2_p->code ) ) >= 0 )
                {
                    duplicateFlag = TRUE;
                    duplicateCount++;
                    break;
                }
            }
            mbLockRelease( pmcExp_q->lock );

            if( duplicateFlag )
            {
                mbDlgExclaim( "Found a duplicate EXP code %s\n", buf2_p );
                continue;
            }
            mbCalloc( exp_p, sizeof( pmcExpStruct_t ) );

            // Copy the EXP code into the newly allocated structure
            mbStrClean( buf2_p, exp_p->code, TRUE );
        }
        else
        {
            // This must be sart of a string. Append to existing string
            strcat( buf1_p, buf2_p );
        }
    }

    // See if there is one last code to add
    if( exp_p )
    {
        mbLockAcquire( pmcExp_q->lock );
        qInsertLast( pmcExp_q, exp_p );

        // Get length of EXP string
        len =  strlen( buf1_p );
        if( len )
        {
            mbMalloc( exp_p->string_p, len + 1 );
            strcpy( exp_p->string_p, buf1_p );
        }

        mbLockRelease( pmcExp_q->lock );
    }

    returnCode = TRUE;

exit:

    mbLog( "Read %ld codes from EXP file '%s' (%ld duplictes)\n",
        pmcExp_q->size, fileName_p, duplicateCount );

    if( in_p ) fclose( in_p );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Function:
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void pmcPickListCfgAdd
(
    Char_p          in_p,
    Int32u_t        type
)
{
    Char_p          buf_p;
    pmcPickListCfg_p   pickList_p;
    Ints_t          len;

    mbMalloc( buf_p, 256 );

    mbStrClean( in_p, buf_p, TRUE );

    len = strlen( buf_p );
    if( len > 0 )
    {
        mbMalloc( pickList_p, sizeof(pmcPickListCfg_t) );
        mbMalloc( pickList_p->string_p, len + 1 );
        nbDlgDebug(( "adding '%s' to pick list queue", buf_p ));
        strcpy( pickList_p->string_p, buf_p );
        pickList_p->type = type;
        mbLockAcquire( pmcPickListCfg_q->lock );
        qInsertLast( pmcPickListCfg_q, pickList_p );
        mbLockRelease( pmcPickListCfg_q->lock );
    }
    mbFree( buf_p );
}



//---------------------------------------------------------------------------
// Function: pmcInitIcdFileRead( )
//---------------------------------------------------------------------------
// Description:
//
// ICD DIAGNOSTIC CODES FILE FORMAT
//
// FIELD NAME               FROM   TO    LEN   EXPLANATION/VALUE
// -----------------------  ----  ----  -----  -------------------------
// Diagnostic Code            1     3     3    Three character ICD9 code
//
// Diagnosis Description      4    74    71
//
//---------------------------------------------------------------------------

Int32s_t            pmcInitIcdFileRead( Char_p fileName_p )
{
    FILE           *in_p;
    Char_p          buf1_p;
    Char_p          buf2_p;
    pmcIcdStruct_p  icd_p, icd2_p;
    Int32u_t        i, j;
    Int32u_t        duplicateCount = 0;
    Boolean_t       duplicateFlag;
    Boolean_t       add;
    Boolean_t       gotLowerCase = FALSE;
    Boolean_t       gotUpperCase = FALSE;
    Ints_t          len;
    Int32s_t        returnCode = FALSE;

    mbMalloc( buf1_p, 1024 );
    mbMalloc( buf2_p, 1024 );

    // Attempt to open ICD file
    in_p = fopen( fileName_p, "r" );
    if( in_p == NIL )
    {
        mbDlgExclaim( "Could not open ICD file '%s'.", fileName_p );
        goto exit;
    }

    while( fgets( buf2_p, 1024, in_p ) != NULL )
    {
        // Clean line
        mbStrClean( buf2_p, buf1_p, TRUE );
        if( strlen( buf1_p ) < 4 ) continue;

        //printf( "Read line '%s' len (%d)\n", buf1_p, strlen( buf1_p ) );

        mbMalloc( icd_p, sizeof( pmcIcdStruct_t ) );
        memset( icd_p, 0 , sizeof( pmcIcdStruct_t ) );

        add = FALSE;
        // First Get the code
        len = strlen( buf1_p );
        if( len >= 3 )
        {
            memcpy( icd_p->code, buf1_p, 3 );
            icd_p->code[3] = 0;
            add = TRUE;
            if( mbStrPos( "0", icd_p->code ) >= 0 )
            {
                mbDlgDebug(( "Code has an OH in '%s'", icd_p->code ));
                pmcIcdContainsOhs = TRUE;
            }

            // Check for case insensitivity
            for( i = 0 ; i < 3 ; i++ )
            {
                if( icd_p->code[i] >= 'A' && icd_p->code[i] <= 'Z' ) gotUpperCase = TRUE;
                if( icd_p->code[i] >= 'a' && icd_p->code[i] <= 'z' ) gotLowerCase = TRUE;
            }
        }


        // Get the description
        if( len >= 4 )
        {
            for( i = 3 , j = 0 ; i < 73 ; i++ )
            {
                if( *( buf1_p + i ) == 0 ) break;
                *( buf2_p + j ) = *( buf1_p + i );
                j++;
            }
            *( buf2_p + j ) = 0;
            mbMalloc( icd_p->description_p, strlen( buf2_p ) + 2 );
            sprintf( icd_p->description_p, " %s", buf2_p );

            nbDlgDebug(( "Code '%s' description '%s'\n", icd_p->icdCode, icd_p->description_p ));
        }

        if( add )
        {
            // MAB: 20020421: Keep a separate list that does not have the
            // descriptions "merged".  This is for use in the ICD list form
            mbMalloc( icd2_p, sizeof( pmcIcdStruct_t ) );
            strcpy( icd2_p->code, icd_p->code );
            mbMalloc( icd2_p->description_p, strlen( icd_p->description_p ) + 1 );
            strcpy( icd2_p->description_p, icd_p->description_p );
            qInsertLast( pmcIcdSingle_q, icd2_p );

            // Look for duplicate entries
            duplicateFlag = FALSE;
            qWalk( icd2_p, pmcIcd_q, pmcIcdStruct_p )
            {
                if( mbStrPosLen( icd2_p->code, icd_p->code ) == 0 )
                {
                    duplicateCount++;
                    duplicateFlag = TRUE;

                    sprintf( buf2_p, "%s,%s", icd2_p->description_p, icd_p->description_p );
                    if( icd2_p->description_p ) mbFree( icd2_p->description_p );
                    mbMalloc( icd2_p->description_p, strlen( buf2_p ) + 1 );
                    strcpy( icd2_p->description_p, buf2_p );
                    break;
                }
            }
        }

        if( duplicateFlag == TRUE || add == FALSE )
        {
            if( icd_p->description_p ) mbFree( icd_p->description_p );
            mbFree( icd_p );
        }
        else
        {
            qInsertLast( pmcIcd_q, icd_p );
        }
    }

    returnCode = TRUE;

exit:

    // Set ICD case sensitivity
    pmcIcdCaseSensitive = FALSE;
    if( gotUpperCase && gotLowerCase )
    {
        pmcIcdCaseSensitive = TRUE;
    }

    mbLog( "Read %ld codes from ICD file '%s' (%ld duplictes)\n",
        pmcIcd_q->size, fileName_p, duplicateCount );

    if( in_p ) fclose( in_p );

    mbFree( buf1_p );
    mbFree( buf2_p );

    return returnCode;
}

//---------------------------------------------------------------------------
// Post-processes the list of local telephone exchanges
//---------------------------------------------------------------------------
Int32u_t pmcLocalExchangeUpdate( void )
{
    pmcLocalExchange_p  ex_p;
    Int32u_t            count = 0;

    mbLockAcquire( gLocalExchange_q->lock );

    qWalk( ex_p, gLocalExchange_q, pmcLocalExchange_p )
    {
        sprintf( ex_p->areaExchange, "%03d%s", pmcCfg[CFG_AREA_CODE].value, ex_p->exchange );
        nbDlgDebug(( "Local exchange '%s'", ex_p->areaExchange ));
        count++;
    }
    mbLockRelease( gLocalExchange_q->lock );
    return count;
}

//---------------------------------------------------------------------------
// This function gets the pat history types from the config queue
//---------------------------------------------------------------------------

Int32s_t pmcPatHistoryTypeInit( void )
{
    mbStrList_p     item_p;
    Char_t          buf[128];
    Ints_t          pos;
    Ints_t          type;

    mbLockAcquire( pmcDBConfig_q->lock ) ;
    mbLockAcquire( gPatHistoryType_q->lock );

    qWalk( item_p, pmcDBConfig_q, mbStrList_p )
    {
        if( item_p->handle == PMC_DB_CONFIG_CONSULT_SECTIONS )
        {
            sprintf( buf, "%s", item_p->str_p );

            pos = mbStrPos( buf, "TYPE:" );
            if( pos > 0 )
            {
                // Truncate string before "TYPE:"
                buf[pos] = 0;

                type = atoi( &buf[pos + 5] );
                mbStrListAddNew( gPatHistoryType_q, buf, item_p->str2_p, type, 0 );
            }
            else
            {
                mbDlgError( "Error parsing patient history configuration." );
            }
        }
    }

    mbLockRelease( pmcDBConfig_q->lock ) ;
    mbLockRelease( gPatHistoryType_q->lock );

    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// This function gets local telephone exchanges (prefixes) from the raw
// config list read from the database, and adds the prefixes to the list
//---------------------------------------------------------------------------

Int32s_t pmcLocalExchangeInit( void )
{
    mbStrList_p     item_p;
    Int32u_t        count = 0;

    qWalk( item_p, pmcDBConfig_q, mbStrList_p )
    {
        if( item_p->handle == PMC_DB_CONFIG_LOCALX )
        {
            pmcLocalExchangeAdd( item_p->str_p );
        }
    }

    count = pmcLocalExchangeUpdate( );
    mbLog( "Read %d local telephone prefixes", count );

    return MB_RET_OK;
}

//---------------------------------------------------------------------------
// This function adds local telephone exchanges (prefixes) to the list
//---------------------------------------------------------------------------

Int32s_t pmcLocalExchangeAdd( Char_p buf_p )
{
    Ints_t              i, l, j;
    Char_t              temp[16];
    Boolean_t           found = FALSE;
    pmcLocalExchange_p  ex_p;
    Int32s_t            result = MB_RET_OK;

    nbDlgDebug(( "Called with '%s'", buf_p ));
    l = strlen( buf_p );

    mbLockAcquire( gLocalExchange_q->lock );

    j = 0;
    for( i = 0 ; i <= l ; i++ )
    {
        if( *(buf_p+i) >= '0' && *(buf_p+i) <= '9' )
        {
            if( j < 5 ) temp[j++] = *(buf_p+i);
        }
        else
        {
            temp[j] = 0;
            if( j == 3 )
            {
                nbDlgDebug(( "Got exchange '%s'", temp ));

                found = FALSE;
                // Search to ensure that exchange is not already in list

                qWalk( ex_p, gLocalExchange_q, pmcLocalExchange_p )
                {
                    if( strcmp( temp, ex_p->exchange ) == 0 )
                    {
                        mbDlgExclaim( "Local telephone exchange '%s' already in list!", temp );
                        found = TRUE;
                        break;
                    }
                }

                if( found == FALSE )
                {
                    mbMalloc( ex_p, sizeof( pmcLocalExchange_t ) );
                    memset( ex_p, 0,  sizeof( pmcLocalExchange_t ) );
                    strncpy( ex_p->exchange, temp, 3 );
                    qInsertLast( gLocalExchange_q, ex_p );
                }
            }
            else
            {
                mbDlgDebug(( "Invalid exchange: '%s'", temp ));
                result = MB_RET_ERR;
            }
            j = 0;

            if( *(buf_p+i) == 0 ) break;
        }
    }

    mbLockRelease( gLocalExchange_q->lock );

    return result;
}







