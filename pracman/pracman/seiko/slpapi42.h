//
// All files within this DLL are compiled with the SLPAPI42_EXPORTS symbol
// defined on the command line. 
//
// SLPAPI42_EXPORTS should not be defined in any project that uses this DLL.
//
#ifdef SLPAPI42_EXPORTS
#define SLPAPI42_API __declspec(dllexport)
#else
#define SLPAPI42_API __declspec(dllimport)
#endif


#define NORMAL              0
#define BOLD                1
#define ITALIC              2
#define UNDERLINE           4

#define LEFT                0
#define CENTER              1
#define RIGHT               2

#define LANDSCAPE           0
#define PORTRAIT            1


// Label type definitions
#define SIZE_STANDARD       1           // Standard (1.1 x 3.5)
#define SIZE_SHIPPING       2           // Shipping (2.15 x 4.0)
#define SIZE_DISKETTE       3           // Diskette (2.15 x 2.75)
#define SIZE_LARGE          4           // Large Address (1.4 x 3.5)
#define SIZE_MULTIPURPOSE   5           // Multi-Purpose (1.5 x 2.0)
#define SIZE_VHS_SPINE      7           // VHS Spine (0.75 x 5.8)
#define SIZE_VHS_FACE       8           // VHS Face (1.8 x 3.05)
#define SIZE_8MM_SPINE      9           // 8mm Spine (0.4 x 2.85)
#define SIZE_FOLDER         10          // File Folder (0.55 x 3.45)
#define SIZE_35MM           11          // 35mm Slide (0.45 x 1.5)
#define SIZE_BADGE          12          // Name Badge (2.15 x 2.75)
#define SIZE_EURO_N         13          // Euro Folder (narrow) (1.5 x 7.45)
#define SIZE_EURO_W         14          // Euro Folder (wide) (2.15 x 7.45)
#define SIZE_ZIPDISK        15          // Zip Disk (1.95 x 2.3)


#ifdef __cplusplus
extern "C" {
#endif

// Non-text functions
SLPAPI42_API int   PASCAL SlpGetErrorCode(void);
SLPAPI42_API int   PASCAL SlpDeleteFont(HFONT hFont);
SLPAPI42_API int   PASCAL SlpGetLabelHeight(void);
SLPAPI42_API int   PASCAL SlpGetLabelWidth(void);
SLPAPI42_API BOOL  PASCAL SlpDrawRectangle(int x, int y, int width, int height, int thickness);
SLPAPI42_API BOOL  PASCAL SlpDrawLine(int xStart, int yStart, int xEnd, int yEnd, int thickness);
SLPAPI42_API BOOL  PASCAL SlpSetBarCodeStyle(int style, int height);
SLPAPI42_API BOOL  PASCAL SlpSetLabelType(int nID, BOOL fPortrait);
SLPAPI42_API BOOL  PASCAL SlpNewLabel(void);
SLPAPI42_API BOOL  PASCAL SlpPrintLabel(int nCopies);
SLPAPI42_API BOOL  PASCAL SlpDrawBitmap(int x, int y, HBITMAP hBitmap);
SLPAPI42_API VOID  PASCAL SlpSetTextMode(int nMode);

// ANSI text functions
SLPAPI42_API BOOL  PASCAL SlpSetPrinter(int nPrinterType, LPSTR szPort);
SLPAPI42_API BOOL  PASCAL SlpDrawTextXY(int x, int y, HFONT hFont, LPSTR lpText);
SLPAPI42_API HFONT PASCAL SlpCreateFont(LPSTR lpName, int nPoints, int nAttribute);
SLPAPI42_API BOOL  PASCAL SlpPrintText(HFONT hFont, LPSTR lpText, int nFormat);
SLPAPI42_API int   PASCAL SlpGetTextWidth(HFONT hFont, LPSTR lpText);
SLPAPI42_API int   PASCAL SlpGetBarCodeWidth(LPSTR lpText);
SLPAPI42_API BOOL  PASCAL SlpDrawBarCodeXY(int x, int y, LPSTR lpText);


// Unicode text functions
SLPAPI42_API BOOL  PASCAL SlpSetPrinterW(int nPrinterType, LPWSTR wszPort);
SLPAPI42_API BOOL  PASCAL SlpDrawTextXYW(int x, int y, HFONT hFont, LPWSTR wszText);
SLPAPI42_API HFONT PASCAL SlpCreateFontW(LPWSTR wszName, int nPoints, int nAttributes);
SLPAPI42_API BOOL  PASCAL SlpPrintTextW(HFONT hFont, LPWSTR wszText, int nFormat);
SLPAPI42_API int   PASCAL SlpGetTextWidthW(HFONT hFont, LPWSTR wszText);
SLPAPI42_API int   PASCAL SlpGetBarCodeWidthW(LPWSTR wszText);
SLPAPI42_API BOOL  PASCAL SlpDrawBarCodeXYW(int x, int y, LPWSTR wszText);

#ifdef UNICODE
#define SlpSetPrinter           SlpSetPrinterW
#define SlpCreateFont           SlpCreateFontW
#define SlpDrawTextXY           SlpDrawTextXYW
#define SlpGetTextWidth         SlpGetTextWidthW
#define SlpDrawBarCodeXY        SlpDrawBarCodeXYW
#define SlpGetBarCodeWidth      SlpGetBarCodeWidthW
#define SlpPrintText            SlpPrintTextW
#endif

#ifdef __cplusplus
}
#endif
