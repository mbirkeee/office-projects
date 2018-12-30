/******************************************************************************
 * File: filecomp.c
 *-----------------------------------------------------------------------------
 * Copyright (c) 2003, Michael A. Bree
 *-----------------------------------------------------------------------------
 * Date: Sept. 13, 2003
 *-----------------------------------------------------------------------------
 * Description:
 *
 *-----------------------------------------------------------------------------
 * $Header: /home/cvs/filecomp/src/filecomp.c,v 1.2 2006/12/05 13:47:36 mikeb Exp $
 *-----------------------------------------------------------------------------
 */

/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>

/* Project include files */
#include "mabGeneral.h"
#include "mabQueue.h"
#include "filecompMain.h"

#define INIT_GLOBALS
#include "filecompGlob.h"
#undef  INIT_GLOBALS

#define MB_SEG_FAULT()\
{\
    int *temp = 0;\
    int _i = (int)*temp;\
    int _x=_i;\
    _i=_x;\
}

extern char *optarg;

void mb_sig( int sig )
{
    printf( "mb_sig called, sig: %d\n", sig );
    exit( 0 );
    return;
}

/******************************************************************************
 * Procedure: main( )
 *-----------------------------------------------------------------------------
 * Description:
 *-----------------------------------------------------------------------------
 */

int main
(
    int             argc,
    Char_p          argv[]
)
{
    Int16s_t        option;
    qHead_t         dirQueueHead;
    qHead_p         dir_q;
    qHead_t         fileQueueHead;
    qHead_p         file_q;
    qHead_t         fileDoneQueueHead;
    qHead_p         fileDone_q;
    qHead_t         fileUniqueQueueHead;
    qHead_p         fileUnique_q;
    dsFileStruct_p  file_p;
    dsFileStruct_p  file2_p;
    Int32u_t        fileCount = 0;
    Int32u_t        matchCount;
    Int32u_t        totalMatchCount = 0;
    Int32u_t        totalFilesExamined = 0;
    Int32u_t        matchSets = 0;
    Boolean_t       displayed;
    Boolean_t       showUnique = FALSE;
    Boolean_t       helpFlag = FALSE;
    Boolean_t       recurseFlag = TRUE;
    Char_t          curDir[256];

#undef  MN
#define MN "main"


    printf( "\n" );

    dir_q = qInitialize( &dirQueueHead );
    file_q = qInitialize( &fileQueueHead );
    fileDone_q = qInitialize( &fileDoneQueueHead );
    fileUnique_q = qInitialize( &fileUniqueQueueHead );

    /* Initialize global variables */
    mbCrcInit( );
    SkipSymlinks = FALSE;
    TempFileCount = 0;
    TotalFileCount = 0;

    getcwd( curDir, 256 );

    signal(SIGSEGV, mb_sig );
    signal(SIGFPE, mb_sig );

    //MB_SEG_FAULT();
    //printf( "After seg fault\n" );

    while( ( option = getopt( argc, argv, "-d:hv?suf" )) != -1 )
    {
        //printf( "optarg: %s\n", optarg );
        switch( option )
        {
          case 1:
          case 'd':
            file_p = fileAdd( NIL, optarg );
            qInsertLast( dir_q, file_p );
            break;

          case 'f':
            recurseFlag = FALSE;
            break;

          case 's':
            SkipSymlinks = TRUE;
            break;

          case 'u':
            showUnique = TRUE;
            break;

          case '?':
          case 'h':
          case 'v':
            helpFlag = TRUE;
            goto exit;

          default:
            printf( "%s: %c: unknown option.\n", DS_PROG_NAME, option );
            helpFlag = TRUE;
            goto exit;
        }

    }


    if( dir_q->size == 0 )
    {
        file_p = fileAdd( NIL, curDir );
        qInsertLast( dir_q, file_p );
    }

    /* Loop through the specified directories, getting the list of files in each */
    qWalk( file_p, dir_q, dsFileStruct_p )
    {
        printf( "Examining directory '%s'...\n", file_p->path_p );
        chdir( curDir );
        if( chdir( file_p->path_p ) != 0 )
        {
            printf( "Could not change to directory '%s'.\n", file_p->path_p );
            goto exit;
        }
        chdir( curDir );
        getFileList( file_p->path_p, file_q, recurseFlag );
    }

    /* Count the detected files */
    chdir( curDir );
    fileCount = 0;

    qWalk( file_p, file_q, dsFileStruct_p )
    {
        if( file_p->type == DS_TYPE_FILE )
        {
            fileCount++;
        }
    }
    printf( "\nComparing %ld files...\n", fileCount );

    while( !qEmpty( file_q ) )
    {
        /* Get a file from the queue */
        file_p = (dsFileStruct_p)qRemoveFirst( file_q );

        /* Ignore file if its of type directory */
        if( file_p->type == DS_TYPE_DIR )
        {
            qInsertLast( fileDone_q, file_p );
            continue;
        }

        totalFilesExamined++;

        /* Compare files to all others remaining in the queue */
        displayed = FALSE;
        matchCount = 0;
        qWalk( file2_p, file_q, dsFileStruct_p )
        {
            /* Ignore directories */
            if( file2_p->type == DS_TYPE_DIR ) continue;

            /* Compare sizes... */
            if( file_p->size == file2_p->size )
            {
                /* Since sizes are equal, compare CRCs */
                mbCrcFile( file_p );
                mbCrcFile( file2_p );
                if( file_p->crc == file2_p->crc )
                {
                    /* Files are duplicates */
                    matchCount++;
                    totalMatchCount++;

                    if( !showUnique )
                    {
                        if( !displayed )
                        {
                            printf( "\nDuplicate files: size: %lu CRC: %08lX\n", (Int32u_t)file_p->size, file_p->crc );
                            printf( "   %s\n", file_p->pathName_p );
                            displayed = TRUE;
                        }
                        printf( "   %s\n", file2_p->pathName_p );

                    }
                    /* Set the matched file type to a DIR so that it gets   */
                    /* ignored in subsequent searches... its already been   */
                    /* listed as a duplicate file                           */
                    file2_p->type = DS_TYPE_DIR;

                }
            }
        }

        if( matchCount )
        {
            /* Are finished with this file */
            qInsertLast( fileDone_q, file_p );
        }
        else
        {
            /* Are finisied with this file */
            qInsertLast( fileUnique_q, file_p );
        }

    if( matchCount > 0 )
    {
       matchSets++;
    }
    }

    if( showUnique )
    {
        if( fileUnique_q->size )
        {
            printf( "\nThe following unique files were detected:\n" );
            qWalk( file_p, fileUnique_q, dsFileStruct_p )
            {
                printf( "  %s\n", file_p->pathName_p );
            }
        }
        else
        {
            printf( "\nNo unique files detected.\n" );
        }
    }
    else
    {
        if( totalMatchCount == 0 )
        {
            printf( "\nNo duplicate files were detected.\n" );
        }
    }

    printf( "\n" );
    printf( "Total files examined:       %lu\n", totalFilesExamined );
    printf( "Total duplicate file sets:  %lu\n", matchSets );
    printf( "Total unique files:         %lu\n", fileUnique_q->size );
    printf( "\n" );

exit:

    chdir( curDir );

    for( ; ; )
    {
        if( qEmpty( dir_q ) ) break;

        file_p = (dsFileStruct_p)qRemoveFirst( dir_q );
        //printf( "removing dir '%s'\n", file_p->pathName );
        fileFree( file_p );
    }

    for( ; ; )
    {
        if( qEmpty( file_q ) ) break;

        file_p = (dsFileStruct_p)qRemoveFirst( file_q );
        //printf( "removing file '%s'\n", file_p->pathName );
        fileFree( file_p );
    }

    for( ; ; )
    {
        if( qEmpty( fileDone_q ) ) break;

        file_p = (dsFileStruct_p)qRemoveFirst( fileDone_q );
        //printf( "removing file '%s'\n", file_p->pathName );
        fileFree( file_p );
    }

    for( ; ; )
    {
        if( qEmpty( fileUnique_q ) ) break;

        file_p = (dsFileStruct_p)qRemoveFirst( fileUnique_q );
        //printf( "removing file '%s'\n", file_p->pathName );
        fileFree( file_p );
    }

    if( helpFlag )
    {
        printf( "%s: (%s %s) - Find duplicate files\n", DS_PROG_NAME, __DATE__, __TIME__ );
        printf( "\n   usage: %s <options> <directory> <directory> ...\n", DS_PROG_NAME );
        printf( "\n   options:\n\n" );
        printf( "     -s       Ignore symbolic links\n" );
        printf( "     -u       Display unique files (default: duplicate files)\n" );
        printf( "     -f       Flat (do not recurse subdirectories)\n" );
    }

    printf( "\n" );
    return 0;
}


/******************************************************************************
 * Function: freeFileEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

dsFileStruct_p fileFree( dsFileStruct_p file_p )
{
    if( file_p == NIL ) goto exit;

    if( file_p->name_p ) free( file_p->name_p );
    if( file_p->path_p ) free( file_p->path_p );
    if( file_p->pathName_p ) free( file_p->pathName_p );
exit:
    return NIL;
}

/******************************************************************************
 * Function: freeFileEntry( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

dsFileStruct_p fileAdd
(
    Char_p  name_p,
    Char_p  path_p
)
{
    dsFileStruct_p  file_p = NIL;
    Ints_t          pathLen;

    if( ( file_p = (dsFileStruct_p)calloc( (size_t)1, sizeof( dsFileStruct_t ) ) ) == NIL ) goto exit;

    if( name_p )
    {
        if( ( file_p->name_p = (Char_p)malloc( strlen( name_p ) + 1 ) ) == NIL )
        {
            file_p = fileFree( file_p );
            goto exit;
        }
        strcpy( file_p->name_p, name_p );
    }

    if( path_p )
    {
        for( ; ; )
        {
            if( ( pathLen = strlen( path_p ) ) == 0 ) break;

            if( *( path_p + pathLen - 1 ) == '/' )
            {
                *( path_p + pathLen - 1 ) = (Char_t)NIL;
            }
            else
            {
                break;
            }
        }

        if( ( file_p->path_p = (Char_p)malloc( strlen( path_p ) + 1 ) ) == NIL )
        {
            file_p = fileFree( file_p );
            goto exit;
        }
        strcpy( file_p->path_p, path_p );
    }

    if( name_p && path_p )
    {
        if( ( file_p->pathName_p = (Char_p)malloc( strlen( path_p ) + strlen( name_p ) + 2 ) ) == NIL )
        {
            file_p = fileFree( file_p );
            goto exit;
        }
        sprintf( file_p->pathName_p, "%s/%s", path_p, name_p );
    }
exit:
    return file_p;

}


/******************************************************************************
 * Function: getFileList( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t getFileList
(
    Char_p      dir_p,
    qHead_p     q_p,
    Boolean_t   recurseFlag
)
{
    struct dirent **namelist_pp = 0;
    int             n;
    struct stat     buf;
    Char_p          filename_p = NIL;
    dsFileStruct_p  file_p = NIL;
    Boolean_t       ignore;
    int             result;

#undef  MN
#define MN "dsGetFileList"

    filename_p = malloc( 512 );

    //printf( "examining directory '%s'\n", dir_p );

    n = scandir( dir_p, &namelist_pp, 0, 0 );
    if( n < 0 )
    {
        printf( "scandir() error: %d on directory '%s'\n", n, dir_p );
        goto exit;
    }
    else
    {
        printf("dir_p: %s, n: %d\n", dir_p, n);

        /* Step through all the entries */
        while( n-- )
        {
            sprintf( filename_p, "%s/%s", dir_p, namelist_pp[n]->d_name );

            if( ( result = lstat( filename_p, &buf ) ) != 0 )
            {
                /* Error */
                printf( "lstat() error: %d on file '%s'\n", result, filename_p );
            }
            else
            {
                ignore = FALSE;

#if 0
                qWalk( file_p, q_p, dsFileStruct_p )
                {
                    if( file_p->inode == buf.st_ino )
                    {
                        //printf( "already have file '%s'... in a loop?\n", namelist_pp[n]->d_name );
                        ignore = TRUE;
                        break;
                    }
                }

#endif
                if( ignore ) continue;

                ignore = TRUE;

                if( S_ISLNK( buf.st_mode ) && SkipSymlinks ) continue;

                if( S_ISREG( buf.st_mode ) )
                {
                    /* This is a regular file */
                    file_p = fileAdd( namelist_pp[n]->d_name, dir_p );
                    file_p->inode = buf.st_ino;
                    file_p->size = buf.st_size;
                    file_p->type = DS_TYPE_FILE;
                    qInsertLast( q_p, file_p );
                    printf( "added file: '%s', queue length: %lu\n", filename_p, q_p->size );

                    // printf( "inode: 0x%08lX size: %d\n", buf.st_ino, sizeof( ino_t ) );
                    ignore = FALSE;

                    TotalFileCount++;
                    if( ++TempFileCount >= 1000 )
                    {
                        printf( "Scanned %ld files...\n", TotalFileCount );
                        TempFileCount = 0;
                    }

                }
                else
                {
                    /* Check to see if this is a directory */
                    if( S_ISDIR( buf.st_mode ) && recurseFlag == TRUE )
                    {
                        if( strcmp( namelist_pp[n]->d_name, ".." ) !=  0 )
                        {
                            if( strcmp( namelist_pp[n]->d_name, "." ) !=  0 )
                            {
                                file_p = fileAdd( namelist_pp[n]->d_name, dir_p );
                                file_p->inode = buf.st_ino;
                                file_p->type = DS_TYPE_DIR;
                                qInsertLast( q_p, file_p );
                                /* This is a directory */
                                getFileList( filename_p, q_p, TRUE );
                                ignore = FALSE;
                            }
                        }
                    }
                }
                //if( ignore )  printf( "ignoring file '%s'\n", filename_p );
            }

            if( namelist_pp[n] )
            {
                free( namelist_pp[n] );
            }
            else
            {
                printf( "Got NIL namelist[%d] on %s", n, dir_p );
            }
        }
    }

exit:

    if( namelist_pp ) free( namelist_pp );
    if( filename_p ) free( filename_p );
    return 0;
}


/******************************************************************************
 * Function: strPos( )
 *-----------------------------------------------------------------------------
 * Description:
 *
 * This function returns the starting position of substring i2 within i1.
 *-----------------------------------------------------------------------------
 */

Int32s_t strPos
(
    Char_p  s1,
    Char_p  s2
)
{
    /* Check if s2 is anywhere within s1 */
    Int32s_t i1, i2, i;

#undef  MN
#define MN "strPos"

    /* Test pointers */
    if( s1 == NIL || s2 == NIL ) return -1;

    /* Get number of possible places to search */
    i1 = strlen( s1 );
    i2 = strlen( s2 );
    i1 = i1 - i2;

    /* return -1 if s1 < s2 */
    if( i1 < 0 )
    {
        return -1;
    }

    /* Search for the string */

    for( i=0; i <= i1; i++, s1++ )
    {
        if( strncmp( s1, s2, (size_t)i2 ) == 0 )
        {
            /* string is found, return starting position */
            return i;
        }
    }
    /* Not Found */

    return -1;
}

//-----------------------------------------------------------------------------
// Procedure: mbCrcInit( )
//-----------------------------------------------------------------------------
// Description:
//
// Generates a 256-word table containing all CRC remainders for every
// possible 8-bit byte.
//
// Polynomial: (MB_CRC_POLYNOMIAL)
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+x^0
//-----------------------------------------------------------------------------

void mbCrcInit( void )
{
    Int32u_t    i, j, crcTableEntry;

    for( i = 0 ; i < 256 ; i++ )
    {
        crcTableEntry = (Int32u_t)( i << 24 );
        for( j = 0 ; j < 8 ; j++ )
        {
            if ( crcTableEntry & 0x80000000L )
            {
                crcTableEntry = ( crcTableEntry << 1 ) ^ MB_CRC_POLYNOMIAL;
            }
            else
            {
                crcTableEntry = ( crcTableEntry << 1 );
            }
        }
        mbCrcTable[i] = crcTableEntry;
    }
    return;
}

//-----------------------------------------------------------------------------
// Procedure:   mbCrcFile( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Int32u_t        mbCrcFile
(
    dsFileStruct_p  file_p
)
{
    FILE           *fp = NIL;
    Int8u_p         buf_p;
    size_t          bytes;
    Int32u_t        crc = 0;
    Int32u_t        size = 0;

    buf_p = (Int8u_p)malloc( 5000 );

    if( !file_p ) goto exit;

    if( file_p->crc != 0 ) goto exit;

    if( ( fp = fopen( file_p->pathName_p, "rb" ) ) == NIL )
    {
        printf( "Could not read '%s' for CRC\n", file_p->pathName_p );
        goto exit;
    }

    for( ; ; )
    {
        bytes = fread( (Void_p)buf_p, 1, 4096, fp );
        size += bytes;

        crc = mbCrcUpdate( crc, buf_p, bytes );
        if( bytes < 4096 ) break;
    }

    bytes = size;

    while( bytes > 0 )
    {
        crc = (crc << 8) ^ mbCrcTable[((crc >> 24) ^ bytes) & 0xFF];
        bytes >>= 8;
    }

    crc = ~crc & 0xFFFFFFFF;

    file_p->crc = crc;
exit:

    if( fp ) fclose( fp );
    free( buf_p );

    return crc;
}

//-----------------------------------------------------------------------------
// Procedure: mbCrcUpdate( )
//-----------------------------------------------------------------------------
// Description:
//
//-----------------------------------------------------------------------------

Int32u_t        mbCrcUpdate
(
    Int32u_t    crc,
    Int8u_p     data_p,
    Int32u_t    dataLen
)
{
    Int32u_t    j;

    for( j = 0 ;  j < dataLen ;  j++ )
    {
        crc = ( crc << 8 ) ^ mbCrcTable[ ( ( crc >> 24 ) ^ *data_p++ ) & 0xFF ];
    }
    return crc;
}

