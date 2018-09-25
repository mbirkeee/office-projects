//---------------------------------------------------------------------------
// File:    mbThermometer.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2001, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    March 26, 2001
//---------------------------------------------------------------------------
// Description:
//
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mbUtils.h"
#include "mbThermometer.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
//#pragma link "CGAUGES"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TThermometer::TThermometer
(
    Char_p      caption_p,
    Int32u_t    minValue,
    Int32u_t    maxValue,
    Boolean_t   showCancelButton
): TForm(Owner)
{
    Int32s_t    screenHeight;
    Int32s_t    screenWidth;
    Int32s_t    controlBorder;
    Int32s_t    YOffset;
    RECT        rect;

    if( caption_p ) Caption = caption_p;
    TheGauge->Visible = TRUE;
    TheGauge->MinValue = minValue;
    TheGauge->MaxValue = maxValue;
    TheGauge->Progress = 0;
    WindowState = wsNormal;
    Ready = FALSE;

    SystemParametersInfo( SPI_GETWORKAREA, 0, (PVOID)&rect, 0 );

    screenWidth  = rect.right;
    screenHeight = rect.bottom;

    HorzScrollBar->Visible = FALSE;
    VertScrollBar->Visible = FALSE;

    // Set some preliminary dimensions
    controlBorder = 15;
    TheGauge->Height = 18;
    Width = 350;
    CancelButton->Width = 80;
    CancelButton->Height = 20;

    Height = 24 + 2 * controlBorder + TheGauge->Height;

    YOffset = ( Height - ClientHeight );

    TheGauge->Width  =   ClientWidth - 2 * controlBorder;
    TheGauge->Left   = ( ClientWidth - TheGauge->Width ) / 2;
    TheGauge->Top = Height - controlBorder - TheGauge->Height - YOffset;

    if( showCancelButton )
    {
        ShowCancelButton = TRUE;
        CancelButton->Enabled = TRUE;
        CancelButton->Visible = TRUE;

        Height +=  CancelButton->Height;
        Height +=  controlBorder;

        CancelButton->Left = ( ClientWidth - CancelButton->Width ) / 2;
        CancelButton->Top = TheGauge->Top + TheGauge->Height + controlBorder;
    }
    else
    {
        ShowCancelButton = FALSE;
        CancelButton->Enabled = FALSE;
        CancelButton->Visible = FALSE;
    }

    // Position the window at the center of the screen
    Top =  ( screenHeight - Height ) / 2;
    Left = ( screenWidth  - Width  ) / 2;

    ButtonDown = FALSE;
    CancelButton->Down = FALSE;

    SQLCursorHandle     = (Void_p)Screen->Cursors[crSQLWait];
    DefaultCursorHandle = (Void_p)Screen->Cursors[crDefault];

    // Set contraints on the window
    Constraints->MinHeight = Height;
    Constraints->MaxHeight = Height;
    Constraints->MinWidth = Width;
    Constraints->MaxWidth = Width;

    if( ShowCancelButton )
    {
        Screen->Cursors[crSQLWait] = Screen->Cursors[crArrow];
        Screen->Cursors[crDefault] = Screen->Cursors[crArrow];

        Cursor = crArrow;
        //Screen->Cursor = crHourGlass;
        Screen->Cursor = crArrow;
    }
    else
    {
        Screen->Cursors[crSQLWait] = Screen->Cursors[crHourGlass];
        Screen->Cursors[crDefault] = Screen->Cursors[crHourGlass];

        Cursor = crHourGlass;
        Screen->Cursor = crHourGlass;
    }
    Ready = TRUE;
}
//---------------------------------------------------------------------------
__fastcall TThermometer::~TThermometer( )
{
    // Restore SQL Wait cursor
    Screen->Cursors[crSQLWait] = (Void_p)SQLCursorHandle;
    Screen->Cursors[crDefault] = (Void_p)DefaultCursorHandle;

    Cursor = crDefault;
    Screen->Cursor = crDefault;

    Ready = FALSE;
}

//---------------------------------------------------------------------------
Int32s_t __fastcall TThermometer::Increment( void )
{
    Int32s_t    returnCode = TRUE;

    if( TheGauge->Progress < TheGauge->MaxValue )
    {
        TheGauge->Progress++;
    }
    else
    {
        TheGauge->Progress = 0;
    }

    if( CheckCancel( ) ) returnCode = FALSE;

    return returnCode;
}
//---------------------------------------------------------------------------
void __fastcall TThermometer::Set( Int32u_t value )
{
    TheGauge->Progress = ( (Int32s_t)value > TheGauge->MaxValue ) ? TheGauge->MaxValue : (Int32s_t)value;
}
//---------------------------------------------------------------------------

Int32s_t  __fastcall TThermometer::CheckCancel( void )
{
    Int32s_t    returnCode = FALSE;
    MSG         msg;
    Boolean_t   processed;
    TRect       button;
    TPoint      buttonOrigin;
    Int32s_t    startMsg = 0;
    Int32s_t    endMsg = 0;

    for( ; ; )
    {
        if( !PeekMessage( &msg, NULL, startMsg, endMsg, PM_REMOVE ) ) break;
        if( msg.hwnd != Handle && IsChild( Handle, msg.hwnd ) == 0 )
        {
            if( msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN ) continue;
        }

        processed = FALSE;

        if( ShowCancelButton && ( msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN ) )
        {
            // dsrDebug( "PeekMessage returned message %d\n", msg.message );
            button = CancelButton->ClientRect;
            buttonOrigin = CancelButton->ClientOrigin;

            button.Top      += buttonOrigin.y;
            button.Bottom   += buttonOrigin.y;
            button.Left     += buttonOrigin.x;
            button.Right    += buttonOrigin.x;

            // For some reason on win9x, the message point seems to be in the wrong spot
            // Read the current cursor position
            if( mbWinVersion( ) == MB_WINDOWS_9X )
            {
                POINT pt;
                GetCursorPos( &pt );
                msg.pt.x = pt.x;
                msg.pt.y = pt.y;
            }
            if(    msg.pt.x > button.Left && msg.pt.x < button.Right
                && msg.pt.y > button.Top  && msg.pt.y < button.Bottom )
            {
                if( msg.message == WM_LBUTTONDOWN )
                {
                    // processed = TRUE;
                    CancelButton->Down = TRUE;
                    ButtonDown = TRUE;
                    CancelButton->Invalidate();
                }
                else if( msg.message == WM_LBUTTONUP )
                {
                    // processed = TRUE;
                    CancelButton->Down = FALSE;
                    CancelButton->Invalidate( );
                    if( ButtonDown == TRUE )
                    {
                        returnCode = TRUE;
                        goto exit;
                    }
                    ButtonDown = FALSE;
                }
            }
            else
            {
                // pmcLog( "message not on button\n" );
                if( ButtonDown == TRUE )
                {
                    // processed = TRUE;
                    CancelButton->Down = FALSE;
                    CancelButton->Invalidate( );
                    ButtonDown = FALSE;
                }
            } // end - if( on button )
        } // end - if( BUTTONUP || BUTTONDOWN )

        if( !processed )
        {
            TranslateMessage( (LPMSG)&msg );
            DispatchMessage(  (LPMSG)&msg );
        }
    }
exit:
    return returnCode;
 }
//---------------------------------------------------------------------------
void __fastcall TThermometer::FormDeactivate(TObject *Sender)
{
    if( Ready && Visible ) SetFocus();
}
//---------------------------------------------------------------------------


