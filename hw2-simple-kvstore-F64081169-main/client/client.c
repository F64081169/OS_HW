#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdlib.h>

#include "sock.h"

char leave_mesg[] = {"You are leaving the server.\n"};


int main(int argc, char **argv)
{
    int opt;
    char *server_host_name = NULL, *server_port = NULL;

    /* Parsing args */
    while ((opt = getopt(argc, argv, "h:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            server_host_name = malloc(strlen(optarg) + 1);
            strncpy(server_host_name, optarg, strlen(optarg));
            break;
        case 'p':
            server_port = malloc(strlen(optarg) + 1);
            strncpy(server_port, optarg, strlen(optarg));
            break;
        case '?':
            fprintf(stderr, "Unknown option \"-%c\"\n", isprint(optopt) ?
                    optopt : '#');
            return 0;
        }
    }

    if (!server_host_name)
    {
        fprintf(stderr, "Error!, No host name provided!\n");
        exit(1);
    }

    if (!server_port)
    {
        fprintf(stderr, "Error!, No port number provided!\n");
        exit(1);
    }

    /* Open a client socket fd */
    int clientfd __attribute__((unused)) = open_clientfd(server_host_name, server_port);
    printf("[INFO] Connected to localhost:%s\n",server_port);
    /* Start your coding client code here! */

    char message[256]= {};
    char receiveMessage[1024] = {};
    int flag = 0;

    printf("[INFO] And please type HELP for avalable cammands\n");
    while(1)
    {

        fgets(message,256,stdin);


        int count = 0;
        flag = 0;
        char mes_3 [5]= {};
        strncpy(mes_3,message,4);
        if(strcmp(mes_3,"HELP")==0 ||strcmp(mes_3,"EXIT")==0)
        {
            count = 0;
            for(int i = 0; i<256; i++)
            {
                if(count > 0)flag = 1;
                if(message[i]==' ')
                {
                    count ++;
                }
            }

        }
        else if(strcmp(mes_3,"GET ")==0 ||strcmp(mes_3,"DELE")==0)
        {
            count = 0;
            for(int i = 0; i<256; i++)
            {
                if(count > 1)flag = 1;
                if(message[i]==' ')
                {
                    count ++;
                }
            }

        }
        else if(strcmp(mes_3,"SET ")==0)
        {
            count = 0;
            for(int i = 0; i<256; i++)
            {
                if(message[i]==' ')
                {
                    count ++;
                }
            }
            if(count!=2)flag = 0;
            else flag = 1;

        }
        else
        {
            flag = 1;

        }
        if(flag==0)
        {
            send(clientfd,message,sizeof(message),0);
            recv(clientfd,receiveMessage,sizeof(receiveMessage),0);
            if(strcmp(receiveMessage,leave_mesg)==0)
            {
                printf("close Socket\n");
                close(clientfd);
                exit(0);
            }
            printf("%s",receiveMessage);


        }
        else
        {
            send(clientfd,message,sizeof(message),0);
            recv(clientfd,receiveMessage,sizeof(receiveMessage),0);
            if(strcmp(receiveMessage,leave_mesg)==0)
            {
                printf("close Socket\n");
                close(clientfd);
                exit(0);
            }
            printf("%s",receiveMessage);



        }
    }
    printf("close Socket\n");
    close(clientfd);
    return 0;
}