# lab1

## 思考题

### Thinking 1.1 

**Q:**请阅读附录中的编译链接详解，尝试分别使用实验环境中的原生 x86 工具链（gcc、 ld、 readelf、 objdump 等）和 MIPS 交叉编译工具链（带有 mips-linux-gnu-前缀），重复其中的编译和解析过程，观察相应的结果，并解释其中向 objdump 传入的参数的含义。  

**A:**

> 上方为只经过编译的文件的反汇编结果
>
> 下方为经过编译和链接之后的文件的反汇编结果

+ x86工具链(重复附录中的操作)

  <img src="./img/lab1-x86.png" style="zoom:10%;" />

+ MIPS交叉编译工具链

<img src="./img/lab1-mips.png" style="zoom:30%;" />

```(shell)
~/test_lab1 $ mips-linux-gnu-gcc -E hello.c > mips_1

~/test_lab1 $ mips-linux-gnu-gcc -c hello.c
~/test_lab1 $ mips-linux-gnu-objdump -DS hello.o > mips_2 

~/test_lab1 $ mips-linux-gnu-gcc -o misp.out hello.c
~/test_lab1 $ mips-linux-gnu-objdump -DS mips.out > mips_3  
```


+ objdump参数含义：

  -d, --disassemble        Display assembler contents of executable sections
  
  ​										反汇编那些特定指令机器码的section
  -D, --disassemble-all    Display assembler contents of all sections
  
  ​										反汇编所有的section
  ​    --disassemble=<sym>  Display assembler contents from <sym>
   -S, --source             Intermix source code with disassembly
  ​    --source-comment[=<txt>] Prefix lines of source code with <txt>
   -s, --full-contents      Display the full contents of all sections requested

### Thinking 1.2

**Q:**思考下述问题：

• 尝试使用我们编写的 readelf 程序，解析之前在 target 目录下生成的内核 ELF 文件。

**A:**
<img src="./img/lab1-1.png" style="zoom:33%;" />

<img src="./img/lab1-2.png" style="zoom:30%;" />
• 也许你会发现我们编写的 readelf 程序是不能解析 readelf 文件本身的，而我们刚才介绍的系统工具 readelf 则可以解析，这是为什么呢？（提示：尝试使用 readelf-h，并阅读 tools/readelf 目录下的 Makefile，观察 readelf 与 hello 的不同）

**A:**

readelf头部文件信息：

<img src="./img/lab1-readelf.png" style="zoom:33%;" />

hello头部文件信息：

<img src="./img/lab1-hello.png" style="zoom:33%;" />

系统工具readelf头部文件信息：

<img src="./img/lab1-readelfpub.png" style="zoom:33%;" />

tools/readelf/Makefile中的readelf与hello:

```shell
readelf: main.o readelf.o
		$(CC) $^ -o $@
readelf: main.o readelf.o
        $(CC) $^ -o $@
hello: hello.c
        $(CC) $^ -o $@ -m32 -static -g
```

hello在编译链接时添加了-m32 -static -g选项，使得生成的目标文件是32位、静态链接并且包含调试信息的。32位指的是指令集架构和内存地址的位数。我们可以看到hello的头部文件信息中类别为**ELF32**，而默认情况下，在64位操作系统上使用 GCC 编译器生成的可执行文件通常是64位的，故我们课程组提供的 readelf 头部文件信息中类别为**ELF64**。

> 当一个 ELF 文件的类型被标识为 ELF32 时，表示该文件采用了32位的标识符、地址和偏移等字段来描述文件的各个部分，如节表、程序头部、符号表等。 ELF64 同理。

但是在课程组提供的 readelf.c 文件中，表示 ELF 头部信息的结构体大小为32位，故课程组的 readelf 可以用来解析 hello 而不可以用来解析它自身。

如果课程组的readelf的编译方式和hello一样，即如下述形式，则readelf_temp可以解析自身。

<img src="./img/lab1-readelf_temp.png" style="zoom:33%;" />

而之所以系统工具 readelf 都能够解析，是因为 readelf 是一个通用的 ELF 文件解析工具，可以处理不同版本的 ELF 文件。当用户使用readelf命令解析一个 ELF 文件时，readelf 会根据文件的格式自动识别是 ELF32 还是 ELF64，并相应地解析文件的结构和内容。

```c
//readelf.c
int is_elf_format(const void *binary, size_t size) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)binary;
    return size >= sizeof(Elf32_Ehdr) && ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
        ehdr->e_ident[EI_MAG1] == ELFMAG1 && ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
        ehdr->e_ident[EI_MAG3] == ELFMAG3;
}
```

### Thinking 1.3

**Q:**在理论课上我们了解到， MIPS 体系结构上电时，启动入口地址为 0xBFC00000 （其实启动入口地址是根据具体型号而定的，由硬件逻辑确定，也有可能不是这个地址，但一定是一个确定的地址），但实验操作系统的内核入口并没有放在上电启动地址，而是按照内存布局图放置。思考为什么这样放置内核还能保证内核入口被正确跳转到？（提示：思考实验中启动过程的两阶段分别由谁执行。）

**A:**QEMU 模拟器支持直接加载 ELF 格式的内核，也就是说， QEMU 已经提供了 bootloader 的引导（启动）功能。 MOS 操作系统不需要再实现 bootloader的功能。在 MOS 操作系统的运行第一行代码前，我们就已经拥有一个正常的程序运行环境，内存和一些外围设备都可以正常使用。QEMU 支持加载 ELF 格式内核，所以启动流程被简化为加载内核到内存，之后跳转到内核的入口，启动就完成了。

1. Makefile 中使用的$(link_script) 来生成的内核

```shell
link_script             := kernel.lds

$(mos_elf): $(modules)
	$(LD) $(LDFLAGS) -o $(mos_elf) -N -T $(link_script) $(objects)
```

2. kernel.lds文件中有`ENTRY(_start)`这 一行命令，这就是内核的入口
3. `start.S`中sp寄存器设置到内核栈空间的位置上，随后跳转到mips_init函数(C语言的主函数)

## 难点分析

### 1.0 常用操作

```shell
objdump -DS 要反汇编的目标文件名 > 导出文本文件名 
file 要查看类型的目标文件名 #获得文件类型
grep -R printcharc #-R的意思是在多个子目录中找
```

### 1.1 ELF相关

+ **MemSiz与FileSiz(ELF头文件中段相关信息)**

> Offset:该段（segment）的数据相对于 ELF文件的偏移。
>
> VirtAddr:该段最终需要被加载到内存的哪个位置。
>
> FileSiz:该段的数据在文件中的长度。
>
> MemSiz:该段的数据在内存中所应当占的大小。

MemSiz永远大于等于FileSiz。若MemSiz大于FileSiz，则操作系统在加载程序的时候，会首先将文件中记录的数据加载到对应的VirtAddr处。之后，向内存中填0 ，直到该段在内存中的大小达到MemSiz为止。那么为什么MemSiz有时候会大于FileSiz 呢？这里举这样一个例子：C语言中未初始化的全局变量，我们需要为其分配内存，但它又不需要被初始化成特定数据。因此，在可执行文件中也只记录它需要占用内存(MemSiz)，但在文件中却没有相应的数据（因为它并不需要初始化成特定数据）。故而在这种情况下， MemSiz会大于FileSiz。这也解释了，为什么C语言中全局变量会有默认值0。这是因为操作系统在加载时将所有未初始化的全局变量所占的内存统一填了0。

+ **ELF头部标识符**

当一个 ELF 文件的类型被标识为 ELF32 时，表示该文件采用了32位的标识符、地址和偏移等字段来描述文件的各个部分，如节表、程序头部、符号表等。 ELF64 同理。

### 1.2 printk实现相关

+ **可变参数列表**

当函数参数列表末尾有省略号时，该函数即有变长的参数表。

由于需要定位变长参数表的起始位置，函数需要含有至少一个固定参数，且变长参数必须在参数表的末尾。 

stdarg.h 头文件中为处理变长参数表定义了一组宏和变量类型如下： 

va_list，变长参数表的变量类型； 

va_start(va_list ap, lastarg)，用于初始化变长参数表的宏； 

va_arg(va_list ap, 类型)，用于取变长参数表下一个参数的宏； 

va_end(va_list ap)，结束使用变长参数表的宏。

+ **回调函数**

回调函数就是一个参数，将这个函数作为参数传到另一个函数里面，当那个函数执行完之后，再执行传进去的这个函数。这个过程就叫做回调。

```c
//定义主函数，回调函数作为参数
function A(callback) {
    callback();  
    console.log('我是主函数');      
}

//定义回调函数
function B(){
    setTimeout("console.log('我是回调函数')", 3000);//模仿耗时操作  
}

//调用主函数，将函数B传进去
A(B);

//输出结果
我是主函数
我是回调函数
```

### 1.3 tmux操作

```shell
tmux	#启动
tmux set mouse on #嘿嘿启用鼠标
exit	#退出/ctrl+D
```

+ 会话

```shell
$ tmux new -s <session-name>	#新建会话
$ tmux detach					#分离会话/ctrl+b d
$ tmux ls
# or
$ tmux list-session				#查看所有会话
# 使用会话编号
$ tmux attach -t 0

# 使用会话名称
$ tmux attach -t <session-name>		#接入会话
# 使用会话编号
$ tmux kill-session -t 0

# 使用会话名称
$ tmux kill-session -t <session-name>	#杀死会话
# 使用会话编号
$ tmux switch -t 0

# 使用会话名称
$ tmux switch -t <session-name>			#切换会话
$ tmux rename-session -t 0 <new-name>	#重命名会话
```

+ 窗口

```shell
$ tmux new-window

# 新建一个指定名称的窗口
$ tmux new-window -n <window-name>				#新建窗口
# 切换到指定编号的窗口
$ tmux select-window -t <window-number>

# 切换到指定名称的窗口
$ tmux select-window -t <window-name>			#切换窗口
```

+ 窗格

```shell
# 划分上下两个窗格
$ tmux split-window

# 划分左右两个窗格
$ tmux split-window -h						#划分窗格
```



## 实验体会

lab1上机题目本身难度不大

+ lab1-exam:更改输出格式
  考察点在于读入具有变长参数列表的函数参数
+ lab1-extra:实现简化的scanf(%d %x %s %c)
  考察点在于编写具有变长参数列表的函数，练习使用回调函数。
  编写逻辑并不复杂，识别百分号后的输入控制符规定的格式→根据格式获取指针→由回调函数in从控制台读取参数→存入指针指向的内存区域。
  但是很可惜，没有通过lab1-extra。原因也很可惜，主要有两个问题，第二个十分严重的问题没有来得及在课上解决。。。
  第一个问题:判断是否有负号之后，没有再读入下一个字符。
  第二个问题:在明晃晃的注释下我又从控制台读取了下一个字符。

<img src="./img/lab1-%E5%AE%9E%E9%AA%8C.png" style="zoom:33%;" />

309行注释掉的部分是实验课上错误的实现方式，每一种case都犯了类似的错误。



希望下次会做得好一些吧

## Reference

[可能是东半球最全面易懂的 Tmux 使用教程！( 强烈建议收藏 )-腾讯云开发者社区-腾讯云 (tencent.com)](https://cloud.tencent.com/developer/article/1526675)