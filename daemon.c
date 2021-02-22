#include "daemon.h"

static void process_packet(unsigned char* buffer, struct ip_stat* stat, int* n, char iface[20]);

int main()
{

      pid_t pid = fork();// Create child process

        // Indication of fork() failure
        if (pid < 0) {
            printf("fork failed!\n");
            exit(EXIT_FAILURE);
        }

        // Parent process. Need to kill it.
        if (pid > 0) {
            printf("PID: %d\n", pid);
             exit(0);
        }
     
        //unmask the file mode
        umask(0);
        pid_t sid = setsid();//set new session,With this call, the daemon process is no longer 
        //a child process, but a process group leader now. 
        if(sid < 0)
        {
            exit(1);
         }
        //printf("Process ID: %d\n", sid);
         // Change the current working directory to root.
         chdir("/");

        // Close stdin. stdout and stderr
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
		  ////////////////socket creating

   ///////////////////socket

        int saddr_size, data_size, sock_raw;
        struct sockaddr saddr;
        unsigned char* buffer = (unsigned char*)malloc(65536);

        struct ip_stat* stat = malloc(65536);
        int stat_size = 0;
        char iface[20];
        const char *path_stat = getcwd(NULL, 1024), *path_iface = getcwd(NULL, 1024);
		strcat(path_stat, "/data/stat.dat");
   		strcat(path_iface, "/data/iface.dat");

        read_stat(path_stat, stat, &stat_size);
        read_iface(path_iface, iface);
        //Create a raw socket that shall sniff
         sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;

        // setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+ 1 ); - Optional, for setting specific interface

        if (sock_raw < 0) {
            printf("Socket Error\n");
            exit(1);
        }

        // Background working
        while (1) {
            saddr_size = sizeof saddr;

            //Receive a packet
            data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, &saddr_size);

            if (data_size < 0) {
                printf("Recvfrom error, failed to get packets\n");
                close(sock_raw);
                exit(1);
            }

            process_packet(buffer, stat, &stat_size, iface);

            write_stat(path_stat, stat, &stat_size);
        }
    }

static void process_packet(unsigned char* buffer, struct ip_stat* stat, int* n, char iface[20])
{
    struct iphdr* iph = (struct iphdr*)buffer;
    int i;

    for (i = 0; i < *n; i++)
        if (stat[i].ip_address == iph->saddr && !strcmp(stat[i].iface, iface)) {
            stat[i].counter++;
            return;
        }

    stat[i].ip_address = iph->saddr;
    stat[i].counter = 1;
    strcpy(stat[i].iface, iface);
    (*n)++;

    
}