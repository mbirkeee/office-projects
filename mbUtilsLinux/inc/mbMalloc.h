/******************************************************************************
 * Copyright (c) 2007, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/mbUtilsLinux/inc/mbMalloc.h,v 1.1.1.1 2007/07/10 05:06:02 mikeb Exp $
 *-----------------------------------------------------------------------------
 * $Log: mbMalloc.h,v $
 * Revision 1.1.1.1  2007/07/10 05:06:02  mikeb
 * Initial Import
 *
 *-----------------------------------------------------------------------------
 */

#ifndef MB_MALLOC_H
#define MB_MALLOC_H

#include <stdlib.h>

#if 0

#define mbMalloc( _size_ ) \
    malloc( _size_ )

#define mbCalloc( _size_ ) \
    calloc( 1, _size_ )

#define mbFree( _ptr_ )                     \
{                                           \
    if( (_ptr_) != NIL ) free( (_ptr_) );   \
}
#endif

Void_p      mbMallocStr( Char_p str_p );
Int32u_t    mbMallocCount( Void_t );
Void_t      mbFree( Void_p ptr_p );
Void_p      mbMalloc( int size );
Void_p      mbCalloc( int size );

#endif /* MB_MALLOC_H */
