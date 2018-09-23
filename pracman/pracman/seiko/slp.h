
#ifndef _SLP_H
#define _SLP_H

typedef enum
    {
    SLP_ORIG    = 0x0001,       // Original SLP
    SLP_PLUS    = 0x0002,       // SLP Plus
    SLP_EZ30    = 0x0004,       // SLP EZ30
    SLP_PRO     = 0x0008,       // SLP Pro
    SLP_120     = 0x0010,       // SLP 120
    SLP_220     = 0x0020,       // SLP 220
    SLP_100     = 0x0040,       // SLP 100
    SLP_200     = 0x0080,       // SLP 200
    SLP_140     = 0x0100,       // SLP 140
    SLP_240     = 0x0200        // SLP 240
    } SLP_MODEL;


#define PointsToProPixels(pts)  MulDiv(pts, 203, 72)
#define ProPixelsToPoints(pix)  MulDiv(pix, 72, 203)

#endif // _SLP_H
