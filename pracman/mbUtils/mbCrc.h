//---------------------------------------------------------------------------
// Function:    mbCrc.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// Crc utilities
//---------------------------------------------------------------------------

#ifndef mbCrc_h
#define mbCrc_h

#ifdef  External
#undef  External
#endif

#ifdef  MB_INIT_GLOBALS
#define External
#else
#define External extern
#endif

// CRC Polynomial
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
#define MB_CRC_POLYNOMIAL 0x04C11DB7L

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

External Int32u_t       mbCrcTable[256];


//---------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------

void            mbCrcInit
(
    void
);

Int32u_t        mbCrcFile
(
    Char_p      fileName_p,
    Int32u_p    size_p,
    Void_p      this_p,
    Int32s_t    (*cancelFunc)( Void_p this_p ),
    Int32s_t    (*bytesFunc)( Void_p this_p, Int32u_t bytes_p ),
    Int32u_p    cancelFlag_p,
    Char_p      cancelString_p
);

Int32u_t        mbCrcUpdate
(
    Int32u_t    crc,
    Int8u_p     data_p,
    Int32u_t    dataLen
);

#endif // mbCrc_h

