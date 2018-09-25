//---------------------------------------------------------------------------
// Function:    mbUtils.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Feb. 9 2002
//---------------------------------------------------------------------------
// Description:
//
// Include file for the mbUtil library
//---------------------------------------------------------------------------

#ifndef mbUtils_h
#define mbUtils_h

#include <stdio.h>
#include <time.h>
#include <dir.h>
#include <vcl.h>

#include "mbTypes.h"
#include "mbStrings.h"
#include "mbDebug.h"
#include "mbLock.h"
#include "mbQueue.h"
#include "mbMalloc.h"
#include "mbDlg.h"
#include "mbLog.h"
#include "mbDateTimeUtils.h"
#include "mbDateTime.h"
#include "mbCrc.h"
#include "mbFileList.h"
#include "mbStrUtils.h"
#include "mbFileUtils.h"
#include "mbThermometer.h"
#include "mbProperty.h"
#include "mbPopup.h"
#include "mbDlgBrowseFolder.h"
                    
//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

// Windows Version Information  GetVersion()
// ===========================
// Platform                    High-order bit  Next 7 bits     Low-order byte
// Windows NT 3.51             0               Build number    3
// Windows NT 4.0              0               Build number    4
// Windows 2000 or Windows XP  0               Build number    5
// Windows 95, 98, or MB       1               Reserved        4
// Win32s with Windows 3.1     1               Build number    3

#define MB_WINDOWS_MASK        0x800000FF
#define MB_WINDOWS_NT_351      0x00000003
#define MB_WINDOWS_NT_40       0x00000004
#define MB_WINDOWS_2000XP      0x00000005
#define MB_WINDOWS_9X          0x80000004
#define MB_WINDOWS_31          0x80000003

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

#define mbWinVersion( ) GetVersion( ) & MB_WINDOWS_MASK

#endif // mbUtils_h

