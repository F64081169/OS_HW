#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define TLB_num 32
#define maxN 99999

FILE*output_file;
int r_count[30]= {};

struct node
{
    char name[100];
    int page_entry;
    struct node*next;
} typedef Node;

struct FFL
{
    int frame;
    int page;
    char process;
    struct FFL * next;
} typedef FFL;

struct TLBE
{
    int VPN;
    int PFN;
    int valid;
    struct TLBE * next;
} typedef TLBE;

struct PTE
{
    int frame;
    int dbi;
    int valid;
    int reference;
    int present;
} typedef PTE;

Node*cnode(char*name,int page_entry)
{
    Node*tmp = malloc(sizeof(Node));
    memset(tmp,0,sizeof(Node));
    strcpy(tmp->name,name);
    tmp->page_entry = page_entry;
    tmp->next = NULL;
    return tmp;
}

void addnode(Node**root,char*name,int page_entry)
{
    Node*tmp = *root;

    if(!tmp)
    {
        *root = cnode(name,page_entry);
    }
    else
    {
        while(tmp->next)
        {
            tmp = tmp->next;
        }

        tmp->next = cnode(name,page_entry);
    }
}

Node* get_trace()
{
    FILE*trace = fopen("trace.txt", "r");
    char word;
    char name[10];
    char page_entry[10];
    int namecount = 0;
    int framecount = 0;
    int flag1 = 0;
    int flag2 = 0;
    Node*root = NULL;
    memset(name, 0, sizeof(name));
    while(word = getc(trace))
    {
        if(word == '\n' || word == EOF)
        {

            addnode(&root,name,atoi(page_entry));
            namecount = 0;
            framecount = 0;
            memset(name, 0, sizeof(name));
            memset(page_entry, 0, sizeof(page_entry));
            if(word == EOF)
            {
                break;
            }
            continue;
        }
        if(word == '(')
        {
            flag1 = !flag1;
            continue;
        }
        if(word == ',')
        {
            flag1 = !flag1;
            flag2 = !flag2;
            continue;
        }
        if(word == ')')
        {
            flag2 = !flag2;
            continue;
        }
        if(flag1)
        {
            name[namecount++] = word;
        }
        if(flag2)
        {
            page_entry[framecount++] = word;
        }
    }
    fclose(trace);
    return root;
}

void get_sys_config(char*TLB_policy,char*page_policy,char*frame_policy,int*process_num,int*vir_num,int*phy_num)
{
    FILE*sys_config = fopen("sys_config.txt", "r");
    char word;
    int flag = 0;
    int counter = 0;
    int counter2 = 0;
    char input[10];
    memset(input, 0, sizeof(input));

    while(word = getc(sys_config))
    {
        if(word == ':')
        {
            flag = !flag;
            continue;
        }
        if(word == '\n' || word == EOF)
        {
            flag = !flag;

            switch(counter2)
            {
            case 0:
                strcpy(TLB_policy,input);
                counter2++;
                break;
            case 1:
                strcpy(page_policy,input);
                counter2++;
                break;
            case 2:
                strcpy(frame_policy,input);
                counter2++;
                break;
            case 3:
                *process_num = atoi(input);
                counter2++;
                break;
            case 4:
                *vir_num = atoi(input);
                counter2++;
                break;
            case 5:
                *phy_num = atoi(input);
                counter2++;
                break;
            default:
                break;
            }

            counter = 0;
            memset(input,0,sizeof(input));
            if(word == EOF)
            {
                return;
            }
            continue;
        }
        if(flag)
        {

            if(word == ' ')
            {
                continue;
            }
            input[counter++] = word;

        }

    }
    fclose(sys_config);
}

void enqueue(FFL** root, FFL* new)
{
    //not circular linked list

    FFL *tmp = *root;
    if(!tmp)
    {
        *root = new;
    }
    else
    {
        while(tmp->next)
        {
            tmp = tmp->next;
        }
        tmp->next = new;
    }
    new->next = NULL;

}

FFL*dequeue(FFL** root) //dequeue from front node
{
    FFL *tmp = *root;
    if(tmp)
    {
        *root = tmp->next;
        tmp->next = NULL;
        return tmp;
    }
    return NULL;
}

TLBE*make_TLB()
{

    TLBE*root,*tmp;
    root = tmp = malloc(sizeof(TLBE));
    memset(tmp, 0, sizeof(TLBE));
    for(int i = 1 ; i < 32 ; i++)  //create 32 TLB entry
    {

        tmp->next = malloc(sizeof(TLBE));
        tmp = tmp->next;
        memset(tmp, 0, sizeof(TLBE));

    }
    return root;

}

FFL* make_free_frame_list(int phy_num)  //create free frame list
{

    FFL*root,*tmp;
    root = tmp = malloc(sizeof(FFL));
    tmp->frame = 0;
    tmp->next = NULL;
    for(int i = 1 ; i < phy_num ; i++)
    {

        tmp->next = malloc(sizeof(FFL));
        tmp = tmp->next;
        tmp->frame = i;
        tmp->next = NULL;

    }
    return root;
}

void flush_TLB(TLBE**TLB)  //flush TLB, clear valid bit to 0
{
    TLBE*root = *TLB;
    while(root)
    {
        root->valid = 0;
        root = root->next;
    }

}

int search_TLB(TLBE**TLB,int page,char*policy)
{
    TLBE*pre,*post;
    pre = post= *TLB;

    while(post)
    {
        //not circular linked list

        if(post->valid == 1) //is in TLB
        {

            if(post->VPN == page)
            {
                //and page match


                if(strcmp(policy,"LRU") == 0) //LRU:recently access entry will put in the front
                {
                    if(post != pre)
                    {
                        pre->next = post->next;
                        post->next = *TLB;
                        *TLB = post;
                    }
                }

                return post->PFN;

            }

        }
        pre = post;
        post = post->next;
    }

    return -1;

}

void update_TLB(TLBE **TLB,int page,int frame, char *policy)
{
    TLBE *pre, *post;
    pre = post= *TLB;
    int TLB_need_to_evict_entry = 1;
    int vpn = 0;
    int x = 0;
    while(post)
    {
        if(post->valid == 0) //need be updated TLB entry's valid == 0
        {
            post->valid = 1;
            post->VPN = page;
            post->PFN = frame;

            if(strcmp(policy,"LRU") == 0)
            {
                if(post != pre) //this entry become front
                {
                    pre->next = post->next;
                    post->next = *TLB;
                    *TLB = post;
                }
            }
            TLB_need_to_evict_entry = 0;
            break;
        }
        pre = post;
        post = post->next;
    }
    if(TLB_need_to_evict_entry == 1) //this entry doesn't in TLB, need change other entry to become this entry in TLB
    {
        if(strcmp(policy,"LRU") == 0)
        {
            post = pre = *TLB;
            if(post)
            {
                while(post->next)
                {
                    pre = post;
                    post = post->next;
                }
            }
            pre->next = NULL;
            post->next = *TLB;
            *TLB = post;
            vpn = post->VPN;
            post->PFN = frame;
            post->VPN = page;
        }
        else if(strcmp(policy,"RANDOM") == 0)
        {
            srand(time(NULL));
            x = rand() % 32;
            post = *TLB;
            while(x)
            {
                post = post->next;
                x--;
            }
            vpn = post->VPN;
            post->VPN = page;
            post->PFN = frame;
        }
        return;
    }
    else
    {
        return;
    }
}

void invalid_TLB(TLBE**TLB,int page) //search specific entry and set valid bit=0
{
    TLBE*post= *TLB;
    while(post)
    {
        if(post->valid == 1)
        {
            if(post->VPN == page)
            {
                post->valid=0;
            }
        }
        post = post->next;
    }
}

int main()
{
    int BDI[maxN];
    int location=-1;
    memset(BDI,0,sizeof(BDI));
    char TLB_policy[10]; //Random,LRU
    char page_policy[10]; //fifo,clock
    char frame_policy[10]; //global local
    int process_num = 0; //how many process
    int vir_num = 0; //how many virtual page
    int phy_num = 0; //how many physical frame
    char cur_process = 0; //current process
    int block_counter = 0; //dbi

    int frame_counter = 0;
    int time_counter = 0;

    int flag = 0;
    output_file = fopen("trace_output.txt","w");

    get_sys_config(TLB_policy,page_policy,frame_policy,&process_num,&vir_num,&phy_num);

    TLBE*TLB = make_TLB();
    PTE vir[process_num][vir_num];
    FFL* free_frame_list = make_free_frame_list(phy_num);

    FFL* global_victim_page = NULL; //initially, 0 victim
    FFL* local_victim_page[process_num];

    int hit_num[process_num]; //TLB hit
    int ref_num[process_num]; //count how many trace_output.txt reference for each process
    int page_ref_num[process_num]; //reference page
    int pagefault_num[process_num]; //number of pagefault
    memset(vir,0,sizeof(vir)); //page table entry array for each process
    memset(hit_num,0,sizeof(hit_num));
    memset(ref_num,0,sizeof(ref_num));
    memset(page_ref_num,0,sizeof(page_ref_num));
    memset(pagefault_num,0,sizeof(pagefault_num));

    for(int i = 0 ; i < process_num ; i++)
    {
        local_victim_page[i] = NULL;
        for(int j = 0 ; j < vir_num ; j++)
        {
            vir[i][j].dbi = -1;
        }
    }

    Node*root = get_trace(); //root will contain all trace entry(A,B process will merge a single big linked list)

    flush_TLB(&TLB);

    while(root)
    {

        PTE *page_table = vir[root->name[0] - 'A'];
        FFL *tmp = NULL;
        int page = root->page_entry;
        int frame = 10;
        int evict_page;
        int dest; //destination dbi
        int cur_process_id = 0;
        int tmp_process_id = 0;
        int tmp_page = 0;
        char evict_process;

        time_counter++;

        if(cur_process != root->name[0])
        {
            flush_TLB(&TLB); //change process will flush TLB
        }

        cur_process = root->name[0];
        cur_process_id = cur_process - 'A';

        ref_num[cur_process_id]++; //reference TLB()

        //TLB mis
        if((frame = search_TLB(&TLB,page,TLB_policy)) == -1)
        {
            page_ref_num[cur_process_id]++; //reference page table
            //page Hit
            if(page_table[page].valid == 1 && page_table[page].present == 1)
            {
                //neither  not in disk and not in page table  nor is in disk but not in page table
                frame = page_table[page].frame;
                page_table[page].reference = 1;
                fprintf(output_file,"Process %c, TLB Miss, Page Hit, %d=>%d\n",cur_process,page,frame);
            }
            //page fault casued by not in disk and not in page table
            //in this case, maybe there has free frame
            else if(page_table[page].valid == 0)
            {
                pagefault_num[cur_process_id]++;
                if(free_frame_list) //still has free frame
                {
                    tmp = dequeue(&free_frame_list);

                    tmp->next = NULL;
                    if(strcmp(frame_policy,"GLOBAL") == 0) //global victim page
                    {
                        enqueue(&global_victim_page,tmp); //not now, but may become victim in the future
                    }
                    else if(strcmp(frame_policy,"LOCAL") == 0) //local victim page
                    {
                        enqueue(&(local_victim_page[cur_process_id]),tmp);
                    }

                    //reassign
                    frame = tmp->frame; //use same frame with free frame
                    tmp->page = page; //reassign free frame's page
                    tmp->process = cur_process;

                    //update page table
                    page_table[page].valid = 1;
                    page_table[page].reference = 1;
                    page_table[page].present = 1;
                    page_table[page].frame = frame;
                    fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict -1 of Process %c to -1, %d<<-1\n",cur_process,frame,cur_process,page);
                }
                else //don't have free frame
                {
                    if(flag == 0) //to make victim page become circulry linked list(when first time do not have free frame will go here)
                    {
                        if(strcmp(frame_policy,"GLOBAL") == 0)//circular linked list
                        {
                            tmp = global_victim_page;
                            if(tmp)
                            {
                                while(tmp->next)
                                {
                                    tmp = tmp->next;
                                }
                            }
                            tmp->next = global_victim_page;
                        }
                        else if(strcmp(frame_policy,"LOCAL") == 0)
                        {
                            for(int i = 0 ; i < process_num ; i++)
                            {
                                tmp = local_victim_page[i];
                                if(tmp)
                                {
                                    while(tmp->next)
                                    {
                                        tmp = tmp->next;
                                    }
                                }
                                tmp->next = local_victim_page[i];
                            }
                        }
                    }
                    flag = 1;
                    if(strcmp(page_policy,"FIFO") == 0)
                    {
                        if(strcmp(frame_policy,"GLOBAL") == 0)
                        {
                            tmp = global_victim_page;
                            global_victim_page = global_victim_page->next;

                            tmp_process_id = tmp->process - 'A'; //evicted which process
                            tmp_page = tmp->page;

                            frame = tmp->frame;

                            vir[tmp_process_id][tmp_page].present = 0;
                            vir[tmp_process_id][tmp_page].reference = 0;
                            // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                            // {
                            //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                            // }


                            for(int i=0; i<maxN; i++)
                            {
                                if(BDI[i]==0)
                                {
                                    location=i;
                                    BDI[i]=1;
                                    break;
                                }
                            }
                            vir[tmp_process_id][tmp_page].dbi = location;


                            evict_page = tmp_page;
                            evict_process = tmp->process;
                            dest = vir[tmp_process_id][tmp_page].dbi;

                            if(tmp->process == cur_process)
                            {
                                invalid_TLB(&TLB,page);
                            }

                            //reassign tmp to use
                            tmp->process = cur_process;
                            tmp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);

                        }

                        else if(strcmp(frame_policy,"LOCAL") == 0)
                        {

                            tmp = local_victim_page[cur_process_id];
                            local_victim_page[cur_process_id] = local_victim_page[cur_process_id]->next;

                            tmp_process_id = tmp->process - 'A';
                            tmp_page = tmp->page;

                            frame = tmp->frame;

                            vir[tmp_process_id][tmp_page].present = 0;
                            vir[tmp_process_id][tmp_page].reference = 0;
                            // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                            // {
                            //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                            // }


                            for(int i=0; i<maxN; i++)
                            {
                                if(BDI[i]==0)
                                {
                                    location=i;
                                    BDI[i]=1;
                                    break;
                                }
                            }
                            vir[tmp_process_id][tmp_page].dbi = location;

                            evict_page = tmp_page;
                            evict_process = tmp->process;
                            dest = vir[tmp_process_id][tmp_page].dbi;

                            if(tmp->process == cur_process)
                            {
                                invalid_TLB(&TLB,evict_page);
                            }

                            tmp->process = cur_process;
                            tmp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);
                        }

                    }
                    else if(strcmp(page_policy,"CLOCK") == 0)
                    {
                        if(strcmp(frame_policy,"GLOBAL") == 0)
                        {
                            while(vir[global_victim_page->process - 'A'][global_victim_page->page].reference)
                            {
                                vir[global_victim_page->process - 'A'][global_victim_page->page].reference = 0;
                                global_victim_page = global_victim_page->next;
                            }
                            tmp = global_victim_page;
                            global_victim_page = global_victim_page->next;

                            tmp_process_id = tmp->process - 'A';
                            tmp_page = tmp->page;

                            frame = tmp->frame;

                            vir[tmp_process_id][tmp_page].present = 0;
                            vir[tmp_process_id][tmp_page].reference = 0;
                            // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                            // {
                            //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                            // }


                            for(int i=0; i<maxN; i++)
                            {
                                if(BDI[i]==0)
                                {
                                    location=i;
                                    BDI[i]=1;
                                    break;
                                }
                            }
                            vir[tmp_process_id][tmp_page].dbi = location;

                            evict_page = tmp_page;
                            evict_process = tmp->process;
                            dest = vir[tmp_process_id][tmp_page].dbi;

                            if(tmp->process == cur_process)
                            {
                                invalid_TLB(&TLB,page);
                            }

                            tmp->process = cur_process;
                            tmp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);
                        }

                        else if(strcmp(frame_policy,"LOCAL") == 0)
                        {
                            while(vir[local_victim_page[cur_process_id]->process - 'A'][local_victim_page[cur_process_id]->page].reference)
                            {
                                vir[local_victim_page[cur_process_id]->process - 'A'][local_victim_page[cur_process_id]->page].reference = 0;
                                local_victim_page[cur_process_id] = local_victim_page[cur_process_id]->next;
                            }
                            tmp = local_victim_page[cur_process_id];
                            local_victim_page[cur_process_id] = local_victim_page[cur_process_id]->next;

                            tmp_process_id = tmp->process - 'A';
                            tmp_page = tmp->page;

                            frame = tmp->frame;

                            vir[tmp_process_id][tmp_page].present = 0;
                            vir[tmp_process_id][tmp_page].reference = 0;
                            // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                            // {
                            //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                            // }


                            for(int i=0; i<maxN; i++)
                            {
                                if(BDI[i]==0)
                                {
                                    location=i;
                                    BDI[i]=1;
                                    break;
                                }
                            }
                            vir[tmp_process_id][tmp_page].dbi = location;

                            evict_page = tmp_page;
                            evict_process = tmp->process;
                            dest = vir[tmp_process_id][tmp_page].dbi;

                            if(tmp->process == cur_process)
                            {
                                //change
                                invalid_TLB(&TLB,evict_page);
                            }

                            tmp->process = cur_process;
                            tmp->page = page;

                            page_table[page].valid = 1;
                            page_table[page].reference = 1;
                            page_table[page].present = 1;
                            page_table[page].frame = frame;
                            fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<-1\n",cur_process,frame,evict_page,evict_process,dest,page);
                        }
                    }
                }
            }
            //page fault casue by page in disk
            //in this case, must not have free frame
            else if(page_table[page].present == 0)
            {
                pagefault_num[cur_process_id]++;
                if(strcmp(page_policy,"FIFO") == 0)
                {
                    if(strcmp(frame_policy,"GLOBAL") == 0)
                    {
                        tmp = global_victim_page;
                        global_victim_page = global_victim_page->next;

                        tmp_process_id = tmp->process - 'A';
                        tmp_page = tmp->page;

                        frame = tmp->frame;

                        vir[tmp_process_id][tmp_page].present = 0;
                        vir[tmp_process_id][tmp_page].reference = 0;
                        // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                        // {
                        //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                        // }


                        for(int i=0; i<maxN; i++)
                        {
                            if(BDI[i]==0)
                            {
                                location=i;
                                BDI[i]=1;
                                break;
                            }
                        }
                        vir[tmp_process_id][tmp_page].dbi = location;

                        evict_page = tmp_page;
                        evict_process = tmp->process;
                        dest = vir[tmp_process_id][tmp_page].dbi;

                        if(tmp->process == cur_process)
                        {
                            invalid_TLB(&TLB,page);
                        }

                        tmp->process = cur_process;
                        tmp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;


                        if(page_table[page].dbi>-1)
                            BDI[page_table[page].dbi]=0;


                        fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                    }


                    else if(strcmp(frame_policy,"LOCAL") == 0)
                    {

                        tmp = local_victim_page[cur_process_id];
                        local_victim_page[cur_process_id] = local_victim_page[cur_process_id]->next;

                        tmp_process_id = tmp->process - 'A';
                        tmp_page = tmp->page;

                        frame = tmp->frame;

                        vir[tmp_process_id][tmp_page].present = 0;
                        vir[tmp_process_id][tmp_page].reference = 0;


                        for(int i=0; i<maxN; i++)
                        {
                            if(BDI[i]==0)
                            {
                                location=i;
                                BDI[i]=1;
                                break;
                            }
                        }
                        vir[tmp_process_id][tmp_page].dbi = location;

                        evict_page = tmp_page;
                        evict_process = tmp->process;
                        dest = vir[tmp_process_id][tmp_page].dbi;

                        if(tmp->process == cur_process)
                        {
                            invalid_TLB(&TLB,evict_page);
                        }

                        tmp->process = cur_process;
                        tmp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;

                        if(page_table[page].dbi>-1)
                            BDI[page_table[page].dbi]=0;


                        fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                    }
                }
                else if(strcmp(page_policy,"CLOCK") == 0)
                {
                    if(strcmp(frame_policy,"GLOBAL") == 0)
                    {
                        while(vir[global_victim_page->process - 'A'][global_victim_page->page].reference)
                        {
                            vir[global_victim_page->process - 'A'][global_victim_page->page].reference = 0;
                            global_victim_page = global_victim_page->next;
                        }
                        tmp = global_victim_page;
                        global_victim_page = global_victim_page->next;

                        tmp_process_id = tmp->process - 'A';
                        tmp_page = tmp->page;

                        frame = tmp->frame;

                        vir[tmp_process_id][tmp_page].present = 0;
                        vir[tmp_process_id][tmp_page].reference = 0;
                        // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                        // {
                        //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                        // }


                        for(int i=0; i<maxN; i++)
                        {
                            if(BDI[i]==0)
                            {
                                location=i;
                                BDI[i]=1;
                                break;
                            }
                        }
                        vir[tmp_process_id][tmp_page].dbi = location;

                        evict_page = tmp_page;
                        evict_process = tmp->process;
                        dest = vir[tmp_process_id][tmp_page].dbi;

                        if(tmp->process == cur_process)
                        {
                            invalid_TLB(&TLB,page);
                        }

                        tmp->process = cur_process;
                        tmp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;

                        if(page_table[page].dbi>-1)
                            BDI[page_table[page].dbi]=0;


                        fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                    }

                    else if(strcmp(frame_policy,"LOCAL") == 0)
                    {
                        while(vir[local_victim_page[cur_process_id]->process - 'A'][local_victim_page[cur_process_id]->page].reference)
                        {
                            vir[local_victim_page[cur_process_id]->process - 'A'][local_victim_page[cur_process_id]->page].reference = 0;
                            local_victim_page[cur_process_id] = local_victim_page[cur_process_id]->next;
                        }
                        tmp = local_victim_page[cur_process_id];
                        local_victim_page[cur_process_id] = local_victim_page[cur_process_id]->next;

                        tmp_process_id = tmp->process - 'A';
                        tmp_page = tmp->page;

                        frame = tmp->frame;

                        vir[tmp_process_id][tmp_page].present = 0;
                        vir[tmp_process_id][tmp_page].reference = 0;
                        // if(vir[tmp_process_id][tmp_page].dbi == -1) //no free frame, so need to kick to disk
                        // {
                        //     vir[tmp_process_id][tmp_page].dbi = block_counter++;
                        // }


                        for(int i=0; i<maxN; i++)
                        {
                            if(BDI[i]==0)
                            {
                                location=i;
                                BDI[i]=1;
                                break;
                            }
                        }
                        vir[tmp_process_id][tmp_page].dbi = location;

                        evict_page = tmp_page;
                        evict_process = tmp->process;
                        dest = vir[tmp_process_id][tmp_page].dbi;

                        if(tmp->process == cur_process)
                        {

                            invalid_TLB(&TLB, evict_page);

                        }

                        tmp->process = cur_process;
                        tmp->page = page;

                        page_table[page].valid = 1;
                        page_table[page].reference = 1;
                        page_table[page].present = 1;
                        page_table[page].frame = frame;

                        if(page_table[page].dbi>-1)
                            BDI[page_table[page].dbi]=0;


                        fprintf(output_file,"Process %c, TLB Miss, Page Fault, %d, Evict %d of Process %c to %d, %d<<%d\n",cur_process,frame,evict_page,evict_process,dest,page,page_table[page].dbi);
                    }
                }
            }
            update_TLB(&TLB,page,frame,TLB_policy);
        }
        //TLB hit
        else
        {
            r_count[cur_process_id]++;
            hit_num[cur_process_id]++;
            fprintf(output_file,"Process %c, TLB Hit, %d=>%d\n",root->name[0],page,frame);
            page_table[page].reference = 1;
            root = root->next; //so if TLB miss, will search TLB again after lots of operation
        }
    }

    FILE *fp = fopen("analysis.txt","w");
    int cnt = 0;
    for(int i = 0 ; i < process_num ; i++)
    {
        if(cnt!=0)
        {
            fprintf(fp,"\n");
        }
        double hit_rate = ((double)hit_num[i])/((double)ref_num[i]);
        //hit_num:TLB hit
        //ref_num:count how many trace_output.txt reference for each process
        fprintf(fp,"Process %c, Effective Access Time = %.3f\n",i+'A',(hit_rate*120 + (1-hit_rate)*220));

        double pagefault_rate = ((double)pagefault_num[i])/((double)r_count[i]);
        //r_count : TLB reference
        fprintf(fp,"Process %c, Page Fault Rate = %.3f",i+'A',pagefault_rate);
        cnt++;
    }

    fclose(fp);
    fclose(output_file);
    return 0;
}