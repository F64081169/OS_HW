#ifndef OS2021_API_H
#define OS2021_API_H

#define STACK_SIZE 8192

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <json-c/json.h>
#include "function_libary.h"

int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode);
void OS2021_ThreadCancel(char *job_name);
void OS2021_ThreadWaitEvent(int event_id);
void OS2021_ThreadSetEvent(int event_id);
void OS2021_ThreadWaitTime(int msec);
void OS2021_DeallocateThreadResource();
void OS2021_TestCancel();


struct t
{
    char job_name[16];
    char state[16];
    char b_priority[16];
    char c_priority[16];

    int tid;
    int cancel_mode;
    int canceled;
    int left_waiting_time;
    int r_time;
    int q_time;
    int w_time;

    ucontext_t* context;
    struct t* next;
};

typedef struct t thread;

void CreateContext(ucontext_t *, ucontext_t *, void *);
void ResetTimer();
void Dispatcher();
void StartSchedulingSimulation();

void LoadThreads();
thread* DispatchedThread();
void Enque(thread*, thread*);
void Deque(thread*);
void Report();
void Timer();

#endif

