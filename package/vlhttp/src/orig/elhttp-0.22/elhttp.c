/*
 *  extra-light http proxy server, v0.22
 *
 *  Copyright (C) 2004,2005  Christophe Devine
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef WIN32

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>

#define recv(a,b,c,d) read(a,b,c)
#define send(a,b,c,d) write(a,b,c)

#else

#pragma comment( lib, "ws2_32.lib" )

#include <winsock2.h>
#include <windows.h>

#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef uint32
#define uint32 unsigned long int
#endif

struct thread_data
{
    int client_fd;
    FILE *logfile;
    uint32 auth_ip;
    uint32 netmask;
    uint32 client_ip;
    int connect;
};

int client_thread( struct thread_data *td );

#ifndef WIN32

int main( int argc, char *argv[] )
{
    int pid;

    int n, proxy_port, proxy_fd;
    struct sockaddr_in proxy_addr;
    struct sockaddr_in client_addr;
    struct thread_data td;

    /* read the arguments */

    proxy_port = ( argc > 1 ) ?        atoi( argv[1] ) : 8208;
    td.auth_ip = ( argc > 2 ) ?   inet_addr( argv[2] ) :    0;
    td.netmask = ( argc > 3 ) ?   inet_addr( argv[3] ) :    0;
    td.logfile = ( argc > 4 ) ? fopen( argv[4], "a+" ) : NULL;
    td.connect = ( argc > 5 ) ?        atoi( argv[5] ) :    1;

    td.auth_ip &= td.netmask;
    td.client_ip = 0;

    /* is inetd mode enabled ? */

    if( proxy_port == 0 )
    {
        td.client_fd = 0; /* stdin */

        return( client_thread( &td ) );
    }

    /* fork into background */

    if( ( pid = fork() ) < 0 )
    {
        return( 2 );
    }

    if( pid ) return( 0 );

    /* create a new session */

    if( setsid() < 0 )
    {
        return( 3 );
    }

    /* close all file descriptors */

    for( n = 0; n < 1024; n++ )
    {
        close( n );
    }

#else

HANDLE tdSem;

#define close(fd) closesocket(fd)

int main( int argc, char *argv[] )
{
    int tid;
    WSADATA wsaData;

    int n, proxy_port, proxy_fd;
    struct sockaddr_in proxy_addr;
    struct sockaddr_in client_addr;
    struct thread_data td;

    FreeConsole();

    tdSem = CreateSemaphore( NULL, 0, 1, NULL );

    if( WSAStartup( MAKEWORD(2,0), &wsaData ) == SOCKET_ERROR )
    {
        return( 3 );
    }

    /* read the arguments */

    proxy_port = ( argc > 1 ) ?        atoi( argv[1] ) : 8208;
    td.auth_ip = ( argc > 2 ) ?   inet_addr( argv[2] ) :    0;
    td.netmask = ( argc > 3 ) ?   inet_addr( argv[3] ) :    0;
    td.logfile = ( argc > 4 ) ? fopen( argv[4], "a+" ) : NULL;
    td.connect = ( argc > 5 ) ?        atoi( argv[5] ) :    1;

    td.auth_ip &= td.netmask;
    td.client_ip = 0;

#endif

    /* create a socket */

    proxy_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

    if( proxy_fd < 0 )
    {
        return( 4 );
    }

    /* bind the proxy on the local port and listen */

#ifndef WIN32

    n = 1;

    if( setsockopt( proxy_fd, SOL_SOCKET, SO_REUSEADDR,
                    (void *) &n, sizeof( n ) ) < 0 )
    {
        return( 5 );
    }

#endif

    proxy_addr.sin_family      = AF_INET;
    proxy_addr.sin_port        = htons( (unsigned short) proxy_port );
    proxy_addr.sin_addr.s_addr = INADDR_ANY;

    if( bind( proxy_fd, (struct sockaddr *) &proxy_addr,
              sizeof( proxy_addr ) ) < 0 )
    {
        return( 6 );
    }

    if( listen( proxy_fd, 10 ) != 0 )
    {
        return( 7 );
    }

    while( 1 )
    {
        n = sizeof( client_addr );

        /* wait for inboud connections */

        if( ( td.client_fd = accept( proxy_fd,
                (struct sockaddr *) &client_addr, &n ) ) < 0 )
        {
            return( 8 );
        }

        td.client_ip = client_addr.sin_addr.s_addr;

        /* verify that the client is authorized */

        if( ( td.client_ip & td.netmask ) != td.auth_ip )
        {
            close( td.client_fd );
            continue;
        }

#ifndef WIN32

        /* fork a child to handle the connection */

        if( ( pid = fork() ) < 0 )
        {
            close( td.client_fd );
            continue;
        }

        if( pid )
        {
            /* in father; wait for the child to terminate */

            close( td.client_fd );
            waitpid( pid, NULL, 0 );
            continue;
        }

        /* in child; fork & exit so that father becomes init */

        if( ( pid = fork() ) < 0 )
        {
            return( 9 );
        }

        if( pid ) return( 0 );

        return( client_thread( &td ) );

#else

        /* spawn a thread to handle the connection */

        CloseHandle( CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)
                                   client_thread, &td, 0, &tid ) );

        /* wait until the thread has read its data */

        WaitForSingleObject( tdSem, INFINITE );

#endif

    }

    /* not reached */

    return( -1 );
}

void log_request( FILE *logfile, uint32 client_ip,
                  char *headers, int headers_len )
{
    int i;
    time_t t;
    struct tm *lt;
    char hyphen[2];
    char buffer[1024];
    char strbuf[32];
    char *ref, *u_a;

    memcpy( buffer, headers, headers_len + 1 );

    /* search for the Referer: and User-Agent: */

    hyphen[0] = '-';
    hyphen[1] = '\0';

    memset( strbuf, 0, sizeof( strbuf ) );

    strbuf[0] = 'R'; strbuf[3] = 'e'; strbuf[6] = 'r';
    strbuf[1] = 'e'; strbuf[4] = 'r'; strbuf[7] = ':';
    strbuf[2] = 'f'; strbuf[5] = 'e'; strbuf[8] = ' ';

    ref = strstr( buffer, strbuf );
    ref = ( ( ref == NULL ) ? hyphen : ref +  9 );

    strbuf[0] = 'U'; strbuf[4] = '-'; strbuf[ 8] = 'n';
    strbuf[1] = 's'; strbuf[5] = 'A'; strbuf[ 9] = 't';
    strbuf[2] = 'e'; strbuf[6] = 'g'; strbuf[10] = ':';
    strbuf[3] = 'r'; strbuf[7] = 'e'; strbuf[11] = ' ';

    u_a = strstr( buffer, strbuf );
    u_a = ( ( u_a == NULL ) ? hyphen : u_a + 12 );

    /* replace special characters with ' ' */

    for( i = 0; i < headers_len; i++ )
    {
        if( buffer[i] < 32 )
        {
            if( buffer[i] == '\r' && buffer[i + 1] == '\n' )
                buffer[i] = '\0';
            else
                buffer[i] = ' ';
        }
    }

    /* finally print the stuff */

    t = time( NULL );
    lt = localtime( &t );

    lt->tm_year += 1900;
    lt->tm_mon++;

    /* for some reason I dislike fixed strings in executables */

    strbuf[ 0] = '['; strbuf[11] = '%'; strbuf[22] = '0';
    strbuf[ 1] = '%'; strbuf[12] = '0'; strbuf[23] = '2';
    strbuf[ 2] = '0'; strbuf[13] = '2'; strbuf[24] = 'd';
    strbuf[ 3] = '4'; strbuf[14] = 'd'; strbuf[25] = ':';
    strbuf[ 4] = 'd'; strbuf[15] = ' '; strbuf[26] = '%';
    strbuf[ 5] = '-'; strbuf[16] = '%'; strbuf[27] = '0';
    strbuf[ 6] = '%'; strbuf[17] = '0'; strbuf[28] = '2';
    strbuf[ 7] = '0'; strbuf[18] = '2'; strbuf[29] = 'd';
    strbuf[ 8] = '2'; strbuf[19] = 'd'; strbuf[30] = ']';
    strbuf[ 9] = 'd'; strbuf[20] = ':'; strbuf[31] = '\0';
    strbuf[10] = '-'; strbuf[21] = '%';

    fprintf( logfile, strbuf,
             lt->tm_year, lt->tm_mon, lt->tm_mday,
             lt->tm_hour, lt->tm_min, lt->tm_sec );

    strbuf[ 0] = ' '; strbuf[10] = '%'; strbuf[20] = 's';
    strbuf[ 1] = '%'; strbuf[11] = 'd'; strbuf[21] = '"';
    strbuf[ 2] = 'd'; strbuf[12] = ' '; strbuf[22] = ' ';
    strbuf[ 3] = '.'; strbuf[13] = '"'; strbuf[23] = '"';
    strbuf[ 4] = '%'; strbuf[14] = '%'; strbuf[24] = '%';
    strbuf[ 5] = 'd'; strbuf[15] = 's'; strbuf[25] = 's';
    strbuf[ 6] = '.'; strbuf[16] = '"'; strbuf[26] = '"';
    strbuf[ 7] = '%'; strbuf[17] = ' '; strbuf[27] = '\r';
    strbuf[ 8] = 'd'; strbuf[18] = '"'; strbuf[28] = '\n';
    strbuf[ 9] = '.'; strbuf[19] = '%'; strbuf[29] = '\0';

    fprintf( logfile, strbuf,
             (int) ( client_ip       ) & 0xFF,
             (int) ( client_ip >>  8 ) & 0xFF,
             (int) ( client_ip >> 16 ) & 0xFF,
             (int) ( client_ip >> 24 ) & 0xFF,
             buffer, ref, u_a );

    fflush( logfile );
}

#define child_exit(ret)         \
    shutdown( client_fd, 2 );   \
    shutdown( remote_fd, 2 );   \
    close( client_fd );         \
    close( remote_fd );         \
    return( ret );

int client_thread( struct thread_data *td )
{
    int remote_fd = -1;
    int remote_port;
    int state, c_flag;
    int n, client_fd;
    uint32 client_ip;

    char *str_end, c;
    char *url_host, buffer[1024];
    char *url_port, last_host[256];

    struct sockaddr_in remote_addr;
    struct hostent *remote_host;
    struct timeval timeout;

    fd_set rfds;

    client_fd = td->client_fd;
    client_ip = td->client_ip;

#ifdef WIN32

    /* let the master thread continue */

    ReleaseSemaphore( tdSem, 1, NULL );

#endif

    /* fetch the http request headers */

    FD_ZERO( &rfds );
    FD_SET( (unsigned int) client_fd, &rfds );

    timeout.tv_sec  = 10;
    timeout.tv_usec =  0;

    if( select( client_fd + 1, &rfds, NULL, NULL, &timeout ) <= 0 )
    {
        child_exit( 11 );
    }
        
    if( ( n = recv( client_fd, buffer, 1023, 0 ) ) <= 0 )
    {
        child_exit( 12 );
    }

    memset( last_host, 0, sizeof( last_host ) );

process_request:

    buffer[n] = '\0';

    /* log the client request */

    if( td->logfile != NULL )
    {
        log_request( td->logfile, client_ip, buffer, n );
    }

    /* obfuscated CONNECT method search */

    c_flag = 0;

    if( buffer[0] == 'C' && buffer[4] == 'E' &&
        buffer[1] == 'O' && buffer[5] == 'C' &&
        buffer[2] == 'N' && buffer[6] == 'T' &&
        buffer[3] == 'N' && buffer[7] == ' ' )
    {
        if( ! td->connect )
        {
            child_exit( 13 );
        }

        c_flag = 1;
    }

    /* skip the http method (GET, PUT, etc.) */

    url_host = buffer;

    while( *url_host != ' ' )
    {
        if( ( url_host - buffer ) > 10 ||
             *url_host == '\0' )
        {
            child_exit( 14 );
        }

        url_host++;
    }

    url_host++;

    /* grab the http server hostname */

    if( ! c_flag )
    {
        if( url_host[0] == 'h' && url_host[4] == ':' &&
            url_host[1] == 't' && url_host[5] == '/' &&
            url_host[2] == 't' && url_host[6] == '/' &&
            url_host[3] == 'p' )
        {
            url_host += 7;
        }
        else
        {
            /* only http protocol is implemented */

            child_exit( 15 );
        }
    }

    /* resolve the http server hostname */

    str_end = url_host;

    while( *str_end != ':' && *str_end != '/' )
    {
        if( ( str_end - url_host ) >= 128 ||
             *str_end == '\0' )
        {
            child_exit( 16 );
        }

        str_end++;
    }

    c = *str_end;
    *str_end = '\0';

    if( ! ( remote_host = gethostbyname( url_host ) ) )
    {
        child_exit( 17 );
    }

    *str_end = c;

    /* grab the http server port */

    if( c != ':' )
    {
        c = *str_end;
        *str_end = '\0';
        remote_port = 80;
    }
    else
    {
        url_port = ++str_end;

        while( *str_end != ' ' && *str_end != '/' )
        {
            if( *str_end < '0' || *str_end > '9' )
            {
                child_exit( 18 );
            }

            str_end++;

            if( str_end - url_port > 5 )
            {
                child_exit( 19 );
            }
        }

        c = *str_end;
        *str_end = '\0';
        remote_port = atoi( url_port );
    }

    if( c_flag )
    {
        if( td->connect == 1 && remote_port != 443 )
        {
            child_exit( 20 );
        }
    }

    /* connect to the remote server, if not already connected */

    if( strcmp( url_host, last_host ) )
    {
        shutdown( remote_fd, 2 );
        close( remote_fd );

        remote_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );

        if( remote_fd < 0 )
        {
            child_exit( 21 );
        }

        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons( (unsigned short) remote_port );

        memcpy( (void *) &remote_addr.sin_addr,
                (void *) remote_host->h_addr,
                remote_host->h_length );

        if( connect( remote_fd, (struct sockaddr *) &remote_addr,
                     sizeof( remote_addr ) ) < 0 )
        {
            child_exit( 22 );
        }

        memset( last_host, 0, sizeof( last_host ) );

        strncpy( last_host, url_host, sizeof( last_host ) - 1 );
    }

    *str_end = c;

    if( c_flag )
    {
        /* send HTTP/1.0 200 OK */

        buffer[0] = 'H'; buffer[ 7] = '0'; buffer[14] = 'K';
        buffer[1] = 'T'; buffer[ 8] = ' '; buffer[15] = '\r';
        buffer[2] = 'T'; buffer[ 9] = '2'; buffer[16] = '\n';
        buffer[3] = 'P'; buffer[10] = '0'; buffer[17] = '\r';
        buffer[4] = '/'; buffer[11] = '0'; buffer[18] = '\n';
        buffer[5] = '1'; buffer[12] = ' ';
        buffer[6] = '.'; buffer[13] = 'O';

        if( send( client_fd, buffer, 19, 0 ) != 19 )
        {
            child_exit( 23 );
        }
    }
    else
    {
        int m_len; /* method string length + 1 */

        /* remove "http://hostname[:port]" & send headers */

        m_len = url_host - 7 - buffer;

        n -= 7 + ( str_end - url_host );

        memcpy( str_end -= m_len, buffer, m_len );

        if( send( remote_fd, str_end, n, 0 ) != n )
        {
            child_exit( 24 );
        }
    }

    /* tunnel the data between the client and the server */

    state = 0;

    while( 1 )
    {
        FD_ZERO( &rfds );
        FD_SET( (unsigned int) client_fd, &rfds );
        FD_SET( (unsigned int) remote_fd, &rfds );
    
        n = ( client_fd > remote_fd ) ? client_fd : remote_fd;

        if( select( n + 1, &rfds, NULL, NULL, NULL ) < 0 )
        {
            child_exit( 25 );
        }

        if( FD_ISSET( remote_fd, &rfds ) )
        {
            if( ( n = recv( remote_fd, buffer, 1023, 0 ) ) <= 0 )
            {
                child_exit( 26 );
            }

            state = 1; /* client finished sending data */

            if( send( client_fd, buffer, n, 0 ) != n )
            {
                child_exit( 27 );
            }
        }

        if( FD_ISSET( client_fd, &rfds ) )
        {
            if( ( n = recv( client_fd, buffer, 1023, 0 ) ) <= 0 )
            {
                child_exit( 28 );
            }

            if( state && ! c_flag )
            {
                /* new http request */

                goto process_request;
            }

            if( send( remote_fd, buffer, n, 0 ) != n )
            {
                child_exit( 29 );
            }
        }
    }

    /* not reached */

    return( -1 );
}
