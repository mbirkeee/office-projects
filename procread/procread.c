/* System include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "procread.h"
#include "mytypes.h"

#define TCP_FILE    "/proc/net/tcp"
#define UDP_FILE    "/proc/net/udp"
#define BUF_SIZE    2048 // Longer than a UDP packet??
#define PROC_DIR    "/proc"

#define HEART               "HEART"
#define HEARTBEAT_VERSION   "2.0"
#define HEARTBEAT_PORT      5066

#define ERR_BUF_OVERFLOW "ERROR: Buffer overflow!"

#define RX_TIMEOUT_SEC      2

#define CMD_NETSTAT         0x7F201C3B

#define DEBUG               (0)

/******************************************************************************
 * Structure for storing strings in singlely linked list
 *-----------------------------------------------------------------------------
 */

typedef struct StringList_s
{
    char                *string_p;
    struct StringList_s *next_p;
} StringList_t, *StringList_p;

/******************************************************************************
 * Structure for storing strings in singlely linked list
 *-----------------------------------------------------------------------------
 */

typedef struct RxMessage_s
{
    unsigned command;
    unsigned heartbeat;
    unsigned offset;
    unsigned handle;
} RxMessage_t, *RxMessage_p;

/******************************************************************************
 * Global Variables
 *-----------------------------------------------------------------------------
 */

StringList_p        gStringListBase_p = NIL;
StringList_p        gStringListCurrent_p = NIL;

Char_t              gBuf1[BUF_SIZE];
Char_t              gBuf2[BUF_SIZE];

#if DEBUG
Intu_t              gMallocCount = 0;
#endif

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
#if DEBUG
            gMallocCount--;
#endif
            free( current_p->string_p );
        }
        free( current_p );
#if DEBUG
        gMallocCount--;
#endif
        current_p = next_p;
    }
    gStringListBase_p = 0;
    gStringListCurrent_p = 0;

    return;
}

/******************************************************************************
 * Add line to the singly linked list of lines
 *-----------------------------------------------------------------------------
 */
void add_line(Char_p start_p)
{
    StringList_p    entry_p;

#if DEBUG
    printf("Add line: '%s'\n", start_p);
#endif

    entry_p = (StringList_p)calloc( 1, sizeof(StringList_t) );
    entry_p->string_p = (char *)malloc( strlen(start_p) + 1 );

    if( entry_p == NIL )
    {
        fprintf( stderr, "ERROR: %s: add_line(): malloc failed\n", HEART);
        return;
    }

    if( entry_p->string_p == NIL )
    {
        free( entry_p );
        fprintf( stderr, "ERROR: %s: add_line(): malloc failed\n", HEART);
        return;
    }

#if DEBUG
    gMallocCount++;
    gMallocCount++;
#endif

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
 * Check to see if this is a "pid" directory.
 *
 * Returns TRUE if there are only digits in the string name_p; FALSE otherwise
 *-----------------------------------------------------------------------------
 */

Ints_t is_pid_dir( Char_p name_p )
{
    Char_t      c;
    Ints_t      i;

    if( name_p == NIL) return FALSE;

    for( i = 0 ; ; i++ )
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
    size_t  bytes;

    bytes = snprintf(gBuf2, BUF_SIZE, "%s/%s/cwd", proc_dir_p, pid_p);
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW );
        fprintf( stderr, "ERROR: %s: read_pid_cwd(): buffer overflow\n", HEART);
        return;
    }

    // printf( "want to read CWD: %s\n", gBuf2 );

    // Read value of symlink into gBuf1
    bytes = readlink( gBuf2, gBuf1, BUF_SIZE );

    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW );
        return;
    }

    // readlink DOES NOT null terminate result!!
    gBuf1[bytes] = 0;

    // printf( "THIS IS THE CWD: %s\n", gBuf1 );

    if( strlen( gBuf1 ) <= 1 ) return;

    bytes = snprintf( gBuf2, BUF_SIZE, "pid: %s cwd: %s", pid_p, gBuf1 );
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW );
        return;
    }

    add_line( gBuf2 );
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
    Ints_t          i, j;

    bytes = snprintf(gBuf1, BUF_SIZE, "%s/%s/cmdline", proc_dir_p, pid_p);
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW );
        // printf("BUFFER OVERFLOW!!!\n");
        return;
    }

    //printf("Want to read CMDLINE: %s\n", gBuf1 );
    if( ( fp = fopen( gBuf1, "rb" ) ) == NIL )
    {
        bytes = snprintf( gBuf2, BUF_SIZE, "ERROR: failed to open: %s", gBuf1 );
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW );
        }
        else
        {
            add_line( gBuf2 );
        }
        // printf( "Failed to open file: %s\n", gBuf1 );
        return;
    }

    // Just do one read... if it fills buffer then we are in trouble!
    bytes = fread( gBuf2, 1, BUF_SIZE, fp );

    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        bytes = snprintf(gBuf1, BUF_SIZE, "ERROR: cannot read: %s/%s/cmdline", proc_dir_p, pid_p );
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW );
        }
        else
        {
            add_line( gBuf1 );
        }
        goto exit;
    }

    for( i = 0, j = 0 ; i < bytes ; i++ )
    {
        // Check for potential buffer overflow due to injected escape char
        if( j >= (BUF_SIZE - 3))
        {
            add_line( ERR_BUF_OVERFLOW );
            goto exit;
        }

        if( gBuf2[i] == 0 )
        {
            gBuf1[j++] = ' ';
        }
        else if( gBuf2[i] == '"' )
        {
            gBuf1[j++] = '\\';
            gBuf1[j++] = '"';
        }
        else if( gBuf2[i] == '\\' )
        {
            gBuf1[j++] = '\\';
            gBuf1[j++] = '\\';
        }
        else
        {
            gBuf1[j++] = gBuf2[i];
        }
        // printf( "%c %d\n", gBuf2[i], (Int8u_t)gBuf2[i] );
    }
    gBuf1[j] = 0;

    // printf("got CMDLINE: '%s'\n", gBuf1);

    if( strlen( gBuf1 ) == 0 )
    {
        // printf( "Skipping 0 length command line\n");
        goto exit;
    }

    bytes = snprintf(gBuf2, BUF_SIZE, "pid: %s cmdline: %s", pid_p, gBuf1 );
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        add_line( ERR_BUF_OVERFLOW );
        goto exit;
    }
    add_line( gBuf2 );

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
        add_line( ERR_BUF_OVERFLOW );
        return;
    }

    dir_p = opendir( gBuf1 );

    if( !dir_p )
    {
        bytes = snprintf(gBuf1, BUF_SIZE, "ERROR: Failed to open: %s/%s/fd", proc_dir_p, pid_p);
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW );
        }
        else
        {
            add_line( gBuf1 );
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
            add_line( ERR_BUF_OVERFLOW );
            // printf("BUFFER OVERFLOW!!!\n");
            continue;
        }
        // printf("GOT A SYMLINK: '%s'\n", gBuf2 );

        // Read value of symlink into gBuf1
        bytes = readlink( gBuf2, gBuf1, BUF_SIZE );

        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            add_line( ERR_BUF_OVERFLOW  );
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
            add_line( ERR_BUF_OVERFLOW );
            // printf("BUFFER OVERFLOW!!!\n");
            continue;
        }

        //printf("GOT SOCKET: %s\n", gBuf2);

        // Add this to the set of data to return
        add_line( gBuf2 );
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
        while( ( dir_entry_p = readdir( dir_p ) ) != NULL )
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

Ints_t  read_net_file( Char_p filename_p, Char_p type_p )
{
    FILE           *fp = NIL;
    size_t          bytes;
    size_t          index;
    Intu_t          size = 0;
    Intu_t          line_length;
    Char_p          next_char_p;
    size_t          rx_buf_size = BUF_SIZE;
    int             i;

    if( ( fp = fopen( filename_p, "rb" ) ) == NIL )
    {
        snprintf( gBuf1, (size_t)BUF_SIZE, "Failed to open file: %s", filename_p );
        add_line( gBuf1 );
        goto exit;
    }

    // Put the received characters into global buf2
    index = snprintf(gBuf2, (size_t)BUF_SIZE, "%s: ", type_p);
    next_char_p = &gBuf2[index];

    for( ; ; )
    {
        bytes = fread( gBuf1, 1, rx_buf_size, fp );
        if( bytes < 0 || bytes > rx_buf_size )
        {
            snprintf( gBuf1, (size_t)BUF_SIZE, "Failed to read file: %s", filename_p );
            add_line( gBuf1 );
            goto exit;
        }

        size += bytes;

        //printf( "Read %d bytes\n", size );

        // Loop through the received bytes looking for newlines and linefeeds
        for( i = 0 ; i < bytes ; i++ )
        {
            // printf( "%c", *(buf1_p + i));

            if( gBuf1[i] == '\n')
            {
                //printf( "Found a LF\n" );
                *next_char_p = 0;
                add_line( gBuf2 );
                index = snprintf(gBuf2, (size_t)BUF_SIZE, "%s: ", type_p);
                next_char_p = &gBuf2[index];
            }
            else if( gBuf1[i] == '\r' )
            {
                //printf( "Found a CR\n");
                *next_char_p = 0;
                add_line( gBuf2 );
                index = snprintf(gBuf2, (size_t)BUF_SIZE, "%s: ", type_p);
                next_char_p = &gBuf2[index];
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
                    fprintf( stderr, "ERROR: %s: Buffer overflow\n", HEART);
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
 * Read the passed in file and add lines to the singly liked list
 *-----------------------------------------------------------------------------
 */

void send_netstat(
    Ints_t                   sock,
    struct sockaddr_storage *addr_storage_p,
    Char_p                   buf_p,
    Int32u_t                 offset,
    Int32u_t                 handle
)
{
    Char_p                   line_p;
    struct sockaddr_in      *addr_p;
    Ints_t                   bytes;
    Ints_t                   status;

    // printf("send_netstat called with offset %u handle %u\n", offset, handle);

    if( offset == 0 )
    {
        // refresh the netstat data
        // printf("REFRESH START\n");
        free_string_list();
        read_net_file( TCP_FILE, "tcp" );
        read_net_file( UDP_FILE, "udp" );
        scan_pid_dir( PROC_DIR );
        // printf("REFRESH DONE\n");
    }

    line_p = get_line( offset );

    if( line_p == NIL)
    {
        bytes = snprintf(gBuf1, BUF_SIZE, "{\"handle\":\"%u\",\"offset\":\"%u\",\"line\":\"__NONE__\"}",
            handle, offset);
    }
    else
    {
        bytes = snprintf(gBuf1, BUF_SIZE, "{\"handle\":\"%u\",\"offset\":\"%u\",\"line\":\"%s\"}",
            handle, offset, line_p);
    }
    if( bytes < 0 || bytes >= BUF_SIZE )
    {
        fprintf( stderr, "%s: buffer overflow!\n", HEART);
        bytes = snprintf(gBuf1, BUF_SIZE, "{\"handle\":\"%u\",\"offset\":\"%u\",\"ERROR\":\"buffer overflow\"}",
            handle, offset);
    }

    addr_p = (struct sockaddr_in *)addr_storage_p;

    status = sendto(sock, gBuf1, bytes, 0, (struct sockaddr *)addr_p,
        sizeof(struct sockaddr_in));

    if( status < 0 ) fprintf( stderr, "%s: send_pvs() failed\n", HEART);

    return;
}

/******************************************************************************
 * Looks at received command. It must have:
 * - the expected number of bytes
 * - a valid command
 * - a heartbeat value within 1 of the current heartbeat
 *-----------------------------------------------------------------------------
 */

Ints_t process_rx_command(
    int                      sock,
    struct sockaddr_storage *addr_storage_p,
    int                      rx_count,
    char                    *buf_p,
    Int32u_t                 heartbeat
)
{
    RxMessage_p             message_p;
    Ints_t                  result = FALSE;
    Int32u_t                handle;
    Int32u_t                offset;
    Int32u_t                command;

    /* Check the received number of bytes against expected */
    if( rx_count < 0 ) goto exit;

    if( rx_count != (int)sizeof( RxMessage_t ))
    {
        fprintf( stderr, "%s: Expect %d bytes, got %d\n",
            HEART, (int)sizeof(RxMessage_t), rx_count);
        goto exit;
    }

    /* The message is in the passed in buffer */
    message_p = (RxMessage_p)buf_p;

    if( heartbeat - ntohl(message_p->heartbeat) > 1 )
    {
        fprintf(stderr, "%s: Invalid count %u (expect %u)\n", HEART,
            ntohl(message_p->heartbeat), heartbeat);
        goto exit;
    }

    handle  = ntohl(message_p->handle);
    offset  = ntohl(message_p->offset);
    command = ntohl(message_p->command);

    // printf("handle:  0x%08x\n", handle);
    // printf("offset:  0x%08x\n", offset);
    // printf("command: 0x%08x\n", command);

    if( command == CMD_NETSTAT )
    {
        send_netstat( sock, addr_storage_p, buf_p, offset, handle );
    }
    else
    {
        fprintf(stderr, "%s: Invalid command: 0x%08x\n", HEART, command);
        goto exit;
    }
    result = TRUE;
exit:
    return result;
}

#if DEBUG
/******************************************************************************
 *
 *-----------------------------------------------------------------------------
 */

Void_t display_string_list()
{
    Ints_t      i;
    size_t      bytes;
    Char_p      result_p;

    for( i = 0 ;  ; i++)
    {
        result_p = get_line( i );
        if( result_p == NIL ) break;

        bytes = snprintf( gBuf1, (size_t)BUF_SIZE, "LINE: %d: %s", i, result_p );
        if( bytes < 0 || bytes >= BUF_SIZE )
        {
            printf("buffer overflow\n");
        }
        else
        {
            printf("%s\n", gBuf1);
        }
    }
}

#endif /* DEBUG */

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
    size_t                  bytes;
    int                     yes = 1;
    int                     sock;
    int                     status;
    unsigned                counter = 0;
    unsigned long           starttime;
    unsigned long           curtime;
    unsigned long           uptime;
    unsigned long           pid;
    unsigned long           beacon_interval = 30;
    unsigned long           next_beacon_time;
    struct sockaddr_in      sock_in;
    struct sockaddr_storage rx_addr;
    socklen_t               rx_addr_len = sizeof(rx_addr);
    struct timeval          tv;
    char                   *cwd_p;

    unsigned short ca_server_port;

    printf( "sizeof( Int8u_t ):  %ld\n", sizeof( Int8u_t ) );
    printf( "sizeof( Int16u_t ): %ld\n", sizeof( Int16u_t ) );
    printf( "sizeof( Int32u_t ): %ld\n", sizeof( Int32u_t ) );
    printf( "sizeof( Int64u_t ): %ld\n", sizeof( Int64u_t ) );
    printf( "sizeof( Intu_t ):   %ld\n", sizeof( Intu_t ) );

    assert( sizeof( Int8u_t ) == 1 );
    assert( sizeof( Int16u_t ) == 2 );
    assert( sizeof( Int32u_t ) == 4 );
    assert( sizeof( Int32s_t ) == 4 );
    assert( sizeof( Int64u_t ) == 8 );


#if DEBUG
    free_string_list();

    read_net_file( TCP_FILE, "tcp" );
    read_net_file( UDP_FILE, "udp" );
    scan_pid_dir( PROC_DIR );
    display_string_list();
#endif

    // Seed random number generator
    srand(time(NULL));

    ca_server_port = (unsigned short)(10000 + 1000.0 * rand() / RAND_MAX );

    memset(&sock_in, 0, sizeof(struct sockaddr_in));

    sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_in.sin_port = htons(0);
    sock_in.sin_family = PF_INET;

    status = bind(sock, (struct sockaddr *)&sock_in, sizeof(struct sockaddr_in));
    if( status < 0 ) fprintf(stderr, "%s: bind() error: %d\n", HEART, status);

    status = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
    if( status < 0 ) fprintf( stderr, "%s: setsockopt SO_BROADCAST failed\n", HEART);

    tv.tv_sec = RX_TIMEOUT_SEC;
    tv.tv_usec = 0;

    status = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if( status < 0 ) fprintf( stderr, "%s: setsockopt SO_RCVTIMEO failed\n", HEART);

    getcwd(gBuf1, BUF_SIZE);

    cwd_p = (Char_p)malloc(strlen(gBuf1) + 1);
    strcpy(cwd_p, gBuf1);

    pid = (unsigned long)getpid();
    starttime = (unsigned long)time(0);

    next_beacon_time = starttime;

    /* Use just a single socket and thread to send heartbeats */
    /* and handle incoming requests */
    for( ; ; )
    {
        /* Look for received commands */
        bytes = recvfrom(sock, gBuf1, BUF_SIZE, 0,
            (struct sockaddr *)&rx_addr, &rx_addr_len);

        // printf("received %ld bytes\n", bytes);

        if( bytes > 0 )
        {
            /* Process any received commands */
            process_rx_command(sock, &rx_addr, bytes, gBuf1, counter);
        }

        curtime = (unsigned long)(time(0));

        // printf("curtime: %lu next_beacon_time: %lu\n", curtime, next_beacon_time);

        /* Send the next heartbeat if needed.  Due to the timeout */
        /* in the recvfrom() call above, we could be up to 'timeout' late */
        if( curtime > next_beacon_time )
        {
            counter++;
            next_beacon_time += beacon_interval;
            uptime = curtime - starttime;

            bytes = snprintf(gBuf1, BUF_SIZE,
                "{\"seq\":\"%u\",\"sp\":\"%u\",\"up\":\"%lu\",\"cur\":\"%lu\",\"pid\":\"%lu\",\"cwd\":\"%s\",\"ver\":\"%s\"}",
                counter, ca_server_port, uptime, curtime, pid, cwd_p, HEARTBEAT_VERSION
            );

            if( bytes < 0 || bytes >= BUF_SIZE )
            {
                fprintf(stderr, "%s: Buffer overflow composing heartbeat\n", HEART);
                continue;
            }

#if DEBUG
            printf("SEND HEARTBEAT: ca_server_port: %u gMallocCount: %u\n", ca_server_port, gMallocCount );
#endif

            sock_in.sin_addr.s_addr = htonl(-1); /* send message to 255.255.255.255 */
            sock_in.sin_port = htons(HEARTBEAT_PORT); /* port number */

            status = sendto(sock, gBuf1, bytes, 0,
                (struct sockaddr *)&sock_in, sizeof(struct sockaddr_in));

            if( status < 0 ) fprintf(stderr, "%s: sendto() failed\n", HEART);
        }
    }

    free(cwd_p);



    return 0;
}


