#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main (int argc, char **argv)
{
    const char *cp = "forbidden";
    struct in_addr in;
    int ret = inet_aton(cp, &in);
    return 0;
}
