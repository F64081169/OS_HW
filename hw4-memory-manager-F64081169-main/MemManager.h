#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include<sys/types.h>


# define TLB_n 32
# define SYS_CONFIG "sys_config.txt"
# define TRACE "trace.txt"
# define OUTPUT "trace_output.txt"
# define ANALYSIS "analysis.txt"

FILE *config;
FILE *trace;
FILE *out;
FILE *analysis;

