#ifndef TEST_DAEMON_H
#define TEST_DAEMON_H

#include <string.h>
#include <stdio.h>
#include <ifaddrs.h> //for interfaces
#include <stdlib.h> //exit
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <zconf.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include<netinet/ip_icmp.h>	//Provides declarations for icmp header
#include<netinet/udp.h>	//Provides declarations for udp header
#include<netinet/tcp.h>	//Provides declarations for tcp header
#include<netinet/ip.h>	//Provides declarations for ip header
#include<netinet/if_ether.h>	//For ETH_P_ALL
#include<net/ethernet.h>	//For ether_header
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<sys/time.h>

struct ip_stat {
    unsigned int ip_address;
    unsigned int counter;
    char iface[20];
};

void read_stat();

void outputall(const char* stat_file)
{
    struct ip_stat* stat = malloc(65536);
    int n = 0;
    read_stat(stat_file, stat, &n);

    printf("Number IP: %d\n", n);

    struct in_addr tmp;
    for (int i = 0; i < n; i++) {
        tmp.s_addr = stat[i].ip_address;
        printf("%s: %d %s\n", inet_ntoa(tmp), stat[i].counter, stat[i].iface);
    }
    printf("\n");
    free(stat);
}

void outputif(const char* stat_file, const char* argv)
{

    char temp[20];
    strcpy(temp, argv);

    struct ip_stat* stat = malloc(65536);
    int n = 0;
    read_stat(stat_file, stat, &n);

    printf("Number IP: %d\n", n);

    struct in_addr tmp;
    for (int i = 0; i < n; i++) {
        if (!strcmp(temp, stat[i].iface)) {
            tmp.s_addr = stat[i].ip_address;
            printf("%s: %d %s\n", inet_ntoa(tmp), stat[i].counter, stat[i].iface);
        }
    }
    printf("\n");
 
}



void write_stat(const char* name, const struct ip_stat* stat, const int* size)
{
    FILE* f = fopen(name, "r+b");

    if (f == NULL)
        f = fopen(name,"w+b");
    fwrite(size, sizeof *size, 1, f);
    for (int i = 0; i < *size; i++)
        fwrite(&stat[i], sizeof(struct ip_stat), 1, f);

    fclose(f);
}

void write_iface(const char* name, const char* iface)
{
    FILE* f = fopen(name, "wb+");

    if (f == NULL)
        return;

    fwrite(iface, sizeof(char), strlen(iface) + 1, f);

    fclose(f);
}

void read_stat(const char* name, struct ip_stat* stat, int* size)
{
    FILE* f = fopen(name, "rb");

    if (f == NULL) {
        *size = 0;
        return;
    }

    fread(size, sizeof(int), 1, f);

    for (int i = 0; i < *size; i++)
        fread(&stat[i], sizeof(struct ip_stat), 1, f);

    fclose(f);
}

void read_iface(const char* name, char* iface)
{
    FILE* f = fopen(name, "rb");

    if (f == NULL) {
        // default interface
        strcpy(iface, "eth0");
        return;
    }

    fread(iface, 1, 20, f);
    fclose(f);
}

//log(n)
int search_ip(const struct ip_stat* stat, const int size,const unsigned ip_addr)
{
    int min = 0, max = size - 1, i;

    while (min <= max) {
        i = (min + max) / 2;

        if (ip_addr == stat[i].ip_address)
            return stat[i].counter;

        if (ip_addr > stat[i].ip_address)
            min = i + 1;
        else
            max = i - 1;
    }
    return 0;
}
#endif
