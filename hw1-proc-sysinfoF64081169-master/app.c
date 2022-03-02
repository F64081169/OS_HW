#include <stdio.h>
#include <stdlib.h>

#define MAX_BUF_SIZE 512

void execution();
void print_file(int,int);

int main(void)
{

    while(1)
    {

        printf("Which information do you want?\n");
        printf("Version(v),CPU(c),Memory(m),Time(t),All(a),Exit(e)?\n");
        execution();
    }

    return 0;

}

void execution()
{
    char input;
    scanf("%c",&input);

    switch(input)
    {
    case 'v':
        printf("Version：");
        print_file(1,1);
        printf("\n----------------------------------------\n");
        break;
    case 'c':
        printf("CPU information：\n");
        print_file(2,2);
        printf("\n----------------------------------------\n");
        break;
    case 'm':
        printf("Memory information：\n");
        print_file(3,3);
        printf("\n----------------------------------------\n");
        break;
    case 't':
        printf("Time information：\n");
        print_file(4,4);
        printf("----------------------------------------\n");
        break;
    case 'a':
        print_file(0,30);
        break;
    case 'e':
        exit(0);
        break;
    default:
        execution();
        break;
    }
}

void print_file(int start,int end)
{
    FILE *fp = fopen("/proc/my_info", "r");
    int count = 0;
    int ignore = 0;
    if(!fp)
        puts("Proc file doesn't exist");
    else
    {
        char buf[MAX_BUF_SIZE];
        while(fgets(buf, sizeof(buf), fp))
        {
            if(buf[0]=='=')count ++;
            if(count>=start && count<=end)
            {
                if(ignore==1)
                    printf("%s", buf);
                ignore = 1;
            }
        }
        fclose(fp);
        fp = NULL;
    }

}