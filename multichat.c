#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

#define IP "225.0.0.7"
#define PORT 7777

#define STDIN 0 
#define STDOUT 1

int main(int argc, char *argv[])
{
    struct sockaddr_in sa;
    int sock;
    struct ip_mreq mreq;

    // generic udp socket
    sock=socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) die("socket failed: %s\n", strerror(errno));

    // create sockaddr
    memset(&sa, 0, sizeof sa);
    sa.sin_family=AF_INET;
    sa.sin_addr.s_addr=inet_addr(IP); // bind doesn't care about this but sendto does
    sa.sin_port=htons(PORT);

    // bind the port
    if (bind(sock,(struct sockaddr *)&sa, sizeof sa) < 0) die("bind failed: %s\n", strerror(errno));
    
    // enable socket for multicast receive  
    memset(&mreq,0,sizeof mreq);
    mreq.imr_multiaddr.s_addr=inet_addr(IP);
    mreq.imr_interface.s_addr=INADDR_ANY;
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof mreq)<0) die ("IP_ADDR_MEMBERSHIP failed: %s\n", strerror(errno));
    
    // but don't receive our own multicast tx
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP, (int[]){0}, sizeof(int)) < 0) die("IP_MULTICAST_LOOP failed: %s\n", strerror(errno));

    while(1)
    {
        fd_set fds;
        char buffer[256]; // buffer must be the same size in both directions

        // wait for activity on sock or stdin
        FD_ZERO(&fds);
        FD_SET(STDIN, &fds);
        FD_SET(sock, &fds);
        if (select(sock+1, &fds, NULL, NULL, NULL) <= 0) die("select failed: %s\n", strerror(errno));

        if (FD_ISSET(STDIN, &fds))
        {
            // copy stdin to socket
            ssize_t got=read(STDIN, buffer, sizeof buffer);
            if (got <= 0) die("read failed: %s\n", strerror(errno));
            if (sendto(sock, buffer, got, 0, (struct sockaddr *) &sa, sizeof sa) < 0) die("sendto failed: %s\n", strerror(errno));
        }

        if (FD_ISSET(sock, &fds))
        {
            // copy rx socket to stdout
            ssize_t got=recvfrom(sock, buffer, sizeof buffer, 0, NULL, NULL);
            if (got < 0) die("recvfrom failed: %s\n", strerror(errno));  
            if (write(STDOUT, buffer, got) <= 0)  die("write failed: %s\n", strerror(errno));
        }
    }
}    

