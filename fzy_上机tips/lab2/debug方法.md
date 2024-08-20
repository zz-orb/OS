对于给定的页目录 pgdir，统计其包含的所有二级页表中满足以下条件的页表项：

1. 页表项有效；
2. 页表项映射的物理地址为给定的 Page *pp 对应的物理地址；
3. 页表项的权限包含给定的权限 perm_mask。

```c
u_int page_perm_stat(Pde *pgdir, struct Page *pp, u_int perm_mask) {
	int count = 0;//统计满足条件的页表项的数量
	Pde *pde;
	Pte *pte;
	for (int i = 0; i < 1024; i++) {
		pde = pgdir + i;
		if(!(*pde & PTE_V)) { //当前页目录是否有效
			continue;
		}
	
		for (int j = 0; j< 1024;j++ ){
			pte = (Pte*)KADDR(PTE_ADDR(*pde)) + j;
			if (!(*pte & PTE_V)) { ////当前页表是否有效
				continue;
			}
			if (((perm_mask | (*pte))== (*pte)) 
                && (((u_long)(page2pa(pp)))>>12) == (((u_long)(PTE_ADDR(*pte)))>>12))
				count++;
            /*该层if判断条件等价于
            (perm_mask & (*pte))== perm_mask
            (page2pa(pp) == PTE_ADDR(*pte))
            */
		}
    }
	return count;
}	


作者: YannaZhang 张杨
链接: https://yanna-zy.gitee.io/2023/04/10/BUAA-OS-2/#post-comment
来源: YannaのBlog
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
```

## debug方法

### 跳转

在命令行根目录输入：

```MAKEFILE
ctags -R *   # 获得tags文件
```

在 Vim 中跳转：打开要查看的代码文件，将光标移动到 函数名 或 结构体 上，按下 Ctrl+]，就可以跳转到 函数 或 结构体 的定义处。再按下 Ctrl+O（字母o）就可以返回跳转前的位置。

### 查找
在命令行查找：

```MAKEFILE
grep -r page_init ./   # 在当前目录中查找所有文件（包括子目录）中有关page_init的文件
```

在 Vim 中查找:

1. 按/

2. 输入搜索样式

3. 按Enter进行搜索

4. 按n搜索下一个匹配结果，或者N查找前面一个匹配结果

### debugk.h法

1. 新建 include/debugk.h :

```c
#ifndef _DBGK_H_
#define _DBGK_H_
#include <printk.h>
#define DEBUGK // 可注释

#ifdef DEBUGK
#define DEBUGK(fmt, ...) do { printk("debugk::" fmt, ##__VA_ARGS___);} while (0)
#else
#define DEBUGK(...)
#endif
#endif // !_DBGK_H_


作者: YannaZhang 张杨
链接: https://yanna-zy.gitee.io/2023/04/10/BUAA-OS-2/#post-comment
来源: YannaのBlog
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
```

2. 要想debug哪个 .c 文件就在文件前面加上 #include <debugk.h>

3. 在 assert 处想要输出的地方加上：

   ```c
   DEBUGK("ckpt%d-待填入语句\n", 数字表示是加的第几个DEBUGK);
   
   ```
4. 运行对应的 make && make run， 查看运行结果，如果卡在了哪里就能由 debugk::ckpy 看出来
5. 如果不行debug的内容输出影响对于整体程序的输出，可以注释掉 include/debugk.h 的下面这一行：

```C
/* #define DEBUGK // 可注释 */
```

### GXemul法

先笼统地看一下运行结果

```makefile
make test lab=2_1  # 编译成相应的测试数据
make objdump       # 得到 target/mos.objdump 的反汇编文件
make dbg           # 进入调试模式
r, 0               # 查看CP0寄存器的值
在target/mos.objdump中查看CP0寄存器中epc地址处的语句，向上找一下可以看到它是那个函数里面
在注释的位置帮助下，可以确定到底是哪条语句出了问题

breakpoint add [page_insert 或 0x80014560]    # 添加断点
c      # 运行到下一个断点，如果没有到断点，则说明根本不会执行到断点函数处
s [n]  # 向后执行 n 条汇编指令
unassemble  # 导出某一个地址后续（或附近）的汇编指令序列
dump [curenv 或 0x804320e8]       # 导出某一个地址后续（或附近）的内存信息
reg         # 导出所有寄存器的值
r, 0        # 导出CP0的寄存器值
tlbdump     # 导出TLB内容
trace       # 
```

作者: YannaZhang 张杨
链接: https://yanna-zy.gitee.io/2023/04/10/BUAA-OS-2/#post-comment
来源: YannaのBlog
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
