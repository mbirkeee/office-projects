//---------------------------------------------------------------------------
// File:    mbDlgBrowseFolder.cpp
//---------------------------------------------------------------------------
// Author:  Michael A. Bree (c) 2005, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:    January 2, 2005
//---------------------------------------------------------------------------
// Description:
//
// Browse for folder dialog
//---------------------------------------------------------------------------

#define NO_WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <shobjidl.h>

#include <stdio.h>
#include <vcl.h>
#pragma hdrstop

#include "mbTypes.h"
#include "mbMalloc.h"
#include "mbDlg.h"
#include "mbDlgBrowseFolder.h"

//-----------------------------------------------------------------------------
// Procedure: mbDlgBrowseFolderCallback( )
//-----------------------------------------------------------------------------
// Description:
//
// Sends BFFM_SETSELECTION to set the initial folder in the browse dialog.
//-----------------------------------------------------------------------------
int __stdcall mbDlgBrowseFolderCallback
(
    HWND        hwnd,
    UINT        uMsg,
    LPARAM      lParam,
    LPARAM      lpData
)
{
    //mbDlgInfo( "callback function called, msg: %u (%u) lParam: % lpData %p",
    //    uMsg, BFFM_INITIALIZED, lParam, lpData );

    if( lpData != 0 && uMsg == BFFM_INITIALIZED	)
    {
        // mbDlgInfo( "callback function called, msg: %u\ lpData: %p", uMsg, lpData );
        PostMessage( hwnd, BFFM_SETSELECTION, FALSE, lpData );
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Procedure: mbPathHandleFree( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Void_t mbPathHandleFree( Void_p handle_p )
{
    LPMALLOC        shellMalloc_p;

    if( handle_p == NIL ) goto exit;

    // Get pointer to Shell's IMALLOC interface
    if( !SUCCEEDED( SHGetMalloc( &shellMalloc_p ) ) ) goto exit;

    shellMalloc_p->Free( handle_p );
exit:
    return;
}

//-----------------------------------------------------------------------------
// Procedure: mbPathHandleGet( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Void_p mbPathHandleGet( Char_p path_p )
{
    LPITEMIDLIST    pidl = NIL;
    AnsiString      name;
    int             size;
    wchar_t        *dest_p = NIL;
    IShellFolder   *shellFolder;

    name = path_p;
    size = name.WideCharBufSize();
    if( ( dest_p = (wchar_t *)malloc( size * 2 ) ) == NIL ) goto exit;

    //mbDlgInfo( "dest_p: %p", dest_p );
    name.WideChar( dest_p, size );

    SHGetDesktopFolder(&shellFolder);
    if( shellFolder->ParseDisplayName( NULL, NULL, dest_p, NULL, &pidl, NULL ) != NOERROR )
    {
        pidl = NIL;
    }
    shellFolder->Release();

exit:
    if( dest_p )
    {
        //mbDlgInfo( "dest_p: %p", dest_p );
        free( dest_p );
    }
    return (Void_p)pidl;
}

//-----------------------------------------------------------------------------
// Procedure: mbDlgBrowseFolderNew( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Ints_t mbDlgBrowseFolderNew( Char_p return_p, Char_p title_p, Void_p *startHandle_pp )
{
    BROWSEINFO      bi;
    LPSTR           buf_p = NIL;
    LPITEMIDLIST    pidlBrowse_p;
    LPITEMIDLIST    pidlLast_p = NIL;
    LPMALLOC        shellMalloc_p;
    Ints_t          returnCode = MB_BUTTON_CANCEL;
    Int32s_t        size;

    if( startHandle_pp ) pidlLast_p = (LPITEMIDLIST)*startHandle_pp;

    // Get pointer to Shell's IMALLOC interface
    if( !SUCCEEDED( SHGetMalloc( &shellMalloc_p ) ) ) goto exit;

    // Allocate a buffer to receive browse information.
    mbMalloc( buf_p, MAX_PATH );

    if( buf_p == NIL ) goto exit;

    // Fill in the BROWSEINFO structure.
    bi.hwndOwner = 0;

    // Folder in whick to start, 0 is desktop
    bi.pidlRoot = 0;

    // Pointer to buffer that will contain the result.  We will ignore this
    bi.pszDisplayName = buf_p;

    // Set the title
    bi.lpszTitle = ( title_p == NIL ) ? "" : title_p;

    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    if( pidlLast_p != NIL )
    {
        // mbDlgInfo( "set callback, pidl: %p", pidlLast_p );
        bi.lpfn = mbDlgBrowseFolderCallback;
        bi.lParam = (Int32u_t)pidlLast_p;
    }
    else
    {
        bi.lpfn = NIL;
    }

    // Browse for a folder and return its PIDL.
    if( ( pidlBrowse_p = SHBrowseForFolder( &bi ) ) == NIL ) goto exit;

    // mbDlgInfo( "SHBrowseForFolder succeeded: buf_p: '%s'\n", buf_p );

    // Get path from pidl
    if( SHGetPathFromIDList( pidlBrowse_p, buf_p ) == FALSE )
    {
        mbDlgInfo( "SHGetPathFromIDList() failed\n" );
        goto exit;
    }

    returnCode = MB_BUTTON_OK;
    if( return_p ) strcpy( return_p, buf_p );

    // Return handle to this directory
    if( startHandle_pp )
    {
        shellMalloc_p->Free( pidlLast_p );
        size = shellMalloc_p->GetSize( pidlBrowse_p );
        if( ( pidlLast_p = (LPITEMIDLIST)shellMalloc_p->Alloc( size ) ) == NIL ) goto exit;
        CopyMemory( pidlLast_p, pidlBrowse_p, size );
        // mbDlgInfo( "stored handle to directory\n" );
        *startHandle_pp = (Void_p)pidlLast_p;
    }

 exit:

    // Must use shell's IMALLOC interface to free memory allocated by shell
    if( shellMalloc_p != NIL )
    {
        if( pidlBrowse_p )
        {
            shellMalloc_p->Free( pidlBrowse_p );
        }
    }

    if( buf_p ) mbFree( buf_p );

    return returnCode;
}





