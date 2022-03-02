# HW2 Simple Key-value Store

## Directories
- /server ->	server program related sources
- /client ->	client program related sources
- /common ->	common inclusions
- /util ->	common utilization
- /build ->	target build directory

## Building the Project
Code out your `/server/server.c` and `/client/client.c`, then
```shell
$ make
```
Test your `/build/server` and `build/client`.

## Implementations
### Please briefly describe your multi-threading design pattern
[IMPORTANT!] If you want to leave server,please EXIT all the clients first!!!!\n
And press 'ctrl+c' to leave server.  

開一個陣列socked[id]其中id=50，限制最多連線50 client，
使用while迴圈來實現multi-threaded的程式，
mutex lock我只在global variable的hash table相關的function鎖住。
### Please briefly describe your data structure implementation
使用hash table搭配linked list 實作 put,get,delete機制(如簡報)
SET:先用djb2 hash function 生成 hash值，再將key value pair insert to 目標陣列hash_table[hash]。
DELETE:先算出hash 值 ，然後到目標陣列hash_table[hash]跑 linked list，如果沒找到會印出key not found訊息，有就會delete key。
GET:先算出hash 值 ，然後到目標陣列hash_table[hash]跑 linked list，如果沒找到會印出key not found訊息，有就會印出該key的value。

## References
* [POSIX thread man pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)
* [socket man pages](https://linux.die.net/man/7/socket)

