//---------------------------------------------------------------------------
// Function:    mbTypes.h
//---------------------------------------------------------------------------
// Author:      Michael A. Bree (c) 2002, Saskatoon SK Canada
//---------------------------------------------------------------------------
// Date:        Oct. 30 2002
//---------------------------------------------------------------------------
// Description:
//
// File functions and macros
//---------------------------------------------------------------------------

#ifndef mbTypes_h
#define mbTypes_h

#include "mbRetCodes.h"

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------

#ifndef NIL
#   define NIL  ( (void *)0 )
#endif /* NIL */

/* Conflicts with vxWorks  */
#ifndef ERROR
#   define ERROR    ( 0 )
#endif

#ifndef OK
#   define OK       ( 1 )
#endif

#undef  FALSE
#define FALSE       ( 0 )

#undef  TRUE
#define TRUE        ( 1 )

/*********************************************************************
 * General Types
 *--------------------------------------------------------------------
 */

#define External            extern
#define Static              static
#define Register            register
#define Enumerated          enum

typedef void                Void;
typedef void                Void_t;
typedef void               *Void_p;

typedef int                 Ints;
typedef int                 Ints_t;
typedef int                *Ints_p;

typedef unsigned int        Intu;
typedef unsigned int        Intu_t;
typedef unsigned int       *Intu_p;

typedef unsigned char       Int8u;
typedef unsigned char       Int8u_t;
typedef unsigned char      *Int8u_p;

typedef signed char         Int8s;
typedef signed char         Int8s_t;
typedef signed char        *Int8s_p;

typedef char                Char;
typedef char                Char_t;
typedef char               *Char_p;

typedef unsigned short      Int16u;
typedef unsigned short      Int16u_t;
typedef unsigned short     *Int16u_p;

typedef signed short        Int16s;
typedef signed short        Int16s_t;
typedef signed short       *Int16s_p;

typedef unsigned long       Int32u;
typedef unsigned long       Int32u_t;
typedef unsigned long      *Int32u_p;

typedef signed long         Int32s;
typedef signed long         Int32s_t;
typedef signed long        *Int32s_p;

typedef bool                Boolean_t;
typedef bool               *Boolean_p;

typedef unsigned __int64    Int64u;
typedef unsigned __int64    Int64u_t;
typedef unsigned __int64   *Int64u_p;

typedef __int64             Int64s;
typedef __int64             Int64s_t;
typedef __int64            *Int64s_p;

typedef float               Float_t;
typedef float              *Float_p;

typedef float               Float32_t;
typedef float              *Float32_p;

typedef double              Double_t;
typedef double             *Double_p;

typedef double              Float64_t;
typedef double             *Float64_p;

//#define PMC_NAME "Practice Manager"

#endif /* mbTypes_h */

