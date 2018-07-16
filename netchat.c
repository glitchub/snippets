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

#define PORT "7777"

#define STDIN 0 
#define STDOUT 1

// Decode sockaddr address to a static string and return it
const char *addrstr(struct sockaddr *sa)
{
    static char s[INET6_ADDRSTRLEN]; 
    void *p;
    if (sa->sa_family == AF_INET)
        p=&(((struct sockaddr_in *)sa)->sin_addr);
    else
        p=&(((struct sockaddr_in6 *)sa)->sin6_addr);
    return inet_ntop(sa->sa_family, p, s, sizeof s) ?: "*invalid*" ;
}

int main(int argc, char *argv[])
{
    int sock;

    if (argc < 2) 
    {
        // We're in server mode
        struct addrinfo hints, *res, *ai;
        struct sockaddr_storage addr;
        socklen_t addrlen=sizeof addr;
        int err, lsock;
        
        // Bind port 7777
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; 
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; 
        if ((err=getaddrinfo(NULL, PORT, &hints, &res)) != 0) die("getaddrinfo failed: %s\n", gai_strerror(err));
        
        for (ai=res; ai; ai->ai_next)
        {
            lsock=socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            err=errno;
            if (lsock >= 0)
            { 
                if (!bind(lsock, ai->ai_addr, ai->ai_addrlen)) break;
                err=errno;
                close(lsock);
            }    
        }
        if (!ai) die("socket/bind failed: %s\n", strerror(err)); // report last error
        freeaddrinfo(res);

        fprintf(stderr, "Waiting for connect on " PORT "...\n");
        if (listen(lsock, 0)) die("listen failed: %s\n", strerror(errno));
        sock=accept(lsock, (struct sockaddr *)&addr, &addrlen);
        if (sock < 0) die("accept failed: %s\n", strerror(errno));
        close(lsock); // reject subsequent connections
        fprintf(stderr, "Connected from %s\n", addrstr((struct sockaddr *)&addr)); 
    } else 
    {
        // We're in client mode
        struct addrinfo hints, *res, *ai;
        int err;
       
        // Resolve hostname
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; 
        hints.ai_socktype = SOCK_STREAM;
        if ((err=getaddrinfo(argv[1], PORT, &hints, &res)) != 0) die("getaddrinfo failed: %s\n", gai_strerror(err));
        
        // Try to connect
        for (ai=res; ai; ai=ai->ai_next)
        {
            sock=socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            err=errno;
            if (sock >= 0)
            { 
                if (!connect(sock, ai->ai_addr, ai->ai_addrlen)) break;
                err=errno;
                close(sock);
            }    
        }
        if (!ai) die("socket/connect failed: %s\n", strerror(err)); // report last error
        fprintf(stderr, "Connected to %s\n", addrstr(ai->ai_addr));
        freeaddrinfo(res);
    }

    // We now have a connection to the other terminal. Copy stdin to socket,
    // and socket to stdout.
    while(1)
    {
        // wait for activity on sock or stdin
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN, &fds);
        FD_SET(sock, &fds);
        if (select(sock+1, &fds, NULL, NULL, NULL) <= 0) die("select failed: %s\n", strerror(errno));

        if (FD_ISSET(STDIN, &fds))
        {
            // copy stdin to socket
            char buffer[256];
            ssize_t got=read(STDIN, buffer, sizeof buffer);
            if (got <= 0) die("read failed: %s\n", strerror(errno));
            while(got)
            {
                ssize_t sent=send(sock, buffer, got, 0);
                if (sent <= 0) die("send failed: %s\n", strerror(errno));
                got-=sent;
            }  
        }

        if (FD_ISSET(sock, &fds))
        {
            // copy socket to stdout
            char buffer[256];
            ssize_t got=recv(sock, buffer, sizeof buffer, 0);
            if (!got) die("Connection closed\n");
            if (got < 0) die("recv failed: %s\n", strerror(errno));  
            if (write(STDOUT, buffer, got) <= 0)  die("write failed: %s\n", strerror(errno));
        }
    }
}    

