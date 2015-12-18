#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "LIST.H"
#include "OBJECT.H"
#include "CACHE.H"

#define MAX_OBJECT_SIZE (100*1024)


typedef struct httpheader
{
    char  method[MAXLINE];
    char  uri[MAXLINE];
    char *ver;
    char *protocol;
    char *host;
    char *file;
    long  length;
} HttpHeader;

void service(int);
void *run(void *);

sem_t gethname;
CACHE *cache;


int main (int argc, char *argv [])
{
    int serverFd, port, *connFd, connlen, connfd;
    struct sockaddr_in connaddr;
    pthread_t tid;

    Signal(SIGPIPE, SIG_IGN);
    createCache(&cache);
    readCount = 0;
    //Check command line args
    if (argc != 2)
    {
        fprintf(stderr, "usage :  %s <port>\n", argv[0]);
        exit(1);
    }

    //Port number
    port = atoi(argv[1]);

    //Open socket and listen
    serverFd = open_listenfd(port);

    if (serverFd < 0)
    {
        fprintf(stderr, "Error: Creating socket\n");
        exit(1);
    }

    //Calculate connection len
    connlen = sizeof(connaddr);

    //Initialize semaphores
    init_sem();


    while (1)
    {

        connFd = Malloc(sizeof(int));

        //Accept connection
        *connFd = accept(serverFd, (SA *)&connaddr, &connlen);


        Pthread_create(&tid, NULL, run, connFd);

        // connfd = Accept(serverFd,(SA *)&connaddr,&connlen);
        // service(connfd);
        // Close(connfd);
    }

    return 0;
}




void init_sem()
{
    Sem_init(&gethname, 0, 1);
}

void *run(void *vargp)
{
    int connFd = *((int *)vargp);
    Pthread_detach(pthread_self());
    Free(vargp);

    if (connFd < 0)
        return;

    service(connFd);
    Close(connFd);
    return NULL;
}

void service (int connFd)
{
    int clientFd;

    rio_t rio, rio_client;

    char buf[MAXLINE];
    char resp_buf[MAX_OBJECT_SIZE];
    char *b = resp_buf;

    HttpHeader hh;

    int flag;
    int cnt;
    int size = 0;
    char uri[MAXLINE];


    //init_rio
    rio_readinitb(&rio, connFd);

    bzero(buf, MAXLINE);
    bzero(resp_buf, MAX_OBJECT_SIZE);
    bzero(uri, MAXLINE);

    //Read Line into buf
    rio_readlineb(&rio, buf, MAXLINE);

    //Parse request line and get results
    flag = parse_request(buf, &hh);

    if (flag)
        return;

    if (searchCache(cache, hh.uri, &b, &size))
    {
        puts("Return from cache");
        rio_writen(connFd, b, size);
        return;
    }

    //Parse hostname and filename
    flag = parse_host_file(&hh, uri);

    if (flag)
        return;

    // printf("Hostname:%s\n",hh.host);
    // printf("Filename:%s\n",hh.file);

    //Create Request for server
    createReq(buf, &hh);

    //Get Headers and Put to buffer
    hdrs_req(buf, &rio);

    //Connect to host
    clientFd = open_clientfd_ts(hh.host, 80);
    if ( clientFd < 0 )
        return;

    //    printf("Request:\n%s\n",buf);

    //Send request
    flag = rio_writen(clientFd, buf, strlen(buf));

    if (flag < strlen(buf))
        return;

    bzero(buf, strlen(buf));

    //Read response line and hdrs
    rio_readinitb(&rio_client, clientFd);
    rio_readlineb(&rio_client, buf, MAXLINE);
    hdrs_resp(buf, &rio_client, &hh);
    //    printf("Response:\n%s\n",buf);


    size = strlen(buf);

    if (size <= 1)
        return;

    memcpy(b, buf, size);
    b += size;

    //Send response line and hdrs
    flag = rio_writen(connFd, buf, size);

    if (flag < size)
        return;

    //Now recv and send file
    while ( (cnt = rio_readlineb(&rio_client, buf, MAXLINE)) > 0)
    {
        size += cnt;
        if (size < MAX_OBJECT_SIZE)
        {
            memcpy(b, buf, cnt);
            b += cnt;
        }
        flag = rio_writen(connFd, buf, cnt);

        if (flag < 0)
            return;
    }

    if (cnt < 0 || flag < 0)
        return;


    if (size < MAX_OBJECT_SIZE)
        insert_Object(cache, hh.uri, resp_buf, size);

}

int
parse_request(char *buf, HttpHeader *hh)
{
    if (!strcmp(buf, ""))
        return 1;

    char ver[MAXLINE];
    sscanf(buf, "%s %s %s", hh->method, hh->uri, ver);
    hh->ver = "HTTP/1.0";



    if ( (strlen(hh->method) <= 1) || (strlen(hh->uri) <= 1) )
        return 1;

    return 0;
}

int
parse_host_file(HttpHeader *hh, char *uri)
{
    char *saveptr;
    char *parse1 = ":";
    char *parse2 = "/";
    char *host, *file, *protocol;

    strcpy(uri, hh->uri);

    hh->protocol = strtok_r(uri, parse1, &saveptr);

    hh->host = strtok_r(NULL, parse2, &saveptr);
    if (hh->host == NULL )
        return 1;

    hh->file = hh->host + strlen(hh->host) + 1;
    if (hh->file == NULL)
        hh->file = "";

    return 0;
}

void
createReq(char *buf, HttpHeader *hh)
{

    char *ver = "HTTP/1.0";

    //Clear buffer
    bzero(buf, MAXLINE);

    //Create request
    sprintf(buf, "%s /%s %s\r\n", hh->method, hh->file, ver);

}

void
hdrs_req(char *request, rio_t *rio)
{

    char *connection = "Connection";
    char *proxy_connection = "Proxy-Connection";
    char *keep_alive = "Keep-Alive";
    char *close = "close";
    char buf[MAXLINE];
    char *header, *value, *saveptr;
    char *eofstr = "\r\n";

    while (1)
    {

        //Read Line
        rio_readlineb(rio, buf, MAXLINE);

        //Get header(key)
        header = strtok_r(buf, ":", &saveptr);

        //Check Headers

        if (!strcmp(header, eofstr))
        {
            sprintf(request, "%s\r\n", request);
            break;
        }
        else if ( (!strcmp(header, connection)) || (!strcmp(header, proxy_connection)) )
        {
            sprintf(request, "%s%s: %s\r\n", request, header, close);
        }
        else if ( !(strcmp(header, keep_alive)) )
        {
            continue;
        }
        else
        {
            //Get Value
            value = buf + strlen(header) + 1;
            sprintf(request, "%s%s:%s", request, header, value);
        }

    }
}

void
hdrs_resp(char *request, rio_t *rio, HttpHeader *hh)
{

    char *connection_length = "Content-Length";
    char buf[MAXLINE];
    char *header, *value, *saveptr;
    char *eofstr = "\r\n";

    hh->length = 0L;

    while (1)
    {

        //Read Line
        rio_readlineb(rio, buf, MAXLINE);

        //Get header
        header = strtok_r(buf, ":", &saveptr);

        //Check Headers

        if (!strcmp(header, eofstr))
        {
            sprintf(request, "%s\r\n", request);
            break;
        }

        value = buf + strlen(header) + 1;

        if (!strcmp(header, connection_length))
        {
            hh->length = atol(value);
        }
        sprintf(request, "%s%s:%s", request, header, value);

    }
}

struct hostent *gethostbyname_ts(char *hostname)
{

    int val = 0;
    struct hostent *sh_hostent, *unsh_hostent;
    unsh_hostent = (struct hostent *)malloc(sizeof(struct hostent));
    if (unsh_hostent == NULL)
        return NULL;

    bzero(unsh_hostent, sizeof(unsh_hostent));
    P(&gethname);
    sh_hostent = gethostbyname(hostname);
    if (sh_hostent != NULL)
        *unsh_hostent = *sh_hostent;
    else
        val = 1;
    V(&gethname);

    if (val)
    {
        free(unsh_hostent);
        return NULL;
    }

    return unsh_hostent;
}

int open_clientfd_ts(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname_ts(hostname)) == NULL)
        return -2; /* check h_errno for cause of error */

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);
    free(hp); /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    return clientfd;
}
