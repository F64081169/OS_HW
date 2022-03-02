# OS2021_Hw3_Template
* [Hw3 requirements](https://docs.google.com/presentation/d/1UFuPUwd17Hogh5Vp8GZbnrLRAddGvC1j/edit#slide=id.p3)

# 給助教看的程式執行注意事項

### 程式執行需求
* 需先安裝json-c的library  

### 執行指令

* 先remove .o檔
```
make clean
```
* 再重新make simulator
```
make simulator
```
* 執行程式
```
./simulator
```
### 可能會遇到的狀況
在執行的過程中可能會遇到function2卡random很久找不到65409，   
可以選擇再等一下，如果真的等太久(大概0.5-1分鐘)重新 ./simulator 一次就會正常。
* 我猜測的原因是while迴圈中每秒srand的值都會一樣，
所以並不會像在其他環境一樣很快就找到65409。
