#include "my_info.h"

static void *my_seq_start(struct seq_file *s, loff_t *pos)

{
    *pos = cpumask_next(*pos - 1, cpu_online_mask);
    if ((*pos) < nr_cpu_ids)
        return &cpu_data(*pos);
    return NULL;

}

static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)

{


    (*pos)++;

    return my_seq_start(s, pos);

}

static void my_seq_stop(struct seq_file *s, void *v)

{

    /* nothing to do, we use a static value in start() */

}
static int memTime_show(struct seq_file *s, void *v)
{
    //  is_otherprint = 1;
    /*time decalaration*/
    struct timespec uptime;
    struct timespec idle;
    u64 nsec;
    u32 rem;
    int num;
    /*memory decalaration struct*/
    struct sysinfo i;
    long cached;
    unsigned long pages[NR_LRU_LISTS];
    int lru;
    /*cpu declaration*/
    struct cpuinfo_x86 *c = v;
    unsigned int cpu;

    cpu = c->cpu_index;

    si_meminfo(&i);

    cached = global_node_page_state(NR_FILE_PAGES) -
             total_swapcache_pages() - i.bufferram;
    if (cached < 0)
        cached = 0;

    for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
        pages[lru] = global_node_page_state(NR_LRU_BASE + lru);

    seq_puts(s,"\n=============Memory=============\n");
    show_val_kb(s, "MemTotal       :", i.totalram);
    show_val_kb(s, "MemFree        :", i.freeram);
    show_val_kb(s, "Buffers        :", i.bufferram);
    show_val_kb(s, "Active         :", pages[LRU_ACTIVE_ANON] +
                pages[LRU_ACTIVE_FILE]);
    show_val_kb(s, "Inactive       :", pages[LRU_INACTIVE_ANON] +
                pages[LRU_INACTIVE_FILE]);
    show_val_kb(s, "Shmem          :", i.sharedram);
    show_val_kb(s, "Dirty          :",
                global_node_page_state(NR_FILE_DIRTY));
    show_val_kb(s, "Writeback      :",
                global_node_page_state(NR_WRITEBACK));
    seq_printf(s, "KernelStack    :%8lu kB\n",
               global_zone_page_state(NR_KERNEL_STACK_KB));
    show_val_kb(s, "PageTables     :",
                global_zone_page_state(NR_PAGETABLE));

    seq_puts(s,"\n=============Time=============\n");
    nsec = 0;
    for_each_possible_cpu(num)
    nsec += (__force u64) kcpustat_cpu(num).cpustat[CPUTIME_IDLE];

    get_monotonic_boottime(&uptime);
    idle.tv_sec = div_u64_rem(nsec, NSEC_PER_SEC, &rem);
    idle.tv_nsec = rem;
    seq_printf(s,"Uptime         :%lu.%02lu (s)\n",(unsigned long) uptime.tv_sec,
               (uptime.tv_nsec / (NSEC_PER_SEC / 100)));
    seq_printf(s,"Idletime       :%lu.%02lu (s)\n",(unsigned long) idle.tv_sec,
               (idle.tv_nsec / (NSEC_PER_SEC / 100)));

    seq_puts(s,"\n");
    return 0;

}
static int cpu_show(struct seq_file *s, void *v)
{

    cnt++;
    /*memory decalaration struct*/
    struct sysinfo i;
    long cached;
    unsigned long pages[NR_LRU_LISTS];
    int lru;
    /*cpu declaration*/
    struct cpuinfo_x86 *c = v;
    unsigned int cpu;

    cpu = c->cpu_index;

    si_meminfo(&i);

    cached = global_node_page_state(NR_FILE_PAGES) -
             total_swapcache_pages() - i.bufferram;
    if (cached < 0)
        cached = 0;

    for (lru = LRU_BASE; lru < NR_LRU_LISTS; lru++)
        pages[lru] = global_node_page_state(NR_LRU_BASE + lru);
    if(!is_cpuprint)
        seq_puts(s,"\n=============CPU=============\n");
    is_cpuprint = 1;
    seq_printf(s, "processor\t: %u\n",cpu);
    seq_printf(s,"model name\t: %s\n",
               c->x86_model_id[0] ? c->x86_model_id : "unknown");
    seq_printf(s, "physical id\t: %d\n", c->phys_proc_id);
    seq_printf(s, "core id\t\t: %d\n", c->cpu_core_id);
    seq_printf(s, "cpu cores\t: %d\n", c->booted_cores);
    /* Cache size */
    if (c->x86_cache_size)
        seq_printf(s, "cache size\t: %u KB\n", c->x86_cache_size);
    seq_printf(s, "clflush size\t: %u\n", c->x86_clflush_size);
    seq_printf(s, "cache_alignment\t: %d\n", c->x86_cache_alignment);
    seq_printf(s, "address sizes\t: %u bits physical, %u bits virtual\n",
               c->x86_phys_bits, c->x86_virt_bits);
    seq_puts(s,"\n");

    if(is_cpuprint && cnt == c->booted_cores)
    {
        memTime_show(s,v);
        is_cpuprint = false;
        cnt=0;
    }

    return 0;
}
static int version_show(struct seq_file *s, void *v)
{
    /*version declaration*/
    seq_puts(s,"\n=============Version=============\n");
    seq_printf(s,"Linux version %s\n",UTS_RELEASE);
    return 0;
}

static int my_seq_show(struct seq_file *s, void *v)

{

    if(!is_cpuprint)
        version_show(s,v);

    cpu_show(s,v);

    return 0;

}



/* This structure gather "function" to manage the sequence */

static struct seq_operations my_seq_ops =
{

    .start = my_seq_start,

    .next = my_seq_next,

    .stop = my_seq_stop,

    .show = my_seq_show,

};



/* This function is called when the /proc file is open. */

static int my_open(struct inode *inode, struct file *file)

{

    return seq_open(file, &my_seq_ops);

};



/* This structure gather "function" that manage the /proc file */

#ifdef HAVE_PROC_OPS

static const struct proc_ops my_file_ops =
{

    .proc_open = my_open,

    .proc_read = seq_read,

    .proc_lseek = seq_lseek,

    .proc_release = seq_release,

};

#else

static const struct file_operations my_file_ops =
{

    .open = my_open,

    .read = seq_read,

    .llseek = seq_lseek,

    .release = seq_release,

};

#endif



static int __init proc_init(void)

{

    struct proc_dir_entry *entry;



    entry = proc_create(PROC_NAME, 0, NULL, &my_file_ops);

    if (entry == NULL)
    {

        remove_proc_entry(PROC_NAME, NULL);

        pr_debug("Error: Could not initialize /proc/%s\n", PROC_NAME);

        return -ENOMEM;

    }



    return 0;

}



static void __exit proc_exit(void)

{

    remove_proc_entry(PROC_NAME, NULL);

    pr_debug("/proc/%s removed\n", PROC_NAME);

}



module_init(proc_init);

module_exit(proc_exit);



MODULE_LICENSE("GPL");