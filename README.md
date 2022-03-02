# OS_HW

## HW1
- 學習linux kernel module，觀察並實作linux sequence proc files 讀取 系統資訊(eg. cpuinfo,meminfo...)
## HW2
- 使用noSQL的資料結構(hashed linked list) 來實作 simple kvstore ，且使用socket 與 pthread來實作server與clients(multi-threads)之間的溝通與資料的 SET,GET(search),DELETE機制。
## HW3
- 使用ucontext模擬thread scheduler
- 步驟:
 1. parsing JSON files
 2. 用struct 包住 thread 的特徵包刮目標函示(job)來模擬 threads 之間的context switch (use ucontext)
 3. 實作 signal 作為 report(每按ctrl+z列出threads狀態) 和 timer(每10ms做這個function) 的功能
 4. 實作threads API 來完成 multi level feedback queue and RR 機制
 5. 其中thread API 有 create thread, create wait event, create general event, set wait event, set general event, test cancel(觸發general event), cancel thread...
## HW4
- 實作多process之間使用memory的機制並計算page fault rate and effective access time
 1. TLB Policy : FIFO or Clock
 2. page replacement Policy : Global or Local 
 3. Disk : 無限空間，memory每踢出一個victim，index就要從小到大找到一個空的index存放victim
