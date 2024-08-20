1. 了解文件系统的基本概念和作用。 
2. 了解普通磁盘的基本结构和读写方式。 
3. 了解实现设备驱动的方法。 
4. 掌握并实现文件系统服务的基本操作。 
5. 了解微内核的基本设计思想和结构。

补充lab4相关：接收方进程先recv发送方进程才能成功发送

`u_int ipc_recv(u_int *whom, void *dstva, u_int *perm)`

if (whom) -> *whom为所收到消息的发送方id

if(perm) -> *perm为收到信息的权限

dstva：接受消息的地址；返回值为发送的值

`int sys_ipc_recv(u_int dstva)`

设置当前进程env_ipc_recving为1，env_ipc_dstvs为dstva，env_status为ENV_NOT_RUNNABLE并插入对应进程队列，返回成功发送/错误值



`void ipc_send(u_int whom, u_int val, const void *srcva, u_int perm) `

调用sys_ipc_try_send

`int sys_ipc_try_send(u_int envid, u_int value, u_int srcva, u_int perm)`

向envid进程发送value(srcva不为零时向接收方进程的env_ipc_dstvs插入一页)，返回成功发送/错误值

#### /kern

##### syscall_all.c

> ```c
> int sys_write_dev(u_int va, u_int pa, u_int len);
>int sys_read_dev(u_int va, u_int pa, u_int len);
> ```

#### /tools :存放的是构建时辅助工具的代码。

##### fsformat.c：创建磁盘镜像。

> ```c
> void write_directory(struct File *dirf, char *path);
> void write_file(struct File *dirf, const char *path);
> //将宿主机上路径为path的【目录（及其下所有目录和文件）/文件】写入磁盘镜像中drif所指向的文件控制块所代表的目录下
> struct File create_file(struct File *dirf);
> //在给定目录下分配新的文件控制块
> ```

  请注意，tools目录下的代码仅用于MOS的构建，在宿主Linux环境（而非 MIPS 模拟器）中运行，也不会被编译进MOS的内核、用户库或用户程序中。 

#### /fs :存放的是文件系统服务进程的代码。

##### fs.c：实现文件系统的基本功能函数

> 在文件系统服务进程中实现磁盘块与内存空间之间的映射。我们需要管理缓冲区内的内存。
> ```c
> void * disk_addr(u_int blockno);  
> //将数据块编号转换为缓冲区范围内的对应虚拟地址 
> int map_block(u_int blockno);
> //分配映射磁盘块需要的物理页面(若已经建立了映射，直接返回0)
> void unmap_block(u_int blockno);
> //释放用来映射磁盘块的物理页面(not free but dirty时写回)
> int dir_lookup(struct File *dir, char *name, struct File **file);
> //在dir指向的文件控制块所代表的目录下寻找名为name的文件。
>```

##### ide.c：通过系统调用与磁盘镜像进行交互

> ```c
> static u_int wait_ide_ready();
> void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs);
> void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs);
> /*
>  *  diskno: disk number.
>  *  secno: start sector number.
>  *  dst: destination for data read from IDE disk.
>  *  nsecs: the number of sectors to read.
>  */
> ```

##### serv.c：该进程的主干函数(文件系统服务进程的入口)

通过IPC通信与用户进程 user/lib/fsipc.c 内的通信函数进行交互。 

#### /user/lib :存放了用户程序的库函数。

##### fsipc.c：实现了与文件系统服务进程的交互(文件系统服务进程的接口)

>```c
>static int fsipc(u_int type, void *fsreq, void *dstva, u_int *perm);
>```

##### file.c：实现了文件系统的用户接口

> ```c
> int open(const char *path, int mode);
> ```

<img src="Note%20lab5/img/image-20240528225824132.png" alt="image-20240528225824132" style="zoom:50%;" /><img src="Note%20lab5/img/image-20240528230547658.png" alt="image-20240528230547658" style="zoom:50%;" />

##### fd.c：实现了文件描述符，允许用户程序使用统一的接口，抽象地操作磁盘文件系统中的文件，以及控制台和管道等虚拟的文件。

> ```c
> int read(int fdnum, void *buf, u_int n);
> ```

## 磁盘驱动

`ide.c`

![image-20240525012446641](Note%20lab5/img/image-20240525012446641.png)

> 怎么检查的IDE状态
