/* timeserver.c - a socket-based time of day server
 *
 * server side:
 * cc timeserver.c -o timeserver
 * ./timeserver &
 *
 * client side:
 * telnet 127.0.1.1 13000
 *
 * tips:
 * netstat -tnl
 *
 * make sure that the server is on the right port
 *
 */

#include  <stdio.h>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <netdb.h>
#include  <time.h>
#include  <strings.h>
#include  <stdlib.h>

#define   PORTNUM  13000   /* our time service port number */
#define   HOSTLEN  256
#define   oops(msg)      { perror(msg); exit(1); }


void
clear_out_struct(struct sockaddr_in *addr_info) {
    bzero((void *) addr_info, sizeof((*addr_info)));
}

void
get_host_info(struct hostent **host_info) {
    char hostname[HOSTLEN];
    gethostname(hostname, HOSTLEN);
    *host_info = gethostbyname(hostname);
}

void
fill_host_info(struct sockaddr_in *addr_info, struct hostent *host_info, uint16_t port_num) {
    clear_out_struct(addr_info);

    get_host_info(&host_info);

    /* fill in host part    */
    bcopy((void *) host_info->h_addr, (void *) (&(*addr_info).sin_addr), host_info->h_length);
    (*addr_info).sin_port = htons(port_num);        /* fill in socket port  */
    (*addr_info).sin_family = AF_INET;            /* fill in addr family  */
}

int
get_tcp_socket(void) {
    int sock_id = socket(PF_INET, SOCK_STREAM, 0);    /* get a socket */
    if (sock_id == -1) {
        oops("socket");
    }
    return sock_id;
}

int main(int ac, char *av[]) {
    struct sockaddr_in saddr;   /* build our address here */
    struct hostent *hp;         /* this is part of our    */
    int sock_id, sock_fd;       /* line id, file desc     */
    FILE *sock_fp;              /* use socket as stream   */
    char *ctime();              /* convert secs to string */
    time_t thetime;             /* the time we report     */

    /*
     * Step 1: ask kernel for a socket
     */

    sock_id = get_tcp_socket();

    /*
     * Step 2: bind address to socket.  Address is host,port
     */

    fill_host_info(&saddr, hp, PORTNUM);

    if (bind(sock_id, (struct sockaddr *) &saddr, sizeof(saddr)) != 0) {
        oops("bind");
    }

    /*
     * Step 3: allow incoming calls with Qsize=1 on socket
     */

    if (listen(sock_id, 1) != 0) {
        oops("listen");
    }

    /*
     * main loop: accept(), write(), close()
     */

    while (1) {
        sock_fd = accept(sock_id, NULL, NULL); /* wait for call */
        printf("Wow! got a call!\n");
        if (sock_fd == -1) {
            oops("accept"); /* error getting calls  */
        }

        sock_fp = fdopen(sock_fd, "w");
        if (sock_fp == NULL) {
            oops("fdopen");
        }

        thetime = time(NULL);

        fprintf(sock_fp, "The time here is ..");
        fprintf(sock_fp, "%s", ctime(&thetime));
        fclose(sock_fp);
    }
}