#include "function_libary.h"
//
void Function1(void)
{
    int i,j;
    while(1)
    {
        i = OS2021_ThreadCreate("random_1","Function2","L",1);
        ((i>0) ? fprintf(stdout,"Created random_1 successfully\n"):
         fprintf(stdout,"Failed to create random_1\n"));
        fflush(stdout);

        j = OS2021_ThreadCreate("random_2","Function2","L",1);
        ((j>0) ? fprintf(stdout,"Created random_2 successfully\n"):
         fprintf(stdout,"Failed to create random_2\n"));
        fflush(stdout);

        OS2021_ThreadWaitEvent(3);
        ((i>0) ? OS2021_ThreadCancel("random_1"): "");
        ((j>0) ? OS2021_ThreadCancel("random_2"): "");
        while(1);
    }
}

void Function2(void)
{
    int the_num;

    int min = 65400;
    int max = 65410;

    while(1)
    {
        srand(time(NULL));
        the_num = rand() % (max - min + 1) + min;
        if(the_num == 65409)
        {
            fprintf(stdout,"I found 65409.\n");
            fflush(stdout);
            OS2021_ThreadSetEvent(3);
            min = 0;
            max = 0;
        }
        OS2021_TestCancel();
    }
}

void Function3(void)
{
    while(1)
    {
        OS2021_ThreadWaitEvent(3);
        fprintf(stdout,"I fell in love with the operating system.\n");
        fflush(stdout);
    }
}

void Function4(void)
{

    while(1)
    {
        OS2021_ThreadWaitTime(1234);
        fprintf(stdout,"I found 65409.\n");
        fflush(stdout);
        OS2021_ThreadSetEvent(6);
        while(1);
    }
}

void Function5(void)
{
    while(1)
    {
        OS2021_ThreadWaitEvent(6);
        fprintf(stdout,"I fell in love with the operating system.\n");
        fflush(stdout);
        OS2021_ThreadWaitTime(86400000);
    }
}

void ResourceReclaim(void)
{
    while(1)
    {
        OS2021_DeallocateThreadResource();
    }
}