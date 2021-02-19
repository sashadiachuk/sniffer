#include "daemon.h"

char iface[20];
char ifaces[20][20];
int countIfaces = 0;
void set_paths(char **p_daem,  char **p_stat, char **p_iface);
static void getInterfaces();

int main(int argc, char* argv[])
{
    char *path_daemon = NULL, *path_stat = NULL, *path_iface = NULL;
    set_paths(&path_daemon, &path_stat, &path_iface);
    getInterfaces();

    if (!strcmp(argv[1], "--help")) {
        printf("Available interfaces:\n");
        for (int i = 0; i < countIfaces; ++i)
            printf("%-8s \n", ifaces[i]);
        printf("Commands to use:\n");
        printf("start               - start sniff default interface(eth0)\n");
        printf("stop                - stop sniff any interface\n");
        printf("show   [ip]         - used to show number of received packets  from selected ip\n");
        printf("selIface  [iface]   - start sniff from selected interface\n");
        printf("statIface [iface]   - tprint to standard output gathered statistics for particular interface, if iface not submitted - for all interfaces \n");
        printf("--help              - help menu\n");
       }
    else if (!strcmp(argv[1], "start")) {
        printf("Starting daemon...\n");
        system("sudo ./daemon");
        printf("Status: %d\n", system(path_daemon));
       }
    else if (!strcmp(argv[1], "stop")) {
        printf("Stoppping daemon...\n");
        system("sudo kill `pidof ./daemon` 2> /dev/null");
    }
    else if (argc >= 3 && !strcmp(argv[1], "show")) {
        struct ip_stat* stat = malloc(65536);
        int stat_size = 0;
        read_stat(path_stat, stat, &stat_size);
        int n;
        // read passed argument of IP address
        unsigned ip = inet_addr(argv[2]);

        printf("Count packets: %d\n", search_ip(stat, stat_size, ip));
        
    }
    else if (argc >= 3 && !strcmp(argv[1], "selIface")) {
        bool res = false;
        for (int i = 0; i < countIfaces; ++i) {
            if (!strcmp(argv[2], ifaces[i])) {// Compare two strings
                strcpy(iface, argv[2]);
                res = true;
                write_iface(path_iface, iface);
                printf("Starting daemon...\n");
                system("sudo ./daemon");
                break;
            }
        }
        if (!res) {
            printf("Interface not found\n");
            exit(1);
        }
    }
    else if (!strcmp(argv[1], "statIface")) {
        output(path_stat);//Specifies the output file name.
    }
    else
        printf("Command not found\n");
    return 0;
}

static void getInterfaces()
{
    struct ifaddrs *ifaddr, *ifa;
    int n;
    if (getifaddrs(&ifaddr) == -1) {//get interface addresses
        perror("getifaddrs");
        exit(1);
    }
    for (ifa = ifaddr, n = 0; ifa != NULL && ifa->ifa_addr->sa_family == AF_PACKET; ifa = ifa->ifa_next, ++n) {
        if (ifa->ifa_addr == NULL)
            continue;
        strcpy(ifaces[n], ifa->ifa_name);
        ++countIfaces;
    }
    freeifaddrs(ifaddr);//get interface addresses
}
// Set files paths for accessing daemon's data
void set_paths(char **p_daem, char **p_stat, char **p_iface) {
    *p_daem = getcwd(NULL, 1024);//get current working directory
    *p_stat = getcwd(NULL, 1024);
    *p_iface = getcwd(NULL, 1024);

    strcat(*p_daem, "/daemon");
    strcat(*p_stat, "/data/stat.dat");
    strcat(*p_iface, "/data/iface.dat");
}