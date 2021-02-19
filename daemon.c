#include "daemon.h"

void process_packet(unsigned char *buffer, struct ip_stat *stat, int *n);
int saddr_size, data_size, sock_raw;
struct sockaddr saddr;


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

       
   		unsigned char *buffer = (unsigned char *) malloc(65536);

   		// For collecting info from network
   		struct ip_stat * stat = malloc(65536);
   		int stat_size = 0;

   		// For managing file names
   		char *PATH_STAT = getcwd(NULL, 1024), *PATH_IFACE = getcwd(NULL, 1024);
   		char *iface = malloc(20);
   		strcat(PATH_STAT, "/data/stat.dat");
   		strcat(PATH_IFACE, "/data/iface.dat");

   		// Read data from files
   		read_stat(PATH_STAT, stat, &stat_size);
   		read_iface(PATH_IFACE, iface);
        //Create a raw socket that shall sniff
        sock_raw = socket( AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
	    
       // setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+ 1 ); - Optional, for setting specific interface
	
	    if(sock_raw < 0)
	    {
		    //Print the error with proper message
		    perror("Socket Error");
		    return 1;
	    }

        // Background working
        while (1) {
            saddr_size = sizeof saddr;

            //Receive a packet
            data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, (socklen_t*)&saddr_size);

            if (data_size < 0) {
                printf("Recvfrom error , failed to get packets\n");
			    close(sock_raw);
                return 1;
            }

          //  process_packet(buffer, stat, &stat_size, iface);
            process_packet(buffer, stat, &stat_size);

        	write_stat(PATH_STAT, stat, &stat_size);
        }
    }

void process_packet(unsigned char *buffer, struct ip_stat *stat, int *n) {
    struct iphdr *iph = (struct iphdr *) buffer;
    int i;

    for (i = 0; i < *n; i++)
        if (stat[i].ip_address == iph->saddr) {
            stat[i].counter++;
            return;
        }

    // If we're here, then we got package from new IP-address
    stat[i].ip_address = iph->saddr;
    stat[i].counter = 1;
    (*n)++;

}