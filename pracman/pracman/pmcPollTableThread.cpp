//---------------------------------------------------------------------------
// File:    pmcPollTableThread.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    Feb 15, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "mbUtils.h"
#include "mbSqlLib.h"

#include "pmcPollTableThread.h"
#include "pmcGlobals.h"
#include "pmcTables.h"
#include "pmcUtils.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall PollTableThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
__fastcall PollTableThread::PollTableThread(bool CreateSuspended)
    : TThread(CreateSuspended)
{
    Priority = tpIdle;
}

//---------------------------------------------------------------------------
// Name             // 0
// Type
// Row_format
// Rows
// Avg_row_length
// Data_length      // 5
// Max_data_length
// Index_length
// Data_free
// Auto_increment
// Create_time      // 10
// Update_time
// Check_time
// Create_options
// Comment          // 14
//
// MAB:20030202: The function below will break if the items returned by
// show table status vary from that shown above.
//---------------------------------------------------------------------------

void __fastcall PollTableThread::Execute()
{
    Ints_t              i;
    Char_p              table_p;
    MbSQL              *sql_p;
    Int32u_t            mySkipCount = 0;

    Sleep( 1000 * 1 );

    while( !Terminated )
    {
        mySkipCount = 0;
        pmcInPoll = TRUE;
        if( gSuspendPoll == 0 )
        {
            sql_p = new MbSQL( "show table status" );

            while( sql_p->RowGet() )
            {
                if( ( table_p = sql_p->String( 0 ) ) == NIL ) break;
                for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
                {
                    if( strcmp( table_p, pmcTableNames_p[i] ) == 0 )
                    {
                         pmcPollTableModifyTime[i] = sql_p->DateTimeInt64u( 12 );
                         pmcPollTableSize[i] = sql_p->Int32u( 6 );
                         break;
                    }
                }
            }
            delete sql_p;
        }
        else
        {
            pmcThreadSkips++;
            mySkipCount++;
            if( mySkipCount > 30 )
            {
                mbLog( "gSuspendPoll seems to be stuck non-0: %u\n", gSuspendPoll );
                mySkipCount = 0;
            }
        }
        pmcInPoll = FALSE;
        pmcThreadTicks++;
        Sleep( 950 );
    }
    pmcThreadTicks = 0;
}

//---------------------------------------------------------------------------
// This hack allows some table times to be read at initialization time
//---------------------------------------------------------------------------

void pmcTableStatusUpdate( Char_p tableName_p )
{
    Ints_t              i;
    Char_p              table_p;
    MbSQL              *sql_p;

    sql_p = new MbSQL( "show table status" );

    while( sql_p->RowGet() )
    {
        if( ( table_p = sql_p->String( 0 ) ) == NIL ) break;
        for( i = 0 ; i < PMC_TABLE_COUNT ; i++ )
        {
            if( strcmp( table_p, pmcTableNames_p[i] ) == 0 )
            {
                if( strcmp( table_p, tableName_p ) == 0 )
                {
                    pmcPollTableModifyTime[i] = sql_p->DateTimeInt64u( 12 );
                    pmcPollTableSize[i] = sql_p->Int32u( 6 );
                    break;
                }
            }
        }
    }
    delete sql_p;
}
