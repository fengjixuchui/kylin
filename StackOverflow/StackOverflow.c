#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define MAX_LEN (255)

char g_ip[MAX_LEN] = {0} ;
int g_port = 0 ;
char g_filepath[MAX_LEN] = {0} ;

int main(int argc, char **argv)
{
    int opt = 0 ;
    
    while((opt= = getopt(argc, argv,"a:p:f:")))
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

    return 0 ;
}