#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define die(...) fprintf(stderr, __VA_ARGS__), exit(1)

char *usage = "\
Usage:\n\
\n\
    ipv4in [-v] host netrange [... netrange]\n\
\n\
Exit true if host IPv4 address is a member of any IPv4 net range.\n\
\n\
Net ranges are given in CIDR format \"x.x.x.x/y\".\n\
\n\
Example:\n\
\n\
  host=209.85.243.48\n\
  if ipv4in $host 64.233.160.0/19 66.102.0.0/20 108.177.8.0/21 209.85.128.0/17 216.58.192.0/19; then\n\
      echo \"$host belongs to google\"\n\
  fi\n"
;

int netrange(char *cidr, uint32_t *ip, uint32_t *mask)
{
    int res = 0;
    char *is = strdup(cidr);
    char *ms = strchr(is, '/');
    if (!*ms) goto out;
    *ms++ = 0;
    if (!*ms) goto out;

    struct in_addr ia;
    if (!inet_aton(is, &ia)) goto out;
    *ip = ntohl(ia.s_addr);

    char *ep;
    long m = strtol(ms, &ep, 10);
    if (*ep || m < 1 || m > 31) goto out;
    *mask = 0xFFFFFFFF << m;
    res = 1;
  out:
    free(is);
    return res;
}

int main(int argc, char *argv[])
{
    int verbose = 0;
    if (argc > 1 && !strcmp(argv[1],"-v"))
    {
        verbose = 1;
        argc--;
        argv++;
    }
    if (argc < 3) die("%s\n", usage);

    struct hostent *h = gethostbyname(argv[1]);
    if (!h) die("Invalid host: '%s'\n", argv[1]);
    uint32_t host = ntohl(*(uint32_t *)((struct in_addr *)h->h_addr));
    if (verbose) printf("%s -> %.8X\n", argv[1], host);

    int res = 1; // fail
    for (int i = 2; i < argc; i++)
    {
        uint32_t ip, mask;
        if (!netrange(argv[i], &ip, &mask)) die("Invalid net range '%s'\n", argv[i]);
        if (verbose) printf("%s -> %.8X %.8X\n", argv[i], ip, mask);
        if ((host & mask) == (ip & mask))
        {
            printf("%s\n", argv[i]);
            return 0;
        }
    }
    return 1; // fail
}
