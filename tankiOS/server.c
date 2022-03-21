#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "server.h"
#include "WoT.h"

void *udp_server(__attribute__((unused)) void *none)
{
    int sock, ret;
    unsigned int addr_len;
    fd_set readfd;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return NULL;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    addr_len = sizeof(addr);

    if (bind(sock, (struct sockaddr *)&addr, addr_len) < 0)
    {
        perror("bind");
        close(sock);
        return NULL;
    }
    int key;
     while (!exit_status)
    {
        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);

        ret = select(sock + 1, &readfd, NULL, NULL, 0);
        if (ret > 0)
        {
            if (FD_ISSET(sock, &readfd))
            {
            recvfrom(sock, &key, sizeof(key), 0, (struct sockaddr *)&addr, &addr_len);
            move_player(key, addr);
            }
        }
    }

    close(sock);
    return (void *)NULL;
}