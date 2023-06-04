#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>

int num = 0;

void child_read(int sig){
    pid_t pid;
    int status;
    while(pid = waitpid(-1, &status, WNOHANG) > 0){
        printf("removed proc: %d \n", pid);
        num++;
    }
    
}

int main(int argc, char *argv[]){

    struct sigaction act;
    int state;
    act.sa_handler = child_read;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

    int serv_sock, clnt_sock, clnt_addr_sz;
    struct sockaddr_in serv_sock_addr, clnt_sock_addr, clnt_addr;

    int len;
    int arr1[12];
    int arr2[12];
    char test[30];
    int check = 0; 
    int str_len = 0;
    int random = 0;

    int pid;

    int fds1[2];
    int fds2[2];

    //recive socket
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_sock_addr, 0, sizeof(serv_sock_addr));
    serv_sock_addr.sin_family = AF_INET;
    serv_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_sock_addr.sin_port = htons(50000);
    
    int enable = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    
    if(bind(serv_sock, (struct sockaddr*)&serv_sock_addr, sizeof(serv_sock_addr)) == -1){
        perror("bind() error");
        exit(1);
    }

    if(listen(serv_sock, 5) == -1){
        perror("listen");
        exit(1);
    }

    pipe(fds1);
    pipe(fds2);

    pid = getpid();
    printf("parent pid: %d \n", pid);

    while(1){
        clnt_addr_sz = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
        int tem = sizeof(clnt_addr);

        check++;
        //fork로 생겨난 자식프로세스와 부모프로세스는 독립적이지 않은가?
        
        if(check == 1){
            if((pid=fork()) == -1){
                perror("fork");
                exit(1);
            }
            if(pid == 0){
                int count = 0;
                int order = 1;
                int con;
                int pid = getpid();
                printf("proc1: %d \n", pid);

                write(clnt_sock, &order, sizeof(order));
                read(fds2[0], &con, sizeof(con));
                write(clnt_sock, &con, sizeof(con));

                close(serv_sock); //***
                while(1) {
                    
                    if(count == 12)
                    {
                        printf("proc1 end \n");
                        break;
                    }
                    //read socket
                    str_len = read(clnt_sock, arr1, sizeof(arr1));
                    if(str_len == 0){
                        memset(arr1, 0, sizeof(arr1));
                        continue;
                    }
                    printf("client1: <To client1> \n");
                    for(int i=0;i<sizeof(arr1)/sizeof(int);i++){
                        printf("%d ", arr1[i]);
                    }
                    printf("\n");
                    //sleep(1);

                    //write pipe
                    write(fds1[1], arr1, sizeof(arr1));
                    //sleep(1);
                    
                    //read pipe
                    str_len = read(fds2[0], arr2, sizeof(arr2));
                    if(str_len == 0){
                        continue;
                    }
                    printf("client1: <read pipe> \n");
                    for(int i=0;i<12;i++){
                        printf("%d ", arr2[i]);
                    }
                    printf("\n");
                    //sleep(1);
                    
                    //write socket
                    write(clnt_sock, arr2, sizeof(arr2));
                    //sleep(1);

                    memset(arr1, 0, sizeof(arr1));
                    memset(arr2, 0, sizeof(arr2));
                    count++;
                }

                
                //close(clnt_sock);

                return 1;
            }
            else{
                close(clnt_sock);
            }
        }
        else if(check == 2){
            if((pid = fork()) == -1){
                perror("fork");
                exit(1);
            }
            if(pid == 0){
                int count = 0;
                int order = 2;

                int pid = getpid();
                printf("proc2: %d \n", pid);

                close(serv_sock); //***
                write(fds2[1], &order, sizeof(order));
                write(clnt_sock, &order, sizeof(order));
                while(1){

                    if(count == 12)
                    {   
                        printf("proc2 end \n");
                        break;
                    }

                    //read pipe
                    str_len = read(fds1[0], arr1, sizeof(arr1));
                    if(str_len == 0){
                        memset(arr1, 0, sizeof(arr1));
                        continue;
                    }
                    
                    //wirte socket
                    write(clnt_sock, arr1, sizeof(arr1));
                    printf("<client 2: read to pipe> \n");
                    for(int i = 0;i<sizeof(arr1)/sizeof(int);i++){
                        printf("%d ", arr1[i]);
                    }
                    printf("\n");
                    
                    //read socket
                    str_len = read(clnt_sock, arr2, sizeof(arr2));
                    if(str_len == 0){
                        memset(arr2, 0, sizeof(arr2));
                        continue;
                    }
                    printf("<To client2> \n");
                    for(int i=0;i<12;i++){
                        printf("%d ", arr2[i]);
                    }
                    printf("\n");

                    //write pipe
                    write(fds2[1], arr2, sizeof(arr2));

                    memset(arr1, 0, sizeof(arr1));
                    memset(arr2, 0, sizeof(arr2));
                    count++;
                }
                
                
                //close(clnt_sock);
                return 1;
            }
            else{
                close(clnt_sock);
            }
        }
        
        else if(check > 2){
            if(num == 2){       //child process count
                break;
            }
            int pid = getpid();
            int order = 0;
            printf("other proc: %d \n", pid);
            printf("Noooo~ \n");
            write(clnt_sock, &order, sizeof(int));
            continue;
            //close(serv_sock);
            //close(clnt_sock);
            //exit(1);
        }
    }
    return 0;
}