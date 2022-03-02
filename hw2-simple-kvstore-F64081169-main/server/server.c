#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>

#include "types.h"
#include "sock.h"


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_unlock(&mutex1);

//struct node
struct node
{
    char key[102];
    char val[102];
    struct node* next ;
    struct node* head ;
} node;

//hashtable to store data
struct node* hash_table[397]= {};
//pthread_t accept_thread[10];

// DJB Hash Function

int hash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c;

    return hash%397;
}


void put_node(char* key, char* value)
{
    struct node* p= malloc(sizeof(struct node));
    int insert_num = hash(key);

    strcpy(p->key,key);
    if(value!=NULL)
    {
        strcpy(p->val,value);
    }


    if(hash_table[insert_num]->next==NULL)
    {
        hash_table[insert_num]->next = p;
        hash_table[insert_num]->head = p;
        p->next=NULL;
        //    printf("%d %s\n",insert_num,p->key);
    }
    else
    {
        hash_table[insert_num]->head->next = p;
        hash_table[insert_num]->head = p;
        //  printf("%d %s\n",insert_num,p->key);
    }
    strcpy(hash_table[insert_num]->head->val,p->val);
    strcpy(hash_table[insert_num]->head->key,p->key);

}



char* get_node(char *key)
{
    int search_num = hash(key);
    struct node* p= hash_table[search_num];

    while(p->next!=NULL)
    {

        if(strcmp(p->next->key,key)==0)
        {
            //  printf("%s",p->next->val);

            return p->next->val;
        }
        p = p->next;
    }

    return "failed";
}




void putting(char input[256],int* forClientSockfd)
{
    //get separate words
    char words[3][102]= {{}};
    int count = 0;
    int e_count=0;
    int cnt = 0;
    while(input[e_count]!='\n')
    {
        if(count == 3)break;
        if(input[e_count]==' ')
        {
            words[count][cnt] = '\0';
            cnt = 0;
            count ++;

        }
        words[count][cnt] = input[e_count];
        e_count++;
        cnt++;
    }
    char m[330]= {};
    char key[102]= {};
    char value[102]= {};
    strcpy(key,words[1]);
    strcpy(value,words[2]);
    key[101] = '\0';
    value[101] = '\0';
    char m1[]= {"[OK] Key value pair ("};
    char m2[] = {","};
    char m3[] = {") is stored!\n"};
    strcat(m,m1);
    strcat(m,key);
    strcat(m,m2);
    strcat(m,value);
    strcat(m,m3);
    send(*forClientSockfd,m,sizeof(m),0);
    strcpy(m,"");
    put_node(key,value);


}

void getting(char input[256],int* forClientSockfd)
{
    //get separate words
    char words[2][102]= {{}};
    int count = 0;
    int e_count=0;
    int cnt = 0;
    while(input[e_count]!='\n')
    {
        if(count == 2)break;
        if(input[e_count]==' ')
        {
            words[count][cnt] = '\0';
            cnt = 0;
            count ++;

        }
        words[count][cnt] = input[e_count];
        e_count++;
        cnt++;
    }

    char key[102]= {};
    //  char m_get[256];
    strcpy(key,words[1]);
    char value[102]= {};

    strcpy(value,get_node(key));


    if(strcmp(value,"failed")==0)
    {
        char m[330]= {};
        strcpy(m,"[ERROR] Key does not exist!\n");
        send(*forClientSockfd,m,sizeof(m),0);
        strcpy(m,"");
    }
    else
    {
        char m[330]= {};
        key[101] = '\0';
        value[101] = '\0';
        char m1[] = {"[OK] The value of"};
        char m2[] = {" is"};
        char m3[] = {"\n"};
        strcat(m,m1);
        strcat(m,key);
        strcat(m,m2);
        strcat(m,value);
        strcat(m,m3);
        send(*forClientSockfd,m,sizeof(m),0);
        strcpy(m,"");

    }


}

void deleting(char input[256],int* forClientSockfd)
{

    //get separate words
    char words[2][102]= {{}};
    int count = 0;
    int e_count=0;
    int cnt = 0;
    while(input[e_count]!='\n')
    {
        if(count == 2)break;
        if(input[e_count]==' ')
        {
            words[count][cnt] = '\0';
            cnt = 0;
            count ++;

        }
        words[count][cnt] = input[e_count];
        e_count++;
        cnt++;
    }
    char key[102]= {};
    strcpy(key,words[1]);
//   printf("%s\n",key);
    int search_num = hash(key);
    struct node* p= hash_table[search_num];
    struct node* t = malloc(sizeof(struct node));
    int flag = 0;

    while(p->next!=NULL)
    {

        if(strcmp(p->next->key,key)==0)
        {
            flag = 1;
            if(strcmp(p->next->key,hash_table[search_num]->head->key)==0)
            {
                hash_table[search_num]->head = p;
                strcpy(hash_table[search_num]->head->val,p->val);
                strcpy(hash_table[search_num]->head->key,p->key);
                t = p->next;
                p->next = NULL;
                free(t);

            }
            else
            {
                t = p->next;
                p->next = t->next;
                free(t);
            }
            break;
        }
        p = p->next;
    }


    if(flag==0)
    {
        char mes3[] = {"[ERROR] Key does not exist! Delete failed!\n"};
        send(*forClientSockfd,mes3,sizeof(mes3),0);
        strcpy(mes3,"");
    }
    else
    {
        char m[330]= {};
        key[101] = '\0';
        char m1[] = {"[OK] Key '"};
        char m2[] = {"' is deleted!\n"};
        strcat(m,m1);
        strcat(m,key);
        strcat(m,m2);
        send(*forClientSockfd,m,sizeof(m),0);
        strcpy(m,"");
    }

}



void* service(void*args)
{

    int* forClientSockfd = (int*)args;
    char inputBuffer[256] = {};


    //threads
    while(1)
    {

        recv(*forClientSockfd,inputBuffer,sizeof(inputBuffer),0);

        char mes_3 [5]= {};
        strncpy(mes_3,inputBuffer,4);
        //if input HELP
        if(strcmp(mes_3,"HELP")==0)
        {
            int count = 0;
            for(int i = 0; i<256; i++)
            {
                if(inputBuffer[i]=='\n'||inputBuffer[i]=='\0')break;
                if(inputBuffer[i]==' ')count++;
            }
            if(count==0)
            {
                char help_mes[] = {"Commands\t\tDescription\nSET [KEY] [VAL]\t\tAdd the key-value pair into database.\nGET [KEY]\t\tGet the value of the key.\nDELETE [KEY]\t\tDelete the key-value pair from the database.\nEXIT\t\t\tExit.\n\n"};
                send(*forClientSockfd,help_mes,sizeof(help_mes),0);
                strcpy(inputBuffer,"");
            }
            else
            {
                char herr[] = {"unkwown/invalid command!\n\n"};
                send(*forClientSockfd,herr,sizeof(herr),0);
            }


        }
        else if(strcmp(mes_3,"SET ")==0)
        {

            int count = 0;
            for(int i = 0; i<256; i++)
            {
                if(inputBuffer[i]=='\n'||inputBuffer[i]=='\0')break;
                if(inputBuffer[i]==' ')count++;
            }
            if(count==2)
            {
                pthread_mutex_lock(&mutex1);
                putting(inputBuffer,forClientSockfd);
                pthread_mutex_unlock(&mutex1);
            }
            else
            {
                char perr[] = {"unkwown/invalid command!\n\n"};
                send(*forClientSockfd,perr,sizeof(perr),0);
            }


        }
        else if(strcmp(mes_3,"GET ")==0)
        {
            int count = 0;
            for(int i = 0; i<256; i++)
            {
                if(inputBuffer[i]=='\n'||inputBuffer[i]=='\0')break;
                if(inputBuffer[i]==' ')count++;
            }
            if(count==1)
            {
                pthread_mutex_lock(&mutex1);
                getting(inputBuffer,forClientSockfd);
                pthread_mutex_unlock(&mutex1);
            }
            else
            {
                char gerr[] = {"unkwown/invalid command!\n\n"};
                send(*forClientSockfd,gerr,sizeof(gerr),0);
            }

        }
        else if(strcmp(mes_3,"DELE")==0)
        {
            int count = 0;
            for(int i = 0; i<256; i++)
            {
                if(inputBuffer[i]=='\n'||inputBuffer[i]=='\0')break;
                if(inputBuffer[i]==' ')count++;
            }
            if(count==1)
            {
                pthread_mutex_lock(&mutex1);
                deleting(inputBuffer,forClientSockfd);
                pthread_mutex_unlock(&mutex1);
            }
            else
            {
                char derr[] = {"unkwown/invalid command!\n\n"};
                send(*forClientSockfd,derr,sizeof(derr),0);
            }




        }
        else if(strcmp(mes_3,"EXIT")==0)
        {
            int count = 0;
            for(int i = 0; i<256; i++)
            {
                if(inputBuffer[i]=='\n'||inputBuffer[i]=='\0')break;
                if(inputBuffer[i]==' ')count++;
            }
            if(count==0)
            {
                char mes3[] = {"You are leaving the server.\n"};
                send(*forClientSockfd,mes3,sizeof(mes3),0);



                pthread_exit(0);
            }
            else
            {
                char eerr[] = {"unkwown/invalid command!\n\n"};
                send(*forClientSockfd,eerr,sizeof(eerr),0);
            }

        }
        else
        {

            char err[] = {"unkwown/invalid command!\n\n"};
            send(*forClientSockfd,err,sizeof(err),0);


        }


    }


}

int main(int argc, char **argv)
{
    char *server_port = 0;
    int opt = 0;


    /* Parsing args */
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch (opt)
        {
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

    if (!server_port)
    {
        fprintf(stderr, "Error! No port number provided!\n");
        exit(1);
    }

    /* Open a listen socket fd */
    int listenfd __attribute__((unused)) = open_listenfd(server_port);

    /* Start coding your server code here! */

    struct sockaddr clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    pthread_t socked_id[50]= {};
    pthread_mutex_init(&mutex1, NULL);

    int flag = 0;
    if(!flag)
    {
        for(int i = 0; i<397; i++)
        {
            hash_table[i] = malloc(sizeof(struct node));
            hash_table[i]->head = NULL;
            hash_table[i]->next = NULL;
        }
        flag = 1;
    }
    printf("[INFO] Start with a clean database...\n[INFO] Initialiazing the server...\n[INFO] Server initialized!\n");
    printf("[INFO] Listening on the port %s...\n",server_port);
    int id = 0;

    while(id<50)
    {
        pthread_t t;
        socked_id[id] = accept(listenfd,(struct sockaddr*) &clientInfo, &addrlen);
        printf("[CLIENT CONNECTED] Connect to client(localhost,%d)\n",clientInfo.sa_family);
        printf("[INFO] Listening on the port %s...\n",server_port);
        pthread_create(&t,NULL,service,(void*)&socked_id[id]);

        printf("[THREAD INFO] Thread %ld created, serving connection fd%d.\n",pthread_self(),id);
        printf("\n[IMPORTANT!] If you want to leave server,please EXIT all the clients first!!!!\nAnd press 'ctrl+c' to leave server.\n");
        id++;



    }
    close(listenfd);
    return 0;
}