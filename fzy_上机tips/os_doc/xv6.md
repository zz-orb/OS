# proc.h

+ **context**:进行寄存器的清除及恢复。

​	保存了基本的数据寄存器（**s0~s11**），同时保存了返回时需要的地址**ra**及栈顶指针**sp**。

+ **cpu**
+ **trapframe**??
+ **proc**

# proc.c

+ **void proc_mapstacks**:在RAM中申请页面，并映射于虚拟地址的高处（映射）
+ **void procinit**:proc_mapstacks()分配的kstack交给各个进程
+ **int cpuid**:当前cpu编号
+ **struct cpu* mycpu**:当前cpu指针
+ **struct proc* myproc**:当前进程指针

# spinlock.c

spinlock.h 定义了struct spinlock

+ **void acquire**

**允许中断时不能持有任何锁**

1. push_off();    pop_off();

   acquire函数的最开始，会先关闭中断。如果打开中断，产生中断后运行的代码如果想获取已经获取的这把锁，就会产生死锁，因为我们无法中断返回而释放锁。

>  在XV6中，这样的场景会触发panic，因为同一个CPU会再次尝试acquire同一个锁。所以spinlock需要处理两类并发，一类是不同CPU之间的并发，一类是相同CPU上普通程序和中断程序之间的并发。针对后一种情况，我们需要在acquire中关闭中断。中断会在release的结束位置再次打开，因为在这个位置才能再次安全的接收中断。所以一个进程一旦持有锁，就不能被中断。

```c
void
acquire(struct spinlock *lk)
{
    for(;;) {
        if(!lk->locked) {
            lk->locked = 1;
            break;
        }
    }
}
```

2. while(__sync_lock_test_and_set(&lk->locked, 1) != 0)

   为了让5、6行变为原子操作， xv6 采用了386硬件上的一条特殊指令 `amoswap`。在这个原子操作中交换了内存中的一个字和一个寄存器的值。函数 `acquire`（1474）在循环中反复使用 `amoswap`；每一次都读取 `lk->locked` 然后设置为1。如果锁已经被持有了，`lk->locked` 就已经为1了，故 `amoswap` 会返回1然后继续循环。如果`amoswap`返回0，但是 `acquire` 已经成功获得了锁，即 `locked` 已经从0变为了1，这时循环可以停止了。一旦锁被获得了，`acquire` 会记录获得锁的 CPU 和栈信息，以便调试。当某个进程获得了锁却没有释放时，这些信息可以帮我们找到问题所在。当然这些信息也被锁保护着，只有在持有锁时才能修改。

3. __sync_synchronize() is 

   a *memory barrier*: it tells the compiler and CPU to not reorder loads or stores across the barrier.

+ **release**

# sleeplock.c

+ **void acquire**

获取休眠锁的过程是在自带的自旋锁的保护之下，要先获取该休眠锁的自旋锁，才能进行下一步操作