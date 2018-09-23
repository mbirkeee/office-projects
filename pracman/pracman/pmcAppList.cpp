//---------------------------------------------------------------------------
// File:    pmcAppList.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada                    
//---------------------------------------------------------------------------
// Date:    Feb 15, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"
                 
#include "pmcDefines.h"
#include "pmcTables.h"
#include "pmcGlobals.h"
#include "pmcUtils.h"
#include "pmcMainForm.h"
#include "pmcAppList.h"
#include "pmcProviderView.h"

//---------------------------------------------------------------------------
// Function:  TMainForm::DayInfoUpdate()
//---------------------------------------------------------------------------
// Description:
//
// Update day view appointment array if required
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::DayViewInfoUpdate
(
    pmcAppViewInfo_p        day_p
)
{
    Int32u_t                i, k;
    Int32s_t                update = FALSE;
    AnsiString              str = "";
    AnsiString              tempStr;
    Char_p                  buf_p = NIL;
    pmcAppInfo_p            app_p;
    qHead_t                 queue;
    Variant                 time;
    Int32u_t                startTime;
    Int32u_t                slotCount;
    Int32u_t                today;

    // Don't proceed if patients list has not been read yet
    if( UpdatePatientListDone == FALSE ) goto exit;

    if( day_p->updateForce != TRUE )
    {
        // Want ability to force skip of update
        if( day_p->updateSkip == TRUE ) goto exit;

        if( day_p->startDate != SelectedDate || day_p->providerId != SelectedProviderId )
        {
            update = TRUE;
            nbDlgDebug(( "Selected date %ld %ld %ld %ld",  day_p->startDate, SelectedDate,
            day_p->providerId, SelectedProviderId ));
        }
        else
        {
            if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ] > pmcDayViewAppLastReadTime )
            {
                update = TRUE;
            }
            else if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcDayViewSlotLastReadTime )
            {
                update = TRUE;
            }
            else
            {
                // Has the appointment table or patient table been modified?
                if( RefreshTable( PMC_TABLE_BIT_APPS | PMC_TABLE_BIT_TIMESLOTS ) ) update = TRUE;
            }
        }
        if( update == FALSE ) goto exit;
    }

    mbMalloc( buf_p, 2048 );

    today = mbToday( );

    update = TRUE;
    day_p->updateForce = FALSE;

    PMC_INC_USE( day_p->notReady );

    // This will get latest time of table
    RefreshTable( PMC_TABLE_BIT_APPS );

    // Initialize time slots array.  All times unavailable by default
    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        day_p->slot_p[i].countt = 1;
        day_p->slot_p[i].available = FALSE;
        day_p->slot_p[i].appointId[1] = 0;
        day_p->slot_p[i].appointId[0] = 0;
        day_p->slot_p[i].color = pmcColor[ PMC_COLOR_BREAK ];
        day_p->slot_p[i].dur[0] = NULL;
        day_p->slot_p[i].firstName[0] = NULL;
        day_p->slot_p[i].lastName[0] = NULL;
        day_p->slot_p[i].title[0] = NULL;
        day_p->slot_p[i].homePhone[0] = NULL;
        day_p->slot_p[i].areaCode[0] = NULL;
        day_p->slot_p[i].type[0] = NULL;
        day_p->slot_p[i].commentIn[0] = NULL;
        day_p->slot_p[i].last = FALSE;
        day_p->slot_p[i].first = FALSE;
        day_p->slot_p[i].refDrName[0] = NULL;
        day_p->slot_p[i].confirmedLetterState = PMC_APP_CONFIRMED_NO;
        day_p->slot_p[i].confirmedPhoneState  = PMC_APP_CONFIRMED_NO;
        day_p->slot_p[i].date = 0;
        day_p->slot_p[i].providerId = 0;
    }

    day_p->providerId = SelectedProviderId;
    day_p->startDate = SelectedDate;

    // Determine the available times
    TimeSlotsGet( day_p, NIL );

    // Next, look for matching appointments from the database
    qInitialize( &queue );

    // Get the actual list of appointments
    AppListGet( &queue, SelectedDate, SelectedDate );

    // Find appointment conflicts
    FindConflicts( &queue, day_p );
    CullProviders( &queue, day_p, NIL );

    // Now loop through the appointments, adding them to the day view array
    for( ; ; )
    {
        if( qEmpty( &queue ) ) break;

        app_p = (pmcAppInfo_p)qRemoveFirst( &queue );

        slotCount = 0;
        for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
        {
            startTime = day_p->slot_p[i].startTime;

            if( ( startTime >= app_p->startTime ) && ( startTime < app_p->stopTime ) )
            {
                slotCount++;

                // Determine if at end of appointment line should be drawn
                if( i < PMC_TIMESLOTS_PER_DAY - 1 )
                {
                    if( day_p->slot_p[i+1].startTime >= app_p->stopTime )
                    {
                        // Must be last slot in appt.
                         day_p->slot_p[i].last = TRUE;
                    }
                }

                if( slotCount == 1 ) day_p->slot_p[i].first = TRUE;

                day_p->slot_p[i].countt++;

                if( day_p->slot_p[i].countt > PMC_MAX_APPS_PER_SLOT )
                {
                    day_p->slot_p[i].countt = PMC_MAX_APPS_PER_SLOT;
                    mbDlgDebug(( "WARNING: Max. appointments per time slot exceeded!" ));
                }

                // Put the ID in the array indexed by count, not count-1
                day_p->slot_p[i].appointId[ day_p->slot_p[i].countt ] = app_p->id;

                // MAB:20020717: Modify to only indicate conflicts on future appointments.
                // This is because they want to mark (and see) past appointments as
                // completed, even if there were conflicts.
                //if( day_p->slot_p[i].countt > 1 || ( app_p->conflictCount && app_p->date > today ) )
                if( day_p->slot_p[i].countt > 1 )
                {
                    day_p->slot_p[i].color = pmcColor[ PMC_COLOR_CONFLICT ];
                    for( k = 1 ; k < (Int32u_t)day_p->slot_p[i].countt ; k++ )
                    {
                        PMC_COLOR_DARKEN( day_p->slot_p[i].color );
                    }
                }
                else
                {
                    // Determine the color
                    if( app_p->date < today )
                    {
                        if( app_p->completed )
                        {
                            // This appointment is in the past
                            day_p->slot_p[i].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                        else
                        {
                            // This appointment is in the past and not marked complete
                            day_p->slot_p[i].color = pmcColor[ PMC_COLOR_NOSHOW ];
                        }
                    }
                    else
                    {
                        // MAB:20010315: Decide to color just the confirmed column
                        // if( app_p->confirmedLetterFlag || app_p->confirmedPhoneFlag )
                        // {
                        //      day_p->slot[i].color = Color[ PMC_COLOR_CONFIRMED ];
                        // }
                        // else
                        {
                            day_p->slot_p[i].color = pmcColor[ PMC_COLOR_APP ];
                        }

                        if( app_p->completed && app_p->date == today )
                        {
                            // Allow todays appointments to be displayed as completed
                            day_p->slot_p[i].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                    }
                    if( app_p->conflictCount )
                    {
                        PMC_COLOR_DARKEN( day_p->slot_p[i].color );
                    }
                    if( app_p->appCount )
                    {
                        PMC_COLOR_DARKEN( day_p->slot_p[i].color );
                    }
                }

                // Get strings (for first slot only)
                if( slotCount == 1 )
                {
                    strcpy( day_p->slot_p[i].refDrName,   app_p->refDrName );
                    sprintf( day_p->slot_p[i].dur, "%d",  app_p->duration );
                    strcpy( day_p->slot_p[i].firstName,   app_p->firstName);
                    strcpy( day_p->slot_p[i].lastName,    app_p->lastName );
                    strcpy( day_p->slot_p[i].title,       app_p->title);
                    strcpy( day_p->slot_p[i].homePhone,   app_p->homePhone );
                    strcpy( day_p->slot_p[i].areaCode,    app_p->areaCode );
                    strcpy( day_p->slot_p[i].commentIn,   app_p->commentIn );
                    strcpy( day_p->slot_p[i].type,        pmcAppTypeStrings[app_p->type] );
                    day_p->slot_p[i].confirmedPhoneState =  app_p->confirmedPhoneState;
                    day_p->slot_p[i].confirmedLetterState = app_p->confirmedLetterState;
                }
            }
        }
        mbFree( app_p );
    }

    // Record last read time for timer updates
    pmcDayViewAppLastReadTime =  pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_APPS );
    pmcDayViewSlotLastReadTime = pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_TIMESLOTS );

    PMC_DEC_USE( day_p->notReady );

exit:

    if( buf_p ) mbFree( buf_p );
    return update;
}

//---------------------------------------------------------------------------
// Function:  TMainForm::WeekViewInfoUpdate()
//---------------------------------------------------------------------------
// Description:
//
// Update day view appointment array if required
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::WeekViewInfoUpdate
(
    pmcAppViewInfo_p        week_p
)
{
    Int32u_t                i, j;
    Int32s_t                update = FALSE;
    AnsiString              str = "";
    AnsiString              tempStr;
    Char_p                  buf_p = NIL;
    pmcAppInfo_p            app_p;
    qHead_t                 queue;
    Variant                 time;
    Int32u_t                startTime;
    Int32u_t                slotCount;
    Int32u_t                slotIndex;
    bool                    found;
    Int32u_t                today;

    // Don't proceed if patients list has not been read yet
    if( UpdatePatientListDone == FALSE ) goto exit;

    if( WeekViewInfo.updateForce != TRUE )
    {
        // Want ability to force skip of update
        if( WeekViewInfo.updateSkip == TRUE ) goto exit;

        if( week_p->startDate != SelectedWeekViewDate[0] || week_p->providerId != SelectedProviderId )
        {
            update = TRUE;
        }
        else
        {
            if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ] > pmcWeekViewAppLastReadTime  )
            {
                update = TRUE;
            }
            else if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcWeekViewSlotLastReadTime  )
            {
                update = TRUE;
            }
            else
            {
                // Has the appointment table or timeslot table been modified?
                if( RefreshTable( PMC_TABLE_BIT_APPS | PMC_TABLE_BIT_TIMESLOTS ) ) update = TRUE;
            }
        }
        if( update == FALSE ) goto exit;
    }

    mbMalloc( buf_p, 515 );

    today = mbToday( );

    update = TRUE;

    WeekViewInfo.updateForce = FALSE;

    PMC_INC_USE( WeekViewInfo.notReady );

    // This will get latest time of table
    RefreshTable( PMC_TABLE_BIT_APPS );

    // Initialize time slots array.  All times unavailable by default
    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        for( j = 0 ; j < 7 ; j++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;

            week_p->slot_p[ slotIndex ].available = FALSE;
            week_p->slot_p[ slotIndex ].countt = 1;
            week_p->slot_p[ slotIndex ].appointId[1] = 0;
            week_p->slot_p[ slotIndex ].appointId[0] = 0;
            week_p->slot_p[ slotIndex ].color = pmcColor[ PMC_COLOR_BREAK ];
            week_p->slot_p[ slotIndex ].dur[0] = NULL;
            week_p->slot_p[ slotIndex ].firstName[0] = NULL;
            week_p->slot_p[ slotIndex ].lastName[0] = NULL;
            week_p->slot_p[ slotIndex ].title[0] = NULL;
            week_p->slot_p[ slotIndex ].homePhone[0] = NULL;
            week_p->slot_p[ slotIndex ].areaCode[0] = NULL;
            week_p->slot_p[ slotIndex ].type[0] = NULL;
            week_p->slot_p[ slotIndex ].last = FALSE;
            week_p->slot_p[ slotIndex ].first = FALSE;
            week_p->slot_p[ slotIndex ].refDrName[0] = NULL;
            week_p->slot_p[ slotIndex ].confirmedLetterState = PMC_APP_CONFIRMED_NO;
            week_p->slot_p[ slotIndex ].confirmedPhoneState  = PMC_APP_CONFIRMED_NO;
            week_p->slot_p[ slotIndex ].commentIn[0] = NULL;
            week_p->slot_p[ slotIndex ].providerId = 0;
            week_p->slot_p[ slotIndex ].date = 0;
        }
    }

    week_p->providerId = SelectedProviderId;
    week_p->startDate = SelectedWeekViewDate[0];

    // Determine the available timeslots
    TimeSlotsGet( week_p, NIL );

    // Next, look for matching appointments from the database
    qInitialize( &queue );

    AppListGet( &queue, SelectedWeekViewDate[0], SelectedWeekViewDate[6] );
    FindConflicts( &queue, week_p );
    CullProviders( &queue, week_p, NIL );

    // Now loop through the appointments, adding them to the array
    for( ; ; )
    {
        if( qEmpty( &queue ) ) break;

        app_p = (pmcAppInfo_p)qRemoveFirst( &queue );

        // Must determine which day of week this appointment is for
        for( found = FALSE, j = 0 ; j < 7 ; j++ )
        {
            if( app_p->date == SelectedWeekViewDate[j] )
            {
                found = TRUE;
                break;
            }
        }

        // Sanity Check
        if( !found ) mbDlgDebug(( "Did not find matching day for appointment!\n" ));

        // j now contains proper day of week
        slotCount = 0;
        for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
        {
            slotIndex =  j * PMC_TIMESLOTS_PER_DAY + i;

            startTime = week_p->slot_p[j * PMC_TIMESLOTS_PER_DAY + i].startTime;

            if( ( startTime >= app_p->startTime ) && ( startTime < app_p->stopTime ) )
            {
                slotCount++;

                // Determine if at end of appointment line should be drawn
                if( i < PMC_TIMESLOTS_PER_DAY - 1 )
                {
                    if( week_p->slot_p[slotIndex + 1].startTime >= app_p->stopTime )
                    {
                        // Must be last slot in appt.
                        week_p->slot_p[slotIndex].last = TRUE;
                    }
                }

                if( slotCount == 1 ) week_p->slot_p[slotIndex].first = TRUE;

                week_p->slot_p[slotIndex].countt++;
                if( week_p->slot_p[slotIndex].countt > PMC_MAX_APPS_PER_SLOT )
                {
                    week_p->slot_p[slotIndex].countt = PMC_MAX_APPS_PER_SLOT;
                    mbDlgDebug(( "WARNING: Max. appointments per time slot exceeded!" ));
                }

                // Put the ID in the array indexed by count, not count-1
                week_p->slot_p[slotIndex].appointId[week_p->slot_p[slotIndex].countt] = app_p->id;

                // MAB:20020717: Modify to only indicate conflicts on future appointments.
                // This is because they want to mark (and see) past appointments as
                // completed, even if there were conflicts.
                // if( week_p->slot_p[slotIndex].count > 1 || app_p->conflictCount  )
                // if( week_p->slot_p[slotIndex].count > 1 || ( app_p->conflictCount  && app_p->date > today ) )
                if( week_p->slot_p[slotIndex].countt > 1 )
                {
                    week_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_CONFLICT ];
                    for( Int16u_t k = 1 ; k < week_p->slot_p[slotIndex].countt ; k++ )
                    {
                        PMC_COLOR_DARKEN( week_p->slot_p[slotIndex].color );
                    }
                }
                else
                {
                    // Determine the color
                    if( app_p->date < today )
                    {
                        if( app_p->completed )
                        {
                            // This appointment is in the past
                            week_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                        else
                        {
                            // This appointment is in the past and not marked complete
                            week_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_NOSHOW ];
                        }
                    }
                    else
                    {
                        // MAB:20010315: Decide to color just the confirmed column
                        // if( app_p->confirmedLetterFlag || app_p->confirmedPhoneFlag )
                        // {
                        //      day_p->slot[i].color = Color[ PMC_COLOR_CONFIRMED ];
                        // }
                        // else
                        {
                            week_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_APP ];
                        }

                        if( app_p->completed && app_p->date == today )
                        {
                            // Allow todays appointments to be displayed as completed
                            week_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                    }
                    if( app_p->conflictCount )
                    {
                        PMC_COLOR_DARKEN( week_p->slot_p[slotIndex].color );
                    }
                    if( app_p->appCount )
                    {
                        PMC_COLOR_DARKEN( week_p->slot_p[slotIndex].color );
                    }
                }

                // Get strings (for first slot only)
                if( slotCount == 1 )
                {
                    strcpy( week_p->slot_p[slotIndex].refDrName,   app_p->refDrName );
                    sprintf( week_p->slot_p[slotIndex].dur, "%d",  app_p->duration );
                    strcpy( week_p->slot_p[slotIndex].firstName,   app_p->firstName);
                    strcpy( week_p->slot_p[slotIndex].lastName,    app_p->lastName );
                    strcpy( week_p->slot_p[slotIndex].title,       app_p->title);
                    strcpy( week_p->slot_p[slotIndex].homePhone,   app_p->homePhone );
                    strcpy( week_p->slot_p[slotIndex].areaCode,    app_p->areaCode );
                    strcpy( week_p->slot_p[slotIndex].commentIn,   app_p->commentIn );
                    strcpy( week_p->slot_p[slotIndex].type,        pmcAppTypeStrings[app_p->type] );
                    week_p->slot_p[slotIndex].confirmedPhoneState =  app_p->confirmedPhoneState;
                    week_p->slot_p[slotIndex].confirmedLetterState = app_p->confirmedLetterState;
                }
            }
        }
        mbFree( app_p );
    }

    // Record last read time for timer updates
    pmcWeekViewAppLastReadTime =  pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_APPS );
    pmcWeekViewSlotLastReadTime =  pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_TIMESLOTS );

    PMC_DEC_USE( WeekViewInfo.notReady );

exit:
    if( buf_p ) mbFree( buf_p );
    return update;
}

//---------------------------------------------------------------------------
// Function:  TMainForm::MonthViewInfoUpdate()
//---------------------------------------------------------------------------
// Description:
//
// Update month view appointment array if required
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::MonthViewInfoUpdate
(
    pmcAppViewInfo_p        month_p
)
{
    Int32u_t                i, j;
    Int32s_t                update = FALSE;
    AnsiString              str = "";
    AnsiString              tempStr;
    Char_p                  buf_p = NIL;
    pmcAppInfo_p            app_p;
    qHead_t                 queue;
    Variant                 time;
    Int32u_t                startTime;
    Int32u_t                slotCount;
    bool                    found;
    Int32u_t                today;
    Int32u_t                slotIndex;

    // Don't proceed if patients list has not been read yet
    if( UpdatePatientListDone == FALSE ) goto exit;

    if( MonthViewInfo.updateForce != TRUE )
    {
        // Want ability to force skip of update
        if( MonthViewInfo.updateSkip == TRUE ) goto exit;

        if( month_p->startDate != SelectedMonthViewDate[0] || month_p->providerId != SelectedProviderId )
        {
            update = TRUE;
        }
        else
        {
            if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ] > pmcMonthViewAppLastReadTime )
            {
                update = TRUE;
            }
            else if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcMonthViewSlotLastReadTime )
            {
                update = TRUE;
            }
            else
            {
                // Has the appointment table or timeslot table been modified?
                if( RefreshTable( PMC_TABLE_BIT_APPS | PMC_TABLE_BIT_TIMESLOTS ) ) update = TRUE;
            }
        }

        if( update == FALSE ) goto exit;
    }

    mbMalloc( buf_p, 2048 );

    today = mbToday( );

    update = TRUE;
    MonthViewInfo.updateForce = FALSE;

    PMC_INC_USE( MonthViewInfo.notReady );

    // This will get latest time of table
    RefreshTable( PMC_TABLE_BIT_APPS );

    // Initialize time slots array.  All times unavailable by default
    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        for( j = 0 ; j < PMC_MONTH_VIEW_COLS ; j++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;

            month_p->slot_p[slotIndex].countt = 1;
            month_p->slot_p[slotIndex].available = FALSE;
            month_p->slot_p[slotIndex].appointId[1] = 0;
            month_p->slot_p[slotIndex].appointId[0] = 0;
            month_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_BREAK ];
            month_p->slot_p[slotIndex].dur[0] = NULL;
            month_p->slot_p[slotIndex].firstName[0] = NULL;
            month_p->slot_p[slotIndex].lastName[0] = NULL;
            month_p->slot_p[slotIndex].title[0] = NULL;
            month_p->slot_p[slotIndex].homePhone[0] = NULL;
            month_p->slot_p[slotIndex].areaCode[0] = NULL;
            month_p->slot_p[slotIndex].type[0] = NULL;
            month_p->slot_p[slotIndex].commentIn[0] = NULL;
            month_p->slot_p[slotIndex].last = FALSE;
            month_p->slot_p[slotIndex].first = FALSE;
            month_p->slot_p[slotIndex].refDrName[0] = NULL;
            month_p->slot_p[slotIndex].confirmedLetterState = PMC_APP_CONFIRMED_NO;
            month_p->slot_p[slotIndex].confirmedPhoneState  = PMC_APP_CONFIRMED_NO;
            month_p->slot_p[slotIndex].providerId = 0;
            month_p->slot_p[slotIndex].date = 0;
        }
    }

    month_p->providerId = SelectedProviderId;
    month_p->startDate = SelectedMonthViewDate[0];

    // Determine the available time slots
    TimeSlotsGet( month_p, NIL );

    // Next, look for matching appointments from the database
    qInitialize( &queue );

    // Get the appointments from the database
    AppListGet( &queue, SelectedMonthViewDate[0],  SelectedMonthViewDateEnd );
    FindConflicts( &queue, month_p );
    CullProviders( &queue, month_p, NIL );

    // Now loop through the appointments, adding them to the day view array
    for( ; ; )
    {
        if( qEmpty( &queue ) ) break;

        app_p = (pmcAppInfo_p)qRemoveFirst( &queue );

        nbDlgDebug(( " appointment start time %ld duration %ld stop time %ld",
            app_p->startTime,
            app_p->duration,
            app_p->stopTime ));

        // Must determine which day of month this appointment is for
        for( found = FALSE, j = 0 ; j < 31 ; j++ )
        {
            if( app_p->date == SelectedMonthViewDate[j] )
            {
                found = TRUE;
                break;
            }
        }

        // Sanity Check
        if( !found ) mbDlgDebug(( "Did not find matching day for appointment!\n" ));

        // j now contains proper day of month
        slotCount = 0;
        for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;
            startTime = month_p->slot_p[slotIndex].startTime;

            if( ( startTime >= app_p->startTime ) && ( startTime < app_p->stopTime ) )
            {
                slotCount++;

                // Determine if at end of appointment line should be drawn
                if( i < PMC_TIMESLOTS_PER_DAY - 1 )
                {
                    if( month_p->slot_p[slotIndex+1].startTime >= app_p->stopTime )
                    {
                        // Must be last slot in appt.
                         month_p->slot_p[slotIndex].last = TRUE;
                    }
                }

                if( slotCount == 1 ) month_p->slot_p[slotIndex].first = TRUE;

                month_p->slot_p[slotIndex].countt++;
                if( month_p->slot_p[slotIndex].countt > PMC_MAX_APPS_PER_SLOT )
                {
                    month_p->slot_p[slotIndex].countt = PMC_MAX_APPS_PER_SLOT;
                    mbDlgDebug(( "WARNING: Max. appointments per time slot exceeded!" ));
                }

                // Put the ID in the array indexed by count, not count-1
                month_p->slot_p[slotIndex].appointId[month_p->slot_p[slotIndex].countt] = app_p->id;

                // MAB:20020717: Modify to only indicate conflicts on future appointments.
                // This is because they want to mark (and see) past appointments as
                // completed, even if there were conflicts.
                // if( month_p->slot_p[slotIndex].count > 1 || app_p->conflictCount )
                //if( month_p->slot_p[slotIndex].count > 1 || ( app_p->conflictCount && app_p->date > today ) )
                if( month_p->slot_p[slotIndex].countt > 1 )
                {
                    month_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_CONFLICT ];
                    for( Int16u_t k = 1 ; k < month_p->slot_p[slotIndex].countt ; k++ )
                    {
                        PMC_COLOR_DARKEN( month_p->slot_p[slotIndex].color );
                    }
                }
                else
                {
                    // Determine the color
                    if( app_p->date < today )
                    {
                        if( app_p->completed )
                        {
                            // This appointment is in the past
                            month_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                        else
                        {
                            // This appointment is in the past and not marked complete
                            month_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_NOSHOW ];
                        }
                    }
                    else
                    {
                        // MAB:20010315: Decide to color just the confirmed column
                        // if( app_p->confirmedLetterFlag || app_p->confirmedPhoneFlag )
                        // {
                        //      day_p->slot[i].color = Color[ PMC_COLOR_CONFIRMED ];
                        // }
                        // else
                        {
                            month_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_APP ];
                        }

                        if( app_p->completed && app_p->date == today )
                        {
                            // Allow todays appointments to be displayed as completed
                            month_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                    }
                    if( app_p->conflictCount )
                    {
                        PMC_COLOR_DARKEN( month_p->slot_p[slotIndex].color );
                    }
                    if( app_p->appCount )
                    {
                        PMC_COLOR_DARKEN( month_p->slot_p[slotIndex].color );
                    }
                 }

                // Get strings (for first slot only)
                if( slotCount == 1 )
                {
                    strcpy( month_p->slot_p[slotIndex].refDrName,   app_p->refDrName );
                    sprintf( month_p->slot_p[slotIndex].dur, "%d",  app_p->duration );
                    strcpy( month_p->slot_p[slotIndex].firstName,   app_p->firstName);
                    strcpy( month_p->slot_p[slotIndex].lastName,    app_p->lastName );
                    strcpy( month_p->slot_p[slotIndex].title,       app_p->title);
                    strcpy( month_p->slot_p[slotIndex].homePhone,   app_p->homePhone );
                    strcpy( month_p->slot_p[slotIndex].areaCode,    app_p->areaCode );
                    strcpy( month_p->slot_p[slotIndex].commentIn,   app_p->commentIn );
                    strcpy( month_p->slot_p[slotIndex].type,        pmcAppTypeStrings[app_p->type] );
                    month_p->slot_p[slotIndex].confirmedPhoneState =  app_p->confirmedPhoneState;
                    month_p->slot_p[slotIndex].confirmedLetterState = app_p->confirmedLetterState;
                }
                else
                {
                    // This is not the first slot in an appointment
                    // MAB:DBG sprintf( day_p->slot[i].commentIn, "DBG: id: %d %s", app_p->id, app_p->commentIn );
                }
            }
        }
        mbFree( app_p );
    }

    // Record last read time for timer updates
    pmcMonthViewAppLastReadTime =  pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_APPS );
    pmcMonthViewSlotLastReadTime = pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_TIMESLOTS );

    PMC_DEC_USE( MonthViewInfo.notReady );

exit:
    if( buf_p ) mbFree( buf_p );
    return update;
}

//---------------------------------------------------------------------------
// Function:  TMainForm::ProviderViewInfoUpdate()
//---------------------------------------------------------------------------
// Description:
//
// Update day view appointment array if required
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::ProviderViewInfoUpdate
(
    pmcAppViewInfo_p        view_p
)
{
    Int32u_t                i, j;
    Int32s_t                update = FALSE;
    AnsiString              str = "";
    AnsiString              tempStr;
    Char_p                  buf_p = NIL;
    pmcAppInfo_p            app_p;
    qHead_t                 queue;
    Variant                 time;
    Int32u_t                startTime;
    Int32u_t                slotCount;
    Int32u_t                slotIndex;
    bool                    found;
    Int32u_t                today;
    pmcProviderViewList_p   providerView_p;

    // Don't proceed if patients list has not been read yet
    if( UpdatePatientListDone == FALSE ) goto exit;

    if( ProviderViewInfo.updateForce != TRUE )
    {
        // Want ability to force skip of update
        if( ProviderViewInfo.updateSkip == TRUE ) goto exit;

        if( view_p->startDate != SelectedDate )
        {
            update = TRUE;
        }
        else
        {
            if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_APPS ] > pmcProviderViewAppLastReadTime  )
            {
                update = TRUE;
            }
            else if( pmcPollTableModifyTime[ PMC_TABLE_INDEX_TIMESLOTS ] > pmcProviderViewSlotLastReadTime  )
            {
                update = TRUE;
            }
            else
            {
                // Has the appointment table or timeslot table been modified?
                if( RefreshTable( PMC_TABLE_BIT_APPS | PMC_TABLE_BIT_TIMESLOTS ) ) update = TRUE;
            }
        }
        if( update == FALSE ) goto exit;
    }

    mbMalloc( buf_p, 515 );

    today = mbToday( );

    update = TRUE;

    ProviderViewInfo.updateForce = FALSE;

    PMC_INC_USE( ProviderViewInfo.notReady );

    // This will get latest time of table
    RefreshTable( PMC_TABLE_BIT_APPS );

    // Initialize time slots array.  All times unavailable by default
    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        for( j = 0 ; j < PMC_PROVIDER_VIEW_COLS ; j++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;

            view_p->slot_p[ slotIndex ].available = FALSE;
            view_p->slot_p[ slotIndex ].countt = 1;
            view_p->slot_p[ slotIndex ].appointId[1] = 0;
            view_p->slot_p[ slotIndex ].appointId[0] = 0;
            view_p->slot_p[ slotIndex ].color = pmcColor[ PMC_COLOR_BREAK ];
            view_p->slot_p[ slotIndex ].dur[0] = NULL;
            view_p->slot_p[ slotIndex ].firstName[0] = NULL;
            view_p->slot_p[ slotIndex ].lastName[0] = NULL;
            view_p->slot_p[ slotIndex ].title[0] = NULL;
            view_p->slot_p[ slotIndex ].homePhone[0] = NULL;
            view_p->slot_p[ slotIndex ].areaCode[0] = NULL;
            view_p->slot_p[ slotIndex ].type[0] = NULL;
            view_p->slot_p[ slotIndex ].last = FALSE;
            view_p->slot_p[ slotIndex ].first = FALSE;
            view_p->slot_p[ slotIndex ].refDrName[0] = NULL;
            view_p->slot_p[ slotIndex ].confirmedLetterState = PMC_APP_CONFIRMED_NO;
            view_p->slot_p[ slotIndex ].confirmedPhoneState  = PMC_APP_CONFIRMED_NO;
            view_p->slot_p[ slotIndex ].commentIn[0] = NULL;
            view_p->slot_p[ slotIndex ].providerId = 0;
            view_p->slot_p[ slotIndex ].date = 0;
        }
    }

    view_p->providerId = SelectedProviderId;
    view_p->startDate = SelectedDate;

    // Determine the available timeslots
    TimeSlotsGet( view_p, pmcProviderView_q );

    // Next, look for matching appointments from the database
    qInitialize( &queue );
    AppListGet( &queue, view_p->startDate, view_p->startDate );
    FindConflicts( &queue, view_p );
    CullProviders( &queue, view_p, pmcProviderView_q );

    // Now loop through the appointments, adding them to the array
    for( ; ; )
    {
        if( qEmpty( &queue ) ) break;

        app_p = (pmcAppInfo_p)qRemoveFirst( &queue );

        mbLockAcquire( pmcProviderView_q->lock );
        j = 0;
        found = FALSE;

        qWalk( providerView_p, pmcProviderView_q, pmcProviderViewList_p )
        {
            if( app_p->providerId == providerView_p->providerId )
            {
                found = TRUE;
                break;
            }
            j++;
        }
        mbLockRelease( pmcProviderView_q->lock );

        // Sanity Check
        if( !found ) mbDlgDebug(( "Did not find provider for appointment!\n" ));

        // j now contains proper column (provider)
        slotCount = 0;
        for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
        {
            slotIndex =  j * PMC_TIMESLOTS_PER_DAY + i;

            startTime = view_p->slot_p[j * PMC_TIMESLOTS_PER_DAY + i].startTime;

            if( ( startTime >= app_p->startTime ) && ( startTime < app_p->stopTime ) )
            {
                slotCount++;
                // Determine if at end of appointment line should be drawn
                if( i < PMC_TIMESLOTS_PER_DAY - 1 )
                {
                    if( view_p->slot_p[slotIndex + 1].startTime >= app_p->stopTime )
                    {
                        // Must be last slot in appt.
                        view_p->slot_p[slotIndex].last = TRUE;
                    }
                }

                if( slotCount == 1 ) view_p->slot_p[slotIndex].first = TRUE;

                view_p->slot_p[slotIndex].countt++;
                if( view_p->slot_p[slotIndex].countt > PMC_MAX_APPS_PER_SLOT )
                {
                    view_p->slot_p[slotIndex].countt = PMC_MAX_APPS_PER_SLOT;
                    mbDlgDebug(( "WARNING: Max. appointments per time slot exceeded!" ));
                }

                // Put the ID in the array indexed by count, not count-1
                view_p->slot_p[slotIndex].appointId[view_p->slot_p[slotIndex].countt] = app_p->id;

                // MAB:20020717: Modify to only indicate conflicts on future appointments.
                // This is because they want to mark (and see) past appointments as
                // completed, even if there were conflicts.
                // if( view_p->slot_p[slotIndex].count > 1 || app_p->conflictCount > 0 )

                //if( view_p->slot_p[slotIndex].count > 1 || ( app_p->conflictCount > 0 && app_p->date > today ) )
                if( view_p->slot_p[slotIndex].countt > 1 )
                {
                    view_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_CONFLICT ];
                    for( Int16u_t k = 1 ; k < view_p->slot_p[slotIndex].countt ; k++ )
                    {
                        PMC_COLOR_DARKEN( view_p->slot_p[slotIndex].color );
                    }
                }
                else
                {
                    // Determine the color
                    if( app_p->date < today )
                    {
                        if( app_p->completed )
                        {
                            // This appointment is in the past
                            view_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                        else
                        {
                            // This appointment is in the past and not marked complete
                            view_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_NOSHOW ];
                        }
                    }
                    else
                    {
                        // MAB:20010315: Decide to color just the confirmed column
                        // if( app_p->confirmedLetterFlag || app_p->confirmedPhoneFlag )
                        // {
                        //      day_p->slot[i].color = Color[ PMC_COLOR_CONFIRMED ];
                        // }
                        // else
                        {
                            view_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_APP ];
                        }

                        if( app_p->completed && app_p->date == today )
                        {
                            // Allow todays appointments to be displayed as completed
                            view_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_COMPLETED ];
                        }
                    }
                    if( app_p->conflictCount )
                    {
                        PMC_COLOR_DARKEN( view_p->slot_p[slotIndex].color );
                    }
                    if( app_p->appCount )
                    {
                        PMC_COLOR_DARKEN( view_p->slot_p[slotIndex].color );
                    }
                }

                // Get strings (for first slot only)
                if( slotCount == 1 )
                {
                    strcpy( view_p->slot_p[slotIndex].refDrName,   app_p->refDrName );
                    sprintf(view_p->slot_p[slotIndex].dur, "%d",  app_p->duration );
                    strcpy( view_p->slot_p[slotIndex].firstName,   app_p->firstName);
                    strcpy( view_p->slot_p[slotIndex].lastName,    app_p->lastName );
                    strcpy( view_p->slot_p[slotIndex].title,       app_p->title);
                    strcpy( view_p->slot_p[slotIndex].homePhone,   app_p->homePhone );
                    strcpy( view_p->slot_p[slotIndex].areaCode,    app_p->areaCode );
                    strcpy( view_p->slot_p[slotIndex].commentIn,   app_p->commentIn );
                    strcpy( view_p->slot_p[slotIndex].type,        pmcAppTypeStrings[app_p->type] );
                    view_p->slot_p[slotIndex].confirmedPhoneState =  app_p->confirmedPhoneState;
                    view_p->slot_p[slotIndex].confirmedLetterState = app_p->confirmedLetterState;
                }
            }
        }
        mbFree( app_p );
    }

    // Record last read time for timer updates
    pmcProviderViewAppLastReadTime  =  pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_APPS );
    pmcProviderViewSlotLastReadTime =  pmcSqlTableUpdateTimeGet( PMC_SQL_TABLE_TIMESLOTS );

    PMC_DEC_USE( ProviderViewInfo.notReady );

exit:

    if( buf_p ) mbFree( buf_p );
    return update;
}


//---------------------------------------------------------------------------
// Function:  TMainForm::WeekViewInfoCellConstruct()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::WeekViewInfoCellConstruct
(
    void
)
{
    Int32u_t        i, j;
    Int32u_t        slotIndex;

    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // For now just blindly copy the colors into the other columns
        for( j = 0 ; j < 7 ; j++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;
            // Propagate color across entire row
            WeekViewCellArray[j][i].str = "";
            WeekViewCellArray[j][i].color = WeekViewInfo.slot_p[slotIndex].color;
            WeekViewCellArray[j][i].last  = WeekViewInfo.slot_p[slotIndex].last;
            WeekViewCellArray[j][i].first = WeekViewInfo.slot_p[slotIndex].first;
            WeekViewCellArray[j][i].str   = WeekViewInfo.slot_p[slotIndex].lastName;
        }
    }
}

//---------------------------------------------------------------------------
// Function:  TMainForm::WeekViewInfoCellConstruct()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::ProviderViewInfoCellConstruct
(
    void
)
{
    Int32u_t        i, j;
    Int32u_t        slotIndex;

    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // For now just blindly copy the colors into the other columns
        for( j = 0 ; j < PMC_PROVIDER_VIEW_COLS ; j++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;
            // Propagate color across entire row
            ProviderViewCellArray[j][i].str = "";
            ProviderViewCellArray[j][i].color = ProviderViewInfo.slot_p[slotIndex].color;
            ProviderViewCellArray[j][i].last  = ProviderViewInfo.slot_p[slotIndex].last;
            ProviderViewCellArray[j][i].first = ProviderViewInfo.slot_p[slotIndex].first;
            ProviderViewCellArray[j][i].str   = ProviderViewInfo.slot_p[slotIndex].lastName;
        }
    }
}


//---------------------------------------------------------------------------
// Function:  TMainForm::AvailTimeListInit()
//---------------------------------------------------------------------------
// Description:
//
// Read available time list from SQL database.
// Put into internal linked list.
// This code is really old and should probably be redone at some time
//---------------------------------------------------------------------------

#if PMC_AVAIL_TIME

void __fastcall TMainForm::AvailTimeListInit
(
    void
)
{
    TDataSet               *dataSet_p;
    pmcAvailTimeStruct_p    availTime_p;
    AnsiString              str = "select * from availtime where not_deleted = 1";
    AnsiString              tempStr;
    Int32u_t                i;
    Variant                 time;

    nbDlgDebug(( "AvailTimeListInit called" ));

    mbLockAcquire( pmcGeneralQueryLock );

    dataSet_p = MainDatabaseModule->GeneralQueryDataSource->DataSet;

    // Ensure SQL command is clear
    MainDatabaseModule->GeneralQuery->SQL->Clear( );
    MainDatabaseModule->GeneralQuery->SQL->Add( str );
    MainDatabaseModule->GeneralQuery->Close( );
    MainDatabaseModule->GeneralQuery->Open( );

    dataSet_p->DisableControls( );

    // Read the actual patient records
    i=0;
    try
    {
        dataSet_p->First( );
        while( !dataSet_p->Eof )
        {
            mbMalloc( availTime_p, sizeof( pmcAvailTimeStruct_t ) );
            qInsertLast( pmcAvailTime_q, availTime_p );

            availTime_p->providerId = dataSet_p->FieldValues[ PMC_SQL_AVAILTIME_FIELD_PROVIDER_ID ];
            availTime_p->dayOfWeek  = dataSet_p->FieldValues[ PMC_SQL_AVAILTIME_FIELD_DAY_OF_WEEK ];
            availTime_p->repeatCode = dataSet_p->FieldValues[ PMC_SQL_AVAILTIME_FIELD_REPEAT_CODE ];

            time = dataSet_p->FieldValues[PMC_SQL_AVAILTIME_FIELD_START_TIME ];
            availTime_p->startTime.Update( &time );

            time = dataSet_p->FieldValues[PMC_SQL_AVAILTIME_FIELD_STOP_TIME ];
            availTime_p->stopTime.Update( &time );

            nbDlgDebug(( "Got startime year %d hour %d min %d string '%s'",
                availTime_p->startTime.Year(),
                availTime_p->startTime.Hour(),
                availTime_p->startTime.Min(),
                availTime_p->startTime.SqlString() ));

             // Setup to do next record
            dataSet_p->Next( );
            i++;
        }
    }
    __finally
    {
        dataSet_p->EnableControls( );
    }

    mbLockRelease( pmcGeneralQueryLock );

    nbDlgDebug(( "Read %ld entries from availtime table", pmcAvailTime_q->size ));

    return;
}

#endif // PMC_AVAIL_TIME

//---------------------------------------------------------------------------
// Function:  TMainForm::DayViewInfoCellConstruct()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::DayViewInfoCellConstruct
(
    void
)
{
    Int32u_t        i, j;

    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // For now just blindly copy the colors into the other columns
        for( j = 0 ; j < PMC_DAY_VIEW_ARRAY_COLS ; j++ )
        {
            // Propagate color across entire row
            DayViewCellArray[j][i].str = "";
            DayViewCellArray[j][i].color = DayViewInfo.slot_p[i].color;
            DayViewCellArray[j][i].last  = DayViewInfo.slot_p[i].last;
            DayViewCellArray[j][i].first = DayViewInfo.slot_p[i].first;

            if( j == PMC_DAY_VIEW_COL_LAST_NAME   ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].lastName;
            if( j == PMC_DAY_VIEW_COL_FIRST_NAME  ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].firstName;
            if( j == PMC_DAY_VIEW_COL_TITLE       ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].title;
            if( j == PMC_DAY_VIEW_COL_AREA_CODE   ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].areaCode;
            if( j == PMC_DAY_VIEW_COL_HOME_PHONE  ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].homePhone;
            if( j == PMC_DAY_VIEW_COL_DURATION    ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].dur;
            if( j == PMC_DAY_VIEW_COL_APP_TYPE    )
            {
                DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].type;
                if( mbStrPos( DayViewCellArray[j][i].str.c_str(), pmcAppTypeStrings[ PMC_APP_TYPE_URGENT ] ) >= 0 )
                {
                    DayViewCellArray[j][i].color = pmcColor[ PMC_COLOR_CONFLICT ];
                }
            }
            if( j == PMC_DAY_VIEW_COL_REF_DR ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].refDrName;

            if( j == PMC_DAY_VIEW_COL_CONF_PHONE )
            {
                if( DayViewInfo.slot_p[i].confirmedPhoneState == PMC_APP_CONFIRMED_RIGHT )
                {
                    DayViewCellArray[j][i].str = "P";
                    DayViewCellArray[j][i].color = pmcColor[ PMC_COLOR_CONFIRMED ];
                }
                else if(  DayViewInfo.slot_p[i].confirmedPhoneState == PMC_APP_CONFIRMED_WRONG )
                {
                    DayViewCellArray[j][i].str = "P";
                    DayViewCellArray[j][i].color = pmcColor[ PMC_COLOR_CONFLICT ];
                }
            }

            if( j == PMC_DAY_VIEW_COL_CONF_LETTER )
            {
                if( DayViewInfo.slot_p[i].confirmedLetterState == PMC_APP_CONFIRMED_RIGHT )
                {
                    DayViewCellArray[j][i].str = "L";
                    DayViewCellArray[j][i].color = pmcColor[ PMC_COLOR_CONFIRMED ];
                }
                else if(  DayViewInfo.slot_p[i].confirmedLetterState == PMC_APP_CONFIRMED_WRONG )
                {
                    DayViewCellArray[j][i].str = "L";
                    DayViewCellArray[j][i].color = pmcColor[ PMC_COLOR_CONFLICT ];
                }
            }

            if( j == PMC_DAY_VIEW_COL_COMMENT ) DayViewCellArray[j][i].str = DayViewInfo.slot_p[i].commentIn;
        }
    }
}


//---------------------------------------------------------------------------
// Function:  TMainForm::MonthViewInfoCellConstruct()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::MonthViewInfoCellConstruct
(
    void
)
{
    Int32u_t        i, j;
    Int32u_t        slotIndex;

    for( i = 0 ; i < PMC_TIMESLOTS_PER_DAY ; i++ )
    {
        // For now just blindly copy the colors into the other columns
        for( j = 0 ; j < 31 ; j++ )
        {
            slotIndex = j * PMC_TIMESLOTS_PER_DAY + i;
            // Propagate color across entire row
            MonthViewCellArray[j][i].str = "";
            MonthViewCellArray[j][i].color = MonthViewInfo.slot_p[slotIndex].color;
            MonthViewCellArray[j][i].last  = MonthViewInfo.slot_p[slotIndex].last;
            MonthViewCellArray[j][i].first = MonthViewInfo.slot_p[slotIndex].first;
            MonthViewCellArray[j][i].str   = MonthViewInfo.slot_p[slotIndex].lastName;
        }
    }
}

//---------------------------------------------------------------------------
// Function:  TMainForm::CullProviders()
//---------------------------------------------------------------------------
// Description:
//
// This function removes all appointments from the list that do not belong
// to the specified provider(s).  The appointments were originally needed
// in the list to look for all manners of appointment conflicts.
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::CullProviders
(
    qHead_p                 app_q,
    pmcAppViewInfo_p        appView_p,
    qHead_p                 providerView_q
)
{
    Int32u_t                providerId = 0;
    Int32u_t                size, i;
    pmcAppInfo_p            app_p;
    bool                    keep;
    bool                    gotLock = FALSE;
    pmcProviderViewList_p   providerView_p;

    switch( appView_p->type )
    {
        case PMC_APP_VIEW_TYPE_DAY:
        case PMC_APP_VIEW_TYPE_WEEK:
        case PMC_APP_VIEW_TYPE_MONTH:
            providerId = appView_p->providerId;
            break;

        case PMC_APP_VIEW_TYPE_PROVIDER:
            mbLockAcquire( providerView_q->lock );
            gotLock = TRUE;
            break;

        default:
            mbDlgDebug(( "Error: invalid appView_p->cols: %ld", appView_p->cols ));
            goto exit;
    }

    size = app_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        app_p = (pmcAppInfo_p)qRemoveFirst( app_q );
        keep = FALSE;

        if( providerId )
        {
            if( app_p->providerId == providerId ) keep = TRUE;
        }
        else
        {
            if( providerView_q == NIL  )
            {
                mbDlgExclaim( "called with providerView_q == NIL" );
                goto exit;
            }

            qWalk( providerView_p, providerView_q, pmcProviderViewList_p )
            {
                if( app_p->providerId == providerView_p->providerId )
                {
                    keep = TRUE;
                    break;
                }
            }
        }

        if( keep )
        {
            qInsertLast( app_q, app_p );
        }
        else
        {
            mbFree( app_p );
        }
    }

exit:
    if( gotLock ) mbLockRelease( providerView_q->lock );
    return TRUE;
}

//---------------------------------------------------------------------------
// Function:  TMainForm::FindConflicts()
//---------------------------------------------------------------------------
// Description:
//
// This function finds all appointment conflicts in the list of
// appointments provided to it.
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::FindConflicts
(
    qHead_p             app_q,
    pmcAppViewInfo_p    appView_p
)
{
    Int32u_t        size;
    Int32u_t        i, j;
    Int32u_t        slotIndex;
    pmcAppInfo_p    app1_p;
    pmcAppInfo_p    app2_p;
    bool            conflict;

    size = app_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        app1_p = (pmcAppInfo_p)qRemoveFirst( app_q );

        qWalk( app2_p, app_q, pmcAppInfo_p )
        {
            conflict = FALSE;

            // Look for conflicts
            if( app2_p->date == app1_p->date )
            {
                // Don't count conflicts for the same provider
                if(     ( app2_p->patientId == app1_p->patientId )
                     && ( app1_p->patientId !=0 )
                     && ( app2_p->providerId != app1_p->providerId ) )

                {
                    if( app2_p->startTime >= app1_p->startTime && app2_p->startTime <  app1_p->stopTime ) conflict = TRUE;
                    if( app1_p->startTime >= app2_p->startTime && app1_p->startTime <  app2_p->stopTime ) conflict = TRUE;
                    if( app2_p->stopTime  >  app1_p->startTime && app2_p->stopTime  <= app1_p->stopTime ) conflict = TRUE;
                    if( app1_p->stopTime  >  app2_p->startTime && app1_p->stopTime  <= app2_p->stopTime ) conflict = TRUE;

                    if( conflict ) app1_p->conflictCount++;
                }
            }
            // Count how many appointments this patient has on this day.
            if( app2_p->date == app1_p->date && app2_p->patientId == app1_p->patientId )
            {
                app1_p->appCount++;
            }
        }
        qInsertLast( app_q, app1_p );
    }

    // This loop checks the appointments against the available timeslots
    size = app_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        app1_p = (pmcAppInfo_p)qRemoveFirst( app_q );

        slotIndex = appView_p->cols * PMC_TIMESLOTS_PER_DAY;
        for( j = 0 ; j < slotIndex ; j++ )
        {
            if( appView_p->slot_p[j].available == FALSE )
            {
                if(    appView_p->slot_p[j].providerId == app1_p->providerId
                    && appView_p->slot_p[j].date == app1_p->date )
                {
                    if(     appView_p->slot_p[j].startTime >= app1_p->startTime
                         && appView_p->slot_p[j].startTime <  app1_p->stopTime )
                    {
                        app1_p->unavail++;
                    }
                }
            }
        }
        qInsertLast( app_q, app1_p );
    }
    return TRUE;

    // MAB:20020720:  The old code below was replaced with the new code
    // above which does not detected conflicts for the same patient with
    // the same provider and which counts the total number of appointments
    // a patient has on a given day.
#if 0
    Int32u_t            size;
    Int32u_t            i, j;
    Int32u_t            slotIndex;
    pmcAppInfo_p    app1_p;
    pmcAppInfo_p    app2_p;
    bool                conflict;

    size = app_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        app1_p = (pmcAppInfo_p)qRemoveFirst( app_q );

        qWalk( app2_p, app_q, pmcAppInfo_p )
        {
            conflict = FALSE;

            // Look for conflicts
            if( app2_p->date == app1_p->date )
            {
                if(    ( ( app2_p->providerId == app1_p->providerId ) )
                    || ( ( app2_p->patientId  == app1_p->patientId  ) && app1_p->patientId !=0 ) )
                {
                    if( app2_p->startTime >= app1_p->startTime && app2_p->startTime <  app1_p->stopTime ) conflict = TRUE;
                    if( app1_p->startTime >= app2_p->startTime && app1_p->startTime <  app2_p->stopTime ) conflict = TRUE;
                    if( app2_p->stopTime  >  app1_p->startTime && app2_p->stopTime  <= app1_p->stopTime ) conflict = TRUE;
                    if( app1_p->stopTime  >  app2_p->startTime && app1_p->stopTime  <= app2_p->stopTime ) conflict = TRUE;

                    if( conflict ) app1_p->conflictCount++;
                }
            }
        }
        qInsertLast( app_q, app1_p );
    }

    // This loop checks the appointments against the available timeslots
    size = app_q->size;

    for( i = 0 ; i < size ; i++ )
    {
        app1_p = (pmcAppInfo_p)qRemoveFirst( app_q );

        slotIndex = appView_p->cols * PMC_TIMESLOTS_PER_DAY;
        for( j = 0 ; j < slotIndex ; j++ )
        {
            if( appView_p->slot_p[j].available == FALSE )
            {
                if(    appView_p->slot_p[j].providerId == app1_p->providerId
                    && appView_p->slot_p[j].date == app1_p->date )
                {
                    if(     appView_p->slot_p[j].startTime >= app1_p->startTime
                         && appView_p->slot_p[j].startTime <  app1_p->stopTime )
                    {
                        app1_p->conflictCount++;
                    }
                }
            }
        }
        qInsertLast( app_q, app1_p );
    }
    return TRUE;
#endif
}

//---------------------------------------------------------------------------
// Function:  TMainForm::AppListGet()
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

void __fastcall TMainForm::AppListGet
(
    qHead_p     app_q,
    Int32u_t    startDate,
    Int32u_t    endDate
)
{
    pmcAppInfo_p            app_p;
    Char_p                  buf_p;
    Char_p                  buf2_p;
    Boolean_t               localFlag;
    MbDateTime              dateTime;
    MbSQL                   sql;

    mbMalloc( buf_p, 4096 );
    mbMalloc( buf2_p, 512 );

    // Format SQL command
    //                          0     1     2     3     4     5     6     7     8     9
    sprintf( buf_p, "select %s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                           "%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,%s.%s,"
                           "%s.%s from "
                           "%s,%s,%s where "
                           "%s.%s=%s.%s and "
                           "%s.%s=%s.%s and "
                           "%s.%s>='%08ld' and "
                           "%s.%s<='%08ld' and "
                           "%s.%s=%ld",

        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_FIRST_NAME,               // 0
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_LAST_NAME,                // 1
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_PATIENTS_FIELD_HOME_PHONE,      // 2
        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_TITLE,                    // 3
        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_LAST_NAME,                // 4
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_ID,                       // 5
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_PATIENT_ID,               // 6
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_START_TIME,          // 7
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_DURATION,            // 8
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_COMMENT_IN,          // 9
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_TYPE,                // 10
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_PHONE_DATE,     // 11
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_PHONE_TIME,     // 12
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_PHONE_ID,       // 13
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_LETTER_DATE,    // 14
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_LETTER_TIME,    // 15
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_CONF_LETTER_ID,      // 16
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_COMPLETED,           // 17
        PMC_SQL_TABLE_APPS,     PMC_SQL_APPS_FIELD_REFERRING_DR_ID,     // 18
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_DATE,                     // 19
        PMC_SQL_TABLE_APPS,     PMC_SQL_FIELD_PROVIDER_ID,              // 20

        PMC_SQL_TABLE_PATIENTS, PMC_SQL_TABLE_DOCTORS, PMC_SQL_TABLE_APPS,

        PMC_SQL_TABLE_PATIENTS, PMC_SQL_FIELD_ID,   PMC_SQL_TABLE_APPS, PMC_SQL_FIELD_PATIENT_ID,
        PMC_SQL_TABLE_DOCTORS,  PMC_SQL_FIELD_ID,   PMC_SQL_TABLE_APPS, PMC_SQL_APPS_FIELD_REFERRING_DR_ID,

        PMC_SQL_TABLE_APPS, PMC_SQL_FIELD_DATE,             startDate,
        PMC_SQL_TABLE_APPS, PMC_SQL_FIELD_DATE,             endDate,
        PMC_SQL_TABLE_APPS, PMC_SQL_FIELD_NOT_DELETED,      PMC_SQL_FIELD_NOT_DELETED_TRUE_VALUE );

    nbDlgDebug(( "SQL: %s len: %ld", buf_p, strlen( buf_p ) ));

    // Execute SQL command
    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        // Get a list of the appointments
        mbMalloc( app_p, sizeof( pmcAppInfo_t ) );
        memset( app_p, 0, sizeof(  pmcAppInfo_t ) );

        qInsertFirst( app_q, app_p );

        app_p->providerId = SelectedProviderId;

        strncpy( app_p->firstName, sql.String( 0 ), PMC_DAY_VIEW_FIRST_NAME_LEN );
        strncpy( app_p->lastName,  sql.String( 1 ), PMC_DAY_VIEW_LAST_NAME_LEN );
        strncpy( buf_p,            sql.String( 2 ), PMC_DAY_VIEW_HOME_PHONE_LEN );
        pmcPhoneFormat( buf_p, app_p->areaCode, app_p->homePhone, &localFlag );
        if( localFlag ) app_p->areaCode[0] = 0;

        strncpy( app_p->title, sql.String( 3 ),  PMC_DAY_VIEW_TITLE_LEN );
        strncpy( app_p->refDrName, sql.String( 4 ), PMC_DAY_VIEW_REF_DR_LEN );

        app_p->id                   = sql.Int32u(  5 );
        app_p->patientId            = sql.Int32u(  6 );
        app_p->startTime            = sql.Int32u(  7 );
        app_p->duration             = sql.Int32u(  8 );
        strncpy( app_p->commentIn,    sql.String(  9 ), PMC_DAY_VIEW_COMMENT_IN_LEN );
        app_p->type                 = sql.Int32u( 10 );
        app_p->confirmedPhoneDate   = sql.Int32u( 11 );
        app_p->confirmedPhoneTime   = sql.Int32u( 12 );
        app_p->confirmedPhoneId     = sql.Int32u( 13 );
        app_p->confirmedLetterDate  = sql.Int32u( 14 );
        app_p->confirmedLetterTime  = sql.Int32u( 15 );
        app_p->confirmedLetterId    = sql.Int32u( 16 );
        app_p->completed            = sql.Int32u( 17 );
        app_p->refDrId              = sql.Int32u( 18 );
        app_p->date                 = sql.Int32u( 19 );
        app_p->providerId           = sql.Int32u( 20 );

        // Determine if the appoinment has been confirmed
        if( app_p->confirmedPhoneDate  && app_p->confirmedPhoneTime  ) app_p->confirmedPhoneState  = PMC_APP_CONFIRMED_WRONG;
        if( app_p->confirmedLetterDate && app_p->confirmedLetterTime ) app_p->confirmedLetterState = PMC_APP_CONFIRMED_WRONG;

        if(    app_p->date       == app_p->confirmedPhoneDate
            && app_p->startTime  == app_p->confirmedPhoneTime
            && app_p->providerId == app_p->confirmedPhoneId )
        {
            app_p->confirmedPhoneState = PMC_APP_CONFIRMED_RIGHT;
        }

        if(    app_p->date       == app_p->confirmedLetterDate
            && app_p->startTime  == app_p->confirmedLetterTime
            && app_p->providerId == app_p->confirmedLetterId )
        {
            app_p->confirmedLetterState = PMC_APP_CONFIRMED_RIGHT;
        }

        // Determine appointment stop time
        dateTime.SetTime( app_p->startTime );
        dateTime.AddMinutes( app_p->duration );
        app_p->stopTime = dateTime.Time();
    }

exit:

    mbFree( buf2_p );
    mbFree( buf_p );
}

//---------------------------------------------------------------------------
// Function:  TMainForm::TimeSlotsGet()
//---------------------------------------------------------------------------
// Description:
//
// Update day view appointment array if required
//---------------------------------------------------------------------------

Int32s_t __fastcall TMainForm::TimeSlotsGet
(
    pmcAppViewInfo_p        appView_p,
    qHead_p                 providerView_q
)
{
    Int32u_t                startDate = 0;
    Int32u_t                endDate  = 0;
    Int32u_t                dateList[64];
    Int32u_t                dayOfWeek[64];
    Char_p                  buf_p = NIL;
    Char_p                  buf2_p = NIL;
    Char_t                  slotString[PMC_TIMESLOTS_PER_DAY+1];
    Int32u_t                date;
    Int32s_t                i, j, slotIndex;
    Boolean_t               found = FALSE;
    Int32u_t                dayOfWeekStart;
    Int32u_t                providerId;
    pmcProviderViewList_p   providerView_p;
    MbSQL                   sql;

    mbMalloc( buf_p, 512 );
    mbMalloc( buf2_p, 256 );

#if PMC_AVAIL_TIME
    // Lock the timeslots table
    pmcSuspendPoll = TRUE;
    while( pmcInPoll ) { Sleep( 100 ); }

    sprintf( buf_p, "lock tables %s write", PMC_SQL_TABLE_TIMESLOTS );
    pmcSqlExec( buf_p );
#endif

    // First, lets figure out start and stop days
    switch( appView_p->type )
    {
        case PMC_APP_VIEW_TYPE_DAY:
            startDate = appView_p->startDate;
            endDate = appView_p->startDate;
            dateList[0] = startDate;
            break;

        case PMC_APP_VIEW_TYPE_WEEK:
            startDate = SelectedWeekViewDate[0];
            endDate =   SelectedWeekViewDate[6];
            for( i = 0 ; i < appView_p->cols ; i++ )
            {
                dateList[i] = SelectedWeekViewDate[i];
            }
            break;

        case PMC_APP_VIEW_TYPE_MONTH:
            startDate = SelectedMonthViewDate[0];
            endDate =   SelectedMonthViewDateEnd;
            mbLog( "Start Date: %lu End Date: %lu\n", SelectedMonthViewDate[0], SelectedMonthViewDateEnd );
            for( i = 0 ; i < appView_p->cols ; i++ )
            {
                dateList[i] = SelectedMonthViewDate[i];
            }
            // Sanity Check
            if( dateList[i-1] != SelectedMonthViewDateEnd ) mbDlgDebug(( "date error" ));
            break;

        case PMC_APP_VIEW_TYPE_PROVIDER:
            startDate = appView_p->startDate;
            endDate = appView_p->startDate;
            break;

        default:
            mbDlgDebug(( "Error: invalid appView_p->cols: %ld", appView_p->cols ));
            goto exit;
    }

    // Get day of week for first day in range
    dayOfWeekStart = mbDayOfWeek( startDate );

    // Compute day of week for each day in range
    for( i = 0 ; i < appView_p->cols ; i++ )
    {
        dayOfWeek[i] = 0;
        if( dateList[i] )
        {
            dayOfWeek[i] = dayOfWeekStart;
        }
        if( ++dayOfWeekStart == 8 ) dayOfWeekStart = 1;
    }

    // Format command to read timeslot info from database
    sprintf( buf_p, "select %s,%s,%s,%s from %s where %s>=%ld and %s<=%ld and (",
        PMC_SQL_FIELD_ID,
        PMC_SQL_FIELD_DATE,
        PMC_SQL_FIELD_PROVIDER_ID,
        PMC_SQL_TIMESLOTS_FIELD_AVAILABLE,
        PMC_SQL_TABLE_TIMESLOTS,
        PMC_SQL_FIELD_DATE, startDate,
        PMC_SQL_FIELD_DATE, endDate );

    if( providerView_q == NIL )
    {
        // Just getting timeslots for one provider
        sprintf( buf2_p, "%s=%ld )", PMC_SQL_FIELD_PROVIDER_ID, appView_p->providerId );
        strcat( buf_p, buf2_p );
    }
    else
    {
        // Getting timeslots for multiple providers

        // Sanity Check
        if( providerView_q->size < 1 || providerView_q->size >= PMC_PROVIDER_VIEW_COLS )
        {
            mbDlgDebug(( "Invalid providerList size: %d\n", providerView_q->size ));
        }

        //  Get appointments for multiple providers
        mbLockAcquire( providerView_q->lock );
        qWalk( providerView_p,  providerView_q, pmcProviderViewList_p )
        {
            sprintf( buf2_p, "%s=%ld or ", PMC_SQL_FIELD_PROVIDER_ID, providerView_p->providerId );
            strcat( buf_p, buf2_p );
        }
        mbLockRelease( providerView_q->lock );
        *(buf_p + strlen( buf_p ) - 3 ) = 0;  // get rid of last "or"
        strcat( buf_p, ")" );
    }

    nbDlgDebug(( buf_p ));

    // Execute SQL command
    if( sql.Query( buf_p ) == FALSE ) goto exit;

    while( sql.RowGet( ) )
    {
        strcpy( slotString, sql.String( 3 ) );
        date = sql.Int32u( 1 );
        providerId = sql.Int32u( 2 );

        // 20040701: Grow the slotString if it is too short
        for( ; ; )
        {
            if( strlen( slotString ) == PMC_TIMESLOTS_PER_DAY ) break;
            strcat( slotString, "0" );
        }

        // Determine which column this is
        if( appView_p->type == PMC_APP_VIEW_TYPE_PROVIDER )
        {
            mbLockAcquire( providerView_q->lock );
            i = 0;
            qWalk( providerView_p, providerView_q, pmcProviderViewList_p )
            {
                if( providerId == providerView_p->providerId )
                {
                    found = TRUE;
                    break;
                }
                i++;
            }
            mbLockRelease( providerView_q->lock );

            // Sanity check
            if( i >= appView_p->cols ) mbDlgDebug(( "Error i:%d\n", i ));

            // Sanity check
            if( found == FALSE ) mbDlgDebug(( "Error: found = FALSE" ));
        }
        else
        {
            // This is not a provider view, use dates to determine columns
            for( found = FALSE, i = 0 ; i < appView_p->cols ; i++ )
            {
                if( date == dateList[i] )
                {
                    found = TRUE;
                    break;
                }
            }
        }

        // At this point, i contains the column
        if( found )
        {
            slotIndex = i * PMC_TIMESLOTS_PER_DAY;
            // Update the available times
            for( j = 0 ; j < PMC_TIMESLOTS_PER_DAY ; j++ )
            {
                // MAB:20020701: Record provider and date for every time
                // slot.  This will simplify detecting conflicts later on.
                appView_p->slot_p[slotIndex].providerId = providerId;
                appView_p->slot_p[slotIndex].date = date;

                if( slotString[j] == PMC_TIMESLOT_AVAILABLE )
                {
                    appView_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_AVAIL ];
                    appView_p->slot_p[slotIndex].countt = 0;
                    appView_p->slot_p[slotIndex].appointId[0] = 0;
                    appView_p->slot_p[slotIndex].available = TRUE;
                }
                slotIndex++;
            }
        }
        else
        {
            // This is an error condition
            mbDlgDebug(( "Error: date %ld not found in date list", date ));
        }
    }

#if PMC_AVAIL_TIME
    pmcAvailTimeStruct_p    availTime_p;
    Int32u_t                startTime;
    Int32u_t                id;

    // Now write entries for the dates that were not found
    for( i = 0 ; i < appView_p->cols ; i++ )
    {
        if( dateFound[i] == FALSE )
        {
            sprintf( buf_p, "select max(%s) from %s where %s>0",
                PMC_SQL_FIELD_ID,
                PMC_SQL_TABLE_TIMESLOTS,
                PMC_SQL_FIELD_ID );

            // Get id from database
            id = pmcSqlSelectInt( buf_p, NIL );

            // Increment id
            id++;

            mbLockAcquire( pmcAvailTime_q->lock );

            for( j = 0 ; j < PMC_TIME_SLOTS_PER_DAY ; j++ ) slotString[j] = PMC_TIMESLOT_UNAVAILABLE;
            slotString[PMC_TIME_SLOTS_PER_DAY] = 0;

            // First loop through available time to see if there is any
//            for( availTime_p = (pmcAvailTimeStruct_p)qInit( pmcAvailTime_q ) ;
//                                                     qDone( pmcAvailTime_q ) ;
//                 availTime_p = (pmcAvailTimeStruct_p)qNext( pmcAvailTime_q ) )
            qWalk( availTime_p, pmcAvailTime_q, pmcAvailTimeStruct_p )
            {
                // Check that available time belongs to selected provider
                if( appView_p->providerId != (Int32u_t)availTime_p->providerId ) continue;

                switch( availTime_p->repeatCode )
                {
                    case PMC_REPEAT_WEEKLY:
                        if( availTime_p->dayOfWeek == (Int16s_t)dayOfWeek[i] )
                        {
                            for( j = 0 ; j < PMC_TIME_SLOTS_PER_DAY ; j++ )
                            {
                                slotIndex = i * PMC_TIME_SLOTS_PER_DAY + j;

                                startTime = appView_p->slot_p[slotIndex].startTime;
                                if( startTime >= availTime_p->startTime.TimeInt &&
                                    startTime <  availTime_p->stopTime.TimeInt  )
                                {
                                    slotString[j] = PMC_TIMESLOT_AVAILABLE;

                                    nbDlgDebug(("Found free time at %d", startTime ));
                                    appView_p->slot_p[slotIndex].color = pmcColor[ PMC_COLOR_AVAIL ];
                                    appView_p->slot_p[slotIndex].count = 0;
                                    appView_p->slot_p[slotIndex].appointId[0] = 0;
                                    appView_p->slot_p[slotIndex].available = TRUE;
                                }
                            }
                        }
                        break;

                    case PMC_REPEAT_DAILY:
                    case PMC_REPEAT_MONTHLY:
                    case PMC_REPEAT_ONCE:
                    case PMC_REPEAT_INVALID:
                        mbDlgDebug(( "Repeat code not currently implemented" ));
                    break;
                }
            }

            mbLockRelease( pmcAvailTime_q->lock );

            // Now create the new record
            sprintf( buf_p, "insert into %s (%s,%s,%s,%s) values (%ld, %ld, %ld, '%s')",
                PMC_SQL_TABLE_TIMESLOTS,
                PMC_SQL_FIELD_ID,
                PMC_SQL_TIMESLOTS_FIELD_DATE,
                PMC_SQL_TIMESLOTS_FIELD_PROVIDER_ID,
                PMC_SQL_TIMESLOTS_FIELD_AVAILABLE,
                id, dateList[i], appView_p->providerId, slotString );

            pmcSqlExec( buf_p );
        }
    }
#endif // PMC_AVAIL_TIME

exit:
#if PMC_AVAIL_TIME
    // Unlock the timeslots table
    sprintf( buf_p, "unlock tables" );
    pmcSqlExec( buf_p );
    pmcSuspendPoll = FALSE;
#endif

    mbFree( buf2_p );
    mbFree( buf_p );
    return TRUE;
}
