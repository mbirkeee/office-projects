
#pragma once

#include <windows.h>

#define NORMAL          	0
#define BOLD            	1
#define ITALIC          	2
#define UNDERLINE       	4

#define LEFT				0
#define CENTER				1
#define RIGHT				2

#define LANDSCAPE			0
#define PORTRAIT			1

// Label type definitions
#define SIZE_STANDARD   	1			// Standard (1.1 x 3.5)
#define SIZE_SHIPPING   	2			// Shipping (2.15 x 4.0)
#define SIZE_DISKETTE   	3			// Diskette (2.15 x 2.75)
#define SIZE_LARGE      	4			// Large Address (1.4 x 3.5)
#define SIZE_MULTIPURPOSE	5			// Multi-Purpose (1.5 x 2.0)
#define SIZE_VHS_SPINE		7			// VHS Spine (0.75 x 5.8)
#define SIZE_VHS_FACE		8			// VHS Face (1.8 x 3.05)
#define SIZE_8MM_SPINE		9			// 8mm Spine (0.4 x 2.85)
#define SIZE_FOLDER			10			// File Folder (0.55 x 3.45)
#define SIZE_35MM			11			// 35mm Slide (0.45 x 1.5)
#define SIZE_BADGE			12			// Name Badge (2.15 x 2.75)

// Error codes
#define API_NO_DC                               -1
#define API_GENERIC_ERROR                       -2
#define API_BAD_LABEL_TYPE                      -3
#define API_BAD_FONT_HANDLE                     -4
#define API_BAD_THICKNESS                       -5
#define API_BAD_BAR_CODE_STYLE                  -6
#define API_BAD_PRINTER_TYPE                    -7
#define API_INVALID_PORT                        -8
#define API_INVALID_LABEL_TYPE                  -9
#define API_INVALID_BAR_CODE_HANDLE             -10
#define API_INVALID_BITMAP_HANDLE               -11
#define API_NO_BAR_CODE_LIB                     -12
#define API_INVALID_LIBRARY_FUNCTION			-13
#define API_PRINTER_OPEN_ERROR                  -14
#define API_INVALID_IMAGE_FILE                  -15
#define API_IMAGE_ERROR                         -16

#define SLP_BC_CODE39       1      
#define SLP_BC_CODE2OF5     2
#define SLP_BC_CODABAR      3
#define SLP_BC_CODE128      4
#define SLP_BC_UPC          5
#define SLP_BC_UPCE         6
#define SLP_BC_EAN13        7
#define SLP_BC_POSTNET      8
#define SLP_BC_RM4SCC       9
#define SLP_BC_MAXICODE     20
#define SLP_BC_PDF417       21
#define SLP_BC_DATAMATRIX   22


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SLPAPI62_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SLPAPI62_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SLPAPI62_EXPORTS
#define SLPAPI62_API __declspec(dllexport)
#else
#define SLPAPI62_API __declspec(dllimport)
#endif

// Exported function prototypes
#ifdef __cplusplus
extern "C"
{
#endif

SLPAPI62_API int    __stdcall SlpGetErrorCode(void);
SLPAPI62_API int    __stdcall SlpDeleteFont(HFONT hFont);
SLPAPI62_API int    __stdcall SlpGetLabelHeight(void);
SLPAPI62_API int    __stdcall SlpGetLabelWidth(void);
SLPAPI62_API BOOL   __stdcall SlpDrawRectangle(int x, int y, int width, int height, int thickness);
SLPAPI62_API BOOL   __stdcall SlpDrawLine(int xStart, int yStart, int xEnd, int yEnd, int thickness);
SLPAPI62_API BOOL	__stdcall SlpStartLabel();
SLPAPI62_API BOOL	__stdcall SlpEndLabel();
SLPAPI62_API void   __stdcall SlpClosePrinter(void);
SLPAPI62_API void   __stdcall SlpCopyLabelToClipboard(void);
SLPAPI62_API void   __stdcall SlpDebugMode(BOOL state);

// ANSI functions
SLPAPI62_API BOOL	__stdcall SlpOpenPrinter(LPSTR szPrinterName, int nID, BOOL fPortrait);
SLPAPI62_API HFONT  __stdcall SlpCreateFont(LPSTR lpName, int nPoints, int nAttribute);
SLPAPI62_API BOOL   __stdcall SlpDrawTextXY(int x, int y, HFONT hFont, LPSTR lpText);
SLPAPI62_API int    __stdcall SlpGetTextWidth(HFONT hFont, LPSTR lpText);
SLPAPI62_API int    __stdcall SlpGetTextHeight(HFONT hFont, LPSTR lpText);
SLPAPI62_API BOOL   __stdcall SlpDrawBarCode(int nLeft, int nTop, int nRight, int nBottom,
                                             LPSTR lpText);
SLPAPI62_API BOOL   __stdcall SlpDrawPicture(int nLeft, int nTop, int nRight, int nBottom,
                                             LPSTR szPath);
SLPAPI62_API BOOL   __stdcall SlpSetBarCodeStyle(int nSymbology, int nRatio, int nMode, 
                                                 int nSecurity, BOOL bReadableText,
                                                 int nFontHeight, int nFontAttributes, 
                                                 LPSTR szFaceName);
SLPAPI62_API void   __stdcall SlpGetVersion(LPSTR szVersion);

// Unicode functions
SLPAPI62_API BOOL	__stdcall SlpOpenPrinterW(LPWSTR szPrinterName, int nID, BOOL fPortrait);
SLPAPI62_API HFONT  __stdcall SlpCreateFontW(LPWSTR wszName, int nPoints, int nAttributes);
SLPAPI62_API BOOL	__stdcall SlpDrawTextXYW(int x, int y, HFONT hFont, LPWSTR wszText);
SLPAPI62_API int	__stdcall SlpGetTextWidthW(HFONT hFont, LPWSTR wszText);
SLPAPI62_API int	__stdcall SlpGetTextHeightW(HFONT hFont, LPWSTR wszText);
SLPAPI62_API BOOL   __stdcall SlpDrawBarCodeW(int nLeft, int nTop, int nRight, int nBottom,
                                              LPWSTR wszText);
SLPAPI62_API BOOL   __stdcall SlpDrawPictureW(int nLeft, int nTop, int nRight, int nBottom,
                                              LPWSTR wszPath);
SLPAPI62_API BOOL   __stdcall SlpSetBarCodeStyleW(int nSymbology, int nRatio, int nMode, 
                                                  int nSecurity, BOOL bReadableText,
                                                  int nFontHeight, int nFontAttributes, 
                                                  LPWSTR wszFaceName);
SLPAPI62_API void   __stdcall SlpGetVersionW(LPWSTR wszVersion);

#ifdef __cplusplus
}
#endif

#ifndef SLPAPI62_EXPORTS
#ifdef UNICODE

#define SlpOpenPrinter      SlpOpenPrinterW
#define SlpCreateFont       SlpCreateFontW
#define SlpDrawTextXY       SlpDrawTextXYW
#define SlpGetTextWidth     SlpGetTextWidthW
#define SlpDrawBarCode      SlpDrawBarCodeW
#define SlpDrawPicture      SlpDrawPictureW
#define SlpSetBarCodeStyle  SlpSetBarCodeStyleW
#define SlpGetVersion       SlpGetVersionW

#endif
#endif

