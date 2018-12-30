/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "procread.h"
#include "mytypes.h"

#define TCP_FILE    "/proc/net/tcp"
#define UDP_FILE    "/proc/net/udp"
#define BUF_SIZE    1024
#define PROC_DIR    "/proc"

#define ERR_BUF_OVERFLOW "ERROR: Buffer overflow!"

/******************************************************************************
 * Structure for storing strings
 *-----------------------------------------------------------------------------
 */

typedef struct StringList_s
{
    char                *string_p;
    struct StringList_s *next_p;
} StringList_t, *StringList_p;

/******************************************************************************
 * Global Variables
 *-----------------------------------------------------------------------------
 */

StringList_p        gStringListBase_p = 0;
StringList_p        gStringListCurrent_p = 0;

Char_t              gBuf1[BUF_SIZE];
Char_t              gBuf2[BUF_SIZE];

/******************************************************************************
 * Frees memory allocated for the singly linked list of strings
 *-----------------------------------------------------------------------------
 */

void free_string_list()
{
    StringList_p    current_p;
    StringList_p    next_p;

    current_p = gStringListBase_p;

    while( current_p )
    {
        next_p = current_p->next_p;
        if( current_p->string_p )
        {
            // gMallocCount--;
            free( current_p->string_p );
        }
        free( current_p );
        // gMallocCount--;
        current_p = next_p;
    }

    //if( gMallocCount != 0 )
   // {
   //     fprintf(stderr, "%s: non-zero malloc count: %d\n", HEART, gMallocCount);
   // }
    gStringListBase_p = 0;
    gStringListCurrent_p = 0;

    return;
}

/******************************************************************************
 * Add line to the singly linked list of lines
 *-----------------------------------------------------------------------------
 */
void add_line(Char_p start_p, Char_p end_p)
{
    Intu_t          line_length;
    StringList_p    entry_p;

    if( end_p != NIL )
    {
        line_length = (Intu_t)(end_p - start_p);
        // printf("Want to add line of length: %d\n", line_length);

        if( line_length == 0 ) return;

        // Terminate the line. Assume that there is space in the buffer
        // (i.e., assume that length checked in calling code)
        *end_p = 0;
    }

    // printf("Add line: '%s'\n", start_p);

    entry_p = (StringList_p)calloc( 1, sizeof(StringList_t) );
    entry_p->string_p = (char *)malloc( strlen(start_p) + 1 );

    strcpy( entry_p->string_p, start_p );

    /* Point global list pointer to first element in list */
    if( gStringListBase_p == 0 ) gStringListBase_p = entry_p;

    /* Link element to previous element */
    if( gStringListCurrent_p ) gStringListCurrent_p->next_p = entry_p;

    gStringListCurrent_p = entry_p;

    return;
}

/******************************************************************************
 * Return the line specified by the index
 *-----------------------------------------------------------------------------
 */

Char_p get_line(int index)
{
    StringList_p    entry_p;
    int             i;

    // printf( "Want to get line index: %d\n", index);

    if( gStringListBase_p == 0 ) return NIL;

    entry_p = gStringListBase_p;

    // Loop until desired entry found.  This is a little "brute force"
    for( i = 0 ; i < index ; i++ )
    {
        if( entry_p == 0 )
        {
            break;
        }

        if( entry_p->next_p == 0 )
        {
            entry_p = 0;
            break;
        }
        entry_p = entry_p->next_p;
    }

    if( entry_p == 0 ) return NIL;

    return entry_p->string_p;
}

/******************************************************************************
 *
 *-----------------------------------------------------------------------------
 */

Ints_t is_pid_dir( Char_p name_p )
{
    Char_t  c;
    int     i;

    if( name_p == NIL) return FALSE;

    for( i = 0 ; ; i++)
    {
        c = *(name_p + i);
        if( c == 0 ) break;

        if( c < '0' || c > '9') return FALSE;
    }
    return TRUE;
}

/******************************************************************************
 *
 *-----------------------------------------------------------------------------
 */

Void_t read_pid_cwd( Char_p proc_dir_p, Char_p pid_p )
{
    size_t          bytes;

    bytes = snprintf(gBuf2, BUF_SIZE, "%s/%s/cwd", proc_dir_p, pid_p);
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line(ERR_BUF_OVERFLOW, NIL);
        // printf("BUFFER OVERFLOW!!!\n");
        return;
    }

    // printf( "want to read CWD: %s\n", gBuf2 );

    // Read value of symlink into gBuf1
    bytes = readlink( gBuf2, gBuf1, BUF_SIZE );

    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW, NIL );
        return;
    }

    // readlink DOES NOT null terminate result!!
    gBuf1[bytes] = 0;

    // printf( "THIS IS THE CWD: %s\n", gBuf1 );

    if( strlen(gBuf1) <= 1) return;

    bytes = snprintf(gBuf2, BUF_SIZE, "pid: %s cwd: %s", pid_p, gBuf1 );
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line(ERR_BUF_OVERFLOW, NIL);
        return;
    }

    add_line(gBuf2, NIL);
    return;
}

/******************************************************************************
 *
 *-----------------------------------------------------------------------------
 */

Void_t read_pid_cmdline( Char_p proc_dir_p, Char_p pid_p )
{
    FILE           *fp = NIL;
    size_t          bytes;
    int             i, j;

    bytes = snprintf(gBuf1, BUF_SIZE, "%s/%s/cmdline", proc_dir_p, pid_p);
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line(ERR_BUF_OVERFLOW, NIL);
        // printf("BUFFER OVERFLOW!!!\n");
        return;
    }

    //printf("Want to read CMDLINE: %s\n", gBuf1 );

    if( ( fp = fopen( gBuf1, "rb" ) ) == NIL )
    {
        bytes = snprintf(gBuf2, BUF_SIZE, "ERROR: cannot open: %s", gBuf1);
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line(ERR_BUF_OVERFLOW, NIL);
        }
        else
        {
            add_line(gBuf2, NIL);
        }
        // printf( "Failed to open file: %s\n", gBuf1 );
        return;
    }

    // Just do one read... if it fills buffer then we are in trouble!
    bytes = fread( gBuf2, 1, BUF_SIZE, fp );

    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        bytes = snprintf(gBuf1, BUF_SIZE, "ERROR: cannot read: %s/%s/cmdline", proc_dir_p, pid_p);
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line(ERR_BUF_OVERFLOW, NIL);
        }
        else
        {
            add_line(gBuf1, NIL);
        }
        goto exit;
    }

    for( i = 0, j = 0 ; i < bytes ; i++ )
    {
        // Check for potential buffer overflow before escaping potential quote escape
        if( j >= (BUF_SIZE - 3))
        {
            add_line(ERR_BUF_OVERFLOW, NIL);
            goto exit;
        }

        if( gBuf2[i] == 0 )
        {
            gBuf1[j++] = ' ';
        }
        else if( gBuf2[i] == '"' )
        {
            gBuf1[j++] = '\\';
            gBuf1[j++] = '\\';
            gBuf1[j++] = '"';
        }
        else
        {
            gBuf1[j++] = gBuf2[i];
        }
    //    printf( "%c %d\n", gBuf2[i], (Int8u_t)gBuf2[i] );
    }
    gBuf1[i] = 0;

    // printf("got CMDLINE: '%s'\n", gBuf1);

    if( strlen( gBuf1 ) == 0 )
    {
        // printf( "Skipping 0 length command line\n");
        goto exit;
    }

    bytes = snprintf(gBuf2, BUF_SIZE, "pid: %s cmdline: %s", pid_p, gBuf1 );
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line(ERR_BUF_OVERFLOW, NIL);
        goto exit;
    }
    add_line(gBuf2, NIL);

exit:
    if( fp ) fclose( fp );
    return;
}

/******************************************************************************
 *
 *-----------------------------------------------------------------------------
 */

Void_t read_pid( Char_p proc_dir_p, Char_p pid_p )
{
    size_t          bytes;
    DIR             *dir_p;
    struct dirent   *dir_entry_p;

    //printf("Want to read pid: %s\n", pid_p);

    bytes = snprintf(gBuf1, BUF_SIZE, "%s/%s/fd", proc_dir_p, pid_p);
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW, NIL );
        return;
    }

    dir_p = opendir( gBuf1 );

    if( !dir_p )
    {
        bytes = snprintf(gBuf1, BUF_SIZE, "ERROR: Failed to open: %s/%s/fd", proc_dir_p, pid_p);
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW, NIL );
        }
        else
        {
            add_line(gBuf1, NIL);
        }
        return;
    }

    while( ( dir_entry_p = readdir( dir_p )) != NULL )
    {
        // printf("Got : %s\n", dir_entry_p->d_name);
        if( dir_entry_p->d_type != DT_LNK ) continue;

        bytes = snprintf(gBuf2, BUF_SIZE, "%s/%s/fd/%s", proc_dir_p, pid_p, dir_entry_p->d_name);

        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW, NIL );
            // printf("BUFFER OVERFLOW!!!\n");
            continue;
        }
        // printf("GOT A SYMLINK: '%s'\n", gBuf2 );

        // Read value of symlink into gBuf1
        bytes = readlink( gBuf2, gBuf1, BUF_SIZE );

        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW, NIL );
           // printf("BUFFER OVERFLOW!!!\n");
            continue;
        }

        // readlink DOES NOT null terminate result!!
        gBuf1[bytes] = 0;

        //printf("written: %ld\n", written );
        if( strstr(gBuf1, "socket") == NULL )
        {
            // printf( "SKIPPING non socket fd %s\n", gBuf1);
            continue;
        }
        //printf("GOT SOCKET: %s\n", gBuf1);

        // Write result into a buffer preceeded by the PID
        bytes = snprintf(gBuf2, BUF_SIZE, "pid: %s %s", pid_p, gBuf1);

        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW, NIL );
            // printf("BUFFER OVERFLOW!!!\n");
            continue;
        }

        //printf("GOT SOCKET: %s\n", gBuf2);

        // Add this to the set of data to return
        add_line(gBuf2, NIL);
    }
    closedir( dir_p );

    // Read the cmdline and cwd for this pid
    read_pid_cmdline( proc_dir_p, pid_p );
    read_pid_cwd( proc_dir_p, pid_p );

    return;
}

/******************************************************************************
 *
 *-----------------------------------------------------------------------------
 */

Void_t scan_pid_dir( Char_p dirname_p )
{
    DIR             *dir_p;
    struct dirent   *dir_entry_p;

    dir_p = opendir( dirname_p );

    if( dir_p )
    {
        while( ( dir_entry_p = readdir( dir_p )) != NULL )
        {
            if( is_pid_dir( dir_entry_p->d_name ) )
            {
                read_pid( dirname_p, dir_entry_p->d_name );
            }
        }
        closedir( dir_p );
    }
    return;
}

/******************************************************************************
 * Read the passed in file and add lines to the singly liked list
 *-----------------------------------------------------------------------------
 */

Ints_t  read_net_file( Char_p filename_p )
{
    FILE           *fp = NIL;
    size_t          bytes;
    Intu_t          size = 0;
    Intu_t          line_length;
    Char_p          next_char_p;
    size_t          rx_buf_size = BUF_SIZE;
    int             i;

    if( ( fp = fopen( filename_p, "rb" ) ) == NIL )
    {
        printf( "Failed to open file: '%s'\n", filename_p );
        goto exit;
    }

    // Put the received characters into global buf2
    next_char_p = gBuf2;

    for( ; ; )
    {
        bytes = fread( gBuf1, 1, rx_buf_size, fp );
        size += bytes;

        //printf( "Read %d bytes\n", size );

        // Loop through the received bytes looking for newlines and linefeeds
        for( i = 0 ; i < bytes ; i++ )
        {
            // printf( "%c", *(buf1_p + i));

            if( gBuf1[i] == '\n')
            {
                //printf( "Found a LF\n" );
                add_line(gBuf2, next_char_p);
                next_char_p = gBuf2;
            }
            else if( gBuf1[i] == '\r' )
            {
                //printf( "Found a CR\n");
                add_line(gBuf2, next_char_p);
                next_char_p = gBuf2;
            }
            else
            {
                // Add the found character to the line buffer (IF THERE IS ROOM!!)
                line_length = (Intu_t)(next_char_p - gBuf2);

                if( line_length < ( BUF_SIZE - 1 ))
                {
                    //printf( "This is the length: %d\n", line_length);
                    *next_char_p++ = gBuf1[i];
                }
                else
                {
                    printf("ERROR! Buffer overflow storing line!\n");
                }
            }
        }

        if( bytes < rx_buf_size )
        {
            // printf("Must be done, got less than a full buffer\n");
            break;
        }
    }

exit:

    if( fp != NIL ) fclose( fp );
    return 0;
}

/******************************************************************************
 * Procedure: main( )
 *-----------------------------------------------------------------------------
 * This code is written to be as lean as possible.
 *---------------------------------------------------------------------------*/

int main
(
    int             argc,
    Char_p          argv[]
)
{
    Char_p          result_p;
    size_t          bytes;
    int             i;

//    printf( "Hello world!\n" );

//    printf( "sizeof( Int8u_t ): %ld\n", sizeof( Int8u_t ) );
//    printf( "sizeof( Int16u_t ): %ld\n", sizeof( Int16u_t ) );
//    printf( "sizeof( Int32u_t ): %ld\n", sizeof( Int32u_t ) );
//    printf( "sizeof( Int64u_t ): %ld\n", sizeof( Int64u_t ) );
//    printf( "sizeof( Intu_t ): %ld\n", sizeof( Intu_t ) );

    free_string_list();

    read_net_file( TCP_FILE );
    read_net_file( UDP_FILE );
    scan_pid_dir( PROC_DIR );

    // Loop through all the data buffered for return
//    printf( "THIS IS THE ACQUIRED DATA -----------------------\n");

    for( i = 0 ;  ; i++)
    {
        result_p = get_line( i );
        if( result_p == NIL ) break;

        bytes = snprintf( gBuf1, (size_t)BUF_SIZE, "{\"i\":\"%d\",\"l\":\"%s\"}", i, result_p );
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            printf("buffer overflow\n");
        }
        else
        {
            printf("%s\n", gBuf1);
        }
    }

    return 0;
}


