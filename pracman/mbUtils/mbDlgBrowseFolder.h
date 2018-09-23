//---------------------------------------------------------------------------
// Function:    mbDlgBrowseFolder.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2005, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        January 5, 2005
//---------------------------------------------------------------------------
// Description:
//
// Browse Folder Utilities
//---------------------------------------------------------------------------

#ifndef mbDlgBrowseFolder_h
#define mbDlgBrowseFolder_h

#ifdef  External
#undef  External
#endif

//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

Ints_t mbDlgBrowseFolderNew( Char_p buf_p, Char_p title_p, Void_p *startHandle_pp );
Void_p mbPathHandleGet( Char_p path_p );
Void_t mbPathHandleFree( Void_p handle_p );

#endif // mbDlgBrowseFolder_h

