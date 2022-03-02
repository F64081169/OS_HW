#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;
ucontext_t timer_context;

thread h_level_queue;
thread m_level_queue;
thread l_level_queue;
thread terminated_queue;
thread wait_queue;
thread event_wait_queues[8];

int tid = 0;
int quantums[3] = { 100, 200, 300 };

int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode)
{
    tid++;
    thread* t = (thread*)malloc(sizeof(thread));

    strcpy(t->job_name, job_name);
    strcpy(t->b_priority, priority);
    strcpy(t->c_priority, priority);
    strcpy(t->state, "READY");
    t->tid = tid;
    t->cancel_mode = cancel_mode;
    t->canceled = 0;
    t->left_waiting_time = 0;
    t->r_time = 0;
    t->q_time = 0;
    t->w_time = 0;
    t->context = (ucontext_t*)malloc(sizeof(ucontext_t));

    if(strcmp(p_function, "Function1") == 0)
    {
        CreateContext(t->context, &dispatch_context, &Function1);
    }
    else if(strcmp(p_function, "Function2") == 0)
    {
        CreateContext(t->context, &dispatch_context, &Function2);
    }
    else if(strcmp(p_function, "Function3") == 0)
    {
        CreateContext(t->context, &dispatch_context, &Function3);
    }
    else if(strcmp(p_function, "Function4") == 0)
    {
        CreateContext(t->context, &dispatch_context, &Function4);
    }
    else if(strcmp(p_function, "Function5") == 0)
    {
        CreateContext(t->context, &dispatch_context, &Function5);
    }
    else if(strcmp(p_function, "ResourceReclaim") == 0)
    {
        CreateContext(t->context, &dispatch_context, &ResourceReclaim);
    }
    else
    {
        free(t->context);
        free(t);
        tid--;
        return -1;
    }

    if(strcmp(t->b_priority, "H") == 0)
    {
        Enque(&h_level_queue, t);
    }
    else if(strcmp(t->b_priority, "M") == 0)
    {
        Enque(&m_level_queue, t);
    }
    else if(strcmp(t->b_priority, "L") == 0)
    {
        Enque(&l_level_queue, t);
    }

    return tid;
}

void OS2021_ThreadCancel(char *job_name)
{
    thread *t = NULL;
    thread *ptr = NULL;
    thread *queues[12] =
    {
        &h_level_queue,
        &m_level_queue,
        &l_level_queue,
        &wait_queue
    };

    for(int i = 0; i < 8; i++)
    {
        queues[i + 4] = &event_wait_queues[i];
    }

    for(int i = 0; i < 12; i++)
    {
        ptr = queues[i];

        while(ptr->next)
        {
            if(strcmp(ptr->next->job_name, job_name) == 0)
            {
                t = ptr->next;
                if(t->cancel_mode == 0)
                {
                    ptr->next = ptr->next->next;
                    strcpy(t->job_name, "TERMINATED");
                    Enque(&terminated_queue, t);
                }
                else
                {
                    t->canceled = 1;
                }
            }
            else
            {
                ptr = ptr->next;
            }
        }
    }
}

void OS2021_ThreadWaitEvent(int event_id)
{
    thread* t = DispatchedThread();

    if(t)
    {
        printf("%s wants to wait for event %d\n", t->job_name, event_id);
        strcpy(t->state, "WAITING");

        if(strcmp(t->c_priority, "H") == 0)
        {
            Deque(&h_level_queue);
        }
        if(strcmp(t->c_priority, "M") == 0)
        {
            Deque(&m_level_queue);
            strcpy(t->c_priority, "H");
            printf("The priority of thread %s is changed from M to H\n", t->job_name);
        }
        else if(strcmp(t->c_priority, "L") == 0)
        {
            Deque(&l_level_queue);
            strcpy(t->c_priority, "M");
            printf("The priority of thread %s is changed from L to M\n", t->job_name);
        }

        Enque(&event_wait_queues[event_id], t);
        makecontext(&dispatch_context, Dispatcher, 0);
        swapcontext(t->context, &dispatch_context);
    }
}

void OS2021_ThreadSetEvent(int event_id)
{
    thread* t = DispatchedThread();
    thread* awakened_thread = NULL;
    thread* ptr = event_wait_queues[event_id].next;

    while(ptr)
    {
        if(awakened_thread == NULL)
        {
            awakened_thread = ptr;
        }
        else if(strcmp(ptr->c_priority, "H") == 0 && strcmp(awakened_thread->c_priority, "M") == 0)
        {
            awakened_thread = ptr;
        }
        else if(strcmp(ptr->c_priority, "H") == 0 && strcmp(awakened_thread->c_priority, "L") == 0)
        {
            awakened_thread = ptr;
        }
        else if(strcmp(ptr->c_priority, "M") && strcmp(awakened_thread->c_priority, "L") == 0)
        {
            awakened_thread = ptr;
        }

        ptr = ptr->next;
    }

    if(awakened_thread)
    {
        ptr = &event_wait_queues[event_id];
        while(ptr->next != awakened_thread)
            ptr = ptr->next;
        ptr->next = ptr->next->next;

        strcpy(awakened_thread->state, "READY");

        if(strcmp(awakened_thread->c_priority, "H") == 0)
        {
            Enque(&h_level_queue, awakened_thread);
        }
        else if(strcmp(awakened_thread->c_priority, "M") == 0)
        {
            Enque(&m_level_queue, awakened_thread);
        }
        else if(strcmp(awakened_thread->c_priority, "L") == 0)
        {
            Enque(&l_level_queue, awakened_thread);
        }

        printf("%s changes the status of %s to READY\n", t->job_name, awakened_thread->job_name);
    }
}

void OS2021_ThreadWaitTime(int msec)
{
    thread* t = DispatchedThread();

    if(t)
    {
        strcpy(t->state, "WAITING");
        t->left_waiting_time = msec * 10;

        if(strcmp(t->c_priority, "H") == 0)
        {
            Deque(&h_level_queue);
        }
        if(strcmp(t->c_priority, "M") == 0)
        {
            Deque(&m_level_queue);
            strcpy(t->c_priority, "H");
            printf("The priority of thread %s is changed from M to H\n", t->job_name);
        }
        else if(strcmp(t->c_priority, "L") == 0)
        {
            Deque(&l_level_queue);
            strcpy(t->c_priority, "M");
            printf("The priority of thread %s is changed from L to M\n", t->job_name);
        }

        Enque(&wait_queue, t);
        makecontext(&dispatch_context, Dispatcher, 0);
        swapcontext(t->context, &dispatch_context);
    }
}

void OS2021_DeallocateThreadResource()
{
    while(terminated_queue.next != NULL)
    {
        thread* t = terminated_queue.next;
        terminated_queue.next = t->next;
        printf("The memory space by %s has been released.\n", t->job_name);
        free(t->context->uc_stack.ss_sp);
        free(t);
    }
}

void OS2021_TestCancel()
{
    thread* t = DispatchedThread();

    if(t != NULL && t->canceled == 1)
    {
        strcpy(t->state, "TERMINATED");

        if(strcmp(t->c_priority, "H") == 0)
        {
            Deque(&h_level_queue);
            Enque(&terminated_queue, t);
        }
        if(strcmp(t->c_priority, "M") == 0)
        {
            Deque(&m_level_queue);
            Enque(&terminated_queue, t);
        }
        else if(strcmp(t->c_priority, "L") == 0)
        {
            Deque(&l_level_queue);
            Enque(&terminated_queue, t);
        }

        makecontext(&dispatch_context, Dispatcher, 0);
        setcontext(&dispatch_context);
    }
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context,(void (*)(void))func,0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void Dispatcher()
{
    thread* t = DispatchedThread();

    if(t)
    {
        strcpy(t->state, "RUNNING");
        swapcontext(&dispatch_context, t->context);

        strcpy(t->state, "TERMINATED");

        if(strcmp(t->c_priority, "H") == 0)
        {
            Deque(&h_level_queue);
        }
        else if(strcmp(t->c_priority, "M") == 0)
        {
            Deque(&m_level_queue);
        }
        else if(strcmp(t->c_priority, "L") == 0)
        {
            Deque(&l_level_queue);
        }

        Enque(&terminated_queue, t);
    }
    //  setcontext(&timer_context);
}

void StartSchedulingSimulation()
{
    Signaltimer.it_interval.tv_sec = 0;
    Signaltimer.it_interval.tv_usec = 10000;
    ResetTimer();

    signal(SIGALRM, Timer);
    signal(SIGTSTP, Report);

    h_level_queue.next = NULL;
    m_level_queue.next = NULL;
    l_level_queue.next = NULL;
    terminated_queue.next = NULL;
    wait_queue.next = NULL;

    for(int i = 0; i < 8; i++)
    {
        event_wait_queues[i].next = NULL;
    }

    CreateContext(&dispatch_context, &timer_context, &Dispatcher);
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);
    LoadThreads();
    swapcontext(&timer_context, &dispatch_context);
    while(1);
}

void LoadThreads()
{
    FILE *fp;
    char context[4096];

    char job_name[16];
    char p_function[16];
    char b_priority[16];
    int cancel_mode;

    fp = fopen("init_threads.json","r");
    fread(context, 4096, 1, fp);
    fclose(fp);

    struct json_object *parsed_object;
    struct json_object *arr;
    struct json_object *single_object;
    struct json_object *name;
    struct json_object *function;
    struct json_object *priority;
    struct json_object *cancel;

    parsed_object = json_tokener_parse(context);
    arr = json_object_object_get(parsed_object, "Threads");

    size_t arr_len = json_object_array_length(arr);

    for(size_t i = 0; i < arr_len; i++)
    {
        single_object = json_object_array_get_idx(arr, i);

        name = json_object_object_get(single_object, "name");
        strcpy(job_name, json_object_get_string(name));

        function = json_object_object_get(single_object, "entry function");
        strcpy(p_function, json_object_get_string(function));

        priority = json_object_object_get(single_object, "priority");
        strcpy(b_priority, json_object_get_string(priority));

        cancel = json_object_object_get(single_object, "cancel mode");
        cancel_mode = json_object_get_int(cancel);

        OS2021_ThreadCreate(job_name, p_function, b_priority, cancel_mode);
    }
}

thread* DispatchedThread()
{
    thread* t = NULL;

    if(h_level_queue.next)
    {
        t = h_level_queue.next;
    }
    else if(m_level_queue.next)
    {
        t = m_level_queue.next;
    }
    else if(l_level_queue.next)
    {
        t = l_level_queue.next;
    }
    sleep(0.0002);
    return t;
}

void Enque(thread* queue, thread* t)
{
    t->next = NULL;
    thread* ptr = queue;

    while(ptr->next)
    {
        ptr = ptr->next;
    }

    ptr->next = t;
}

void Deque(thread* queue)
{
    if(queue->next)
    {
        queue->next = queue->next->next;
    }
}

void Report()
{
    printf("\n***************************************************************************************\n");
    printf("*     %-8s %-15s %-10s %-12s %-12s %-8s %-8s *\n", "TID", "Name", "State", "B_Priority", "C_Priority", "Q_Time", "W_Time");

    thread* queues[12] =
    {
        h_level_queue.next,
        m_level_queue.next,
        l_level_queue.next,
        wait_queue.next
    };

    for(int i = 0; i < 8; i++)
    {
        queues[i + 4] = event_wait_queues[i].next;
    }

    for(int i = 0; i < 12; i++)
    {
        thread* ptr = queues[i];
        while(ptr != NULL)
        {
            printf("*     %-8d %-15s %-10s %-12s %-12s %-8d %-8d *\n", ptr->tid, ptr->job_name, ptr->state, ptr->b_priority, ptr->c_priority, ptr->q_time, ptr->w_time);
            ptr = ptr->next;
        }
    }

    printf("***************************************************************************************\n");
}

void Timer()
{
    int idle = 1;
    int out_of_time = 0;
    thread *ptr = NULL;
    thread *t = NULL;

    thread* queues[3] =
    {
        h_level_queue.next,
        m_level_queue.next,
        l_level_queue.next
    };

    for(int i = 0; i < 3; i++)
    {
        ptr = queues[i];
        while(ptr != NULL)
        {
            if(strcmp(ptr->state, "READY") == 0)
            {
                ptr->q_time += 10;
            }
            else if(strcmp(ptr->state, "RUNNING") == 0)
            {
                ptr->r_time += 10;
                if(ptr->r_time >= quantums[i])
                {
                    t = ptr;
                    out_of_time = 1;
                }
                else
                {
                    idle = 0;
                }
            }
            ptr = ptr->next;
        }
    }

    if(out_of_time)
    {
        strcpy(t->state, "READY");
        t->r_time = 0;

        if(strcmp(t->c_priority, "H") == 0)
        {
            strcpy(t->c_priority, "M");
            printf("The priority of thread %s is changed from H to M\n", t->job_name);
            Deque(&h_level_queue);
            Enque(&m_level_queue, t);
        }
        else if(strcmp(t->c_priority, "M") == 0)
        {
            strcpy(t->c_priority, "L");
            printf("The priority of thread %s is changed from M to L\n", t->job_name);
            Deque(&m_level_queue);
            Enque(&l_level_queue, t);
        }
        else if(strcmp(t->c_priority, "L") == 0)
        {
            Deque(&l_level_queue);
            Enque(&l_level_queue, t);
        }
    }

    ptr = &wait_queue;
    while(ptr->next != NULL)
    {
        ptr->next->w_time += 10;
        ptr->next->left_waiting_time -= 10;

        if(ptr->next->left_waiting_time == 0)
        {
            t = ptr->next;
            ptr->next = ptr->next->next;
            strcpy(t->state, "READY");

            if(strcmp(t->c_priority, "H") == 0)
            {
                Enque(&h_level_queue, t);
            }
            else if(strcmp(t->c_priority, "M") == 0)
            {
                Enque(&m_level_queue, t);
            }
            else if(strcmp(t->c_priority, "L") == 0)
            {
                Enque(&l_level_queue, t);
            }
        }
        else
        {
            ptr = ptr->next;
        }
    }

    for(int i = 0; i < 8; i++)
    {
        ptr = event_wait_queues[i].next;

        while(ptr)
        {
            ptr->w_time += 10;
            ptr = ptr->next;
        }
    }

    if(idle && DispatchedThread())
    {
        makecontext(&dispatch_context, Dispatcher, 0);
        setcontext(&dispatch_context);
    }
}