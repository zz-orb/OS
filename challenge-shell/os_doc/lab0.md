# lab0

## 思考题

### Thinking 0.1

使用add命令之前，README.txt处于`Untracked`(未跟踪的文件)；使用add命令之后，README.txt处于`Staged`(要提交的变更)；提交README.txt之后，其处于`Unmodified`；修改README.txt之后，其处于`Modified`(尚未暂存以备提交的变更)。故Modified.txt和Untracked.txt中status并不相同。

### Thinking 0.2

![](./img/Thinking%200.2.png)
add the file: `git add`

stage the file: `git add`

commit: `git commit`

### Thinking 0.3

1. 代码文件print.c 被错误删除时，应当使用什么命令将其恢复？

    `git restore print.c` 或者 `git checkout -- print.c` (将工作区恢复为暂存区的样子)
2. 代码文件 print.c 被错误删除后，执行了 `git rm print.c` 命令，此时应当使用什么命令将其恢复？

    `git rm print.c` 已将文件从暂存区删除，故先通过 `git reset HEAD print.c` 将暂存区恢复master分支的样子， 再使用 g`it restore print.c` 或者 `git checkout -- print.c`
3. 无关文件 hello.txt 已经被添加到暂存区时，如何在不删除此文件的前提下将其移出暂存区？

    `git rm hello.txt` 直接从暂存区删除文件，工作区不会做出改变

### Thinking 0.4

提交三个版本之后：
![](./img/Thinking%200.4-1.png)

回退版本二：
![](./img/Thinking%200.4-2.png)

通过hashcode回到版本三：
![](./img/Thinking%200.4-3.png)

**!!值得注意的是：git reset --hard会将工作区、暂存区和当前分支都重置到指定的提交状态，因此会更改工作区的内容。如果有未提交的更改，这些更改会被丢弃。**

### Thinking 0.5

```shell
echo first
echo second > output.txt
echo third > output.txt
echo forth >> output.txt
```
`>` :重定向命令的标准输出到文件(覆盖原文件)

`>>` :命令的输出追加到指定文件

### Thinking 0.6

command文件内容：
![](./img/Thinking%200.6-1.png)

result文件内容：

![](./img/Thinking%200.6-2.png)

解释说明:（可以从test文件的内容入手）
test给a赋值为1，给b赋值为2，给c赋值为a+b，即c的值为3；之后将c、b、a的值依次重定向输入到file1、file2、file3；接着把file1、file2、file3追加到file4；最后将file4的结果输出到result中。

+ echo echo Shell Start 与 echo 'echo Shell Start' 效果是否有区别：没有区别

+ echo echo \$c>file1 与 echo 'echo \$c>file1' 效果是否有区别 ：有区别，前者将echo \$c输出到file1中(\$c会替换为变量的值)，后者将字符串echo \$c>file1输出到标准输出($c不会被替换)。

  除此之外，“ ”中的\$1会被解释，而’ ‘中的\$1不会被解释。

## 难点分析
### Makefile编写
$<:代表第一个依赖文件

$^:代表所有的依赖文件

$@:target内容

*.o:所有的.o文件

*.c:所有的.c文件

外层Makefile调用内层Makefile

```shell
all: test.o funcdir
	$(MAKE) -C funcdir	//进入内层文件夹
	gcc funcdir/function.o test.o -o test
.PHONY: clean
clean:
	rm -f *.o test
	$(MAKE) clean -C funcdir
```

### shell中的表达式计算
- expr 算术表达式
  
    1. 运算符之间要有空格，例如：expr<空格>变量<空格>运算符<空格>变量。  `expr 2 + 3`, `c=$(expr $a + $b)`
    
    2. 乘法运算符使用 \*，乘法运算符前需要加 \。例如：`expr 2 \* 3`
-  \$[算术表达式] 及 $((算术表达式)) (无格式限制)

    ```shell
    echo $[5+9]
    
    c=$[$a+$b]
    echo $c
    
    echo $((5+9))
    
    c=$(($a/$b))
    echo $c
    ```

### shell单引号、双引号和反引号

单引号和双引号用于变量值出现空格时，比如 name=zhang san 这样执行就会出现问  题，而必须用引号括起来，比如 name="zhang san"。

+ 单引号括起来的字符都是普通字符，就算特殊字符也不再有特殊含义
+ 双引号括起来的字符中，"\$"、"\\"和反引号是拥有特殊含义的，"\$"代表引用变量的值(如果需要在双引号中间输出"$"和反引号，则要在符号前加入转义符"\\")
+ 反引号代表引用命令
  
    如果需要调用命令的输出，或把命令的输出赋予变量，则命令必须使用反引号包含，这条命令才会执行，反引号的作用和 $(命令) 是一样的
    ```(shell)
    $ echo ls
    ls
    $ echo `ls`
    code err.txt handle-ps.sh hello.c ps.out test
    ```

### sed grep awk

+ sed
	
  sed表达式可以使用单引号来引用，但是如果表达式内部包含变量字符串，就需要使用双引号。
  
  ```shell
  $ test=hello 
  $ echo hello WORLD | sed "s/$test/HELLO" 
  HELLO WORLD
  ```
  
  ```shell
  n=1
  m=1
  if [ $# -eq 2 ]
  then
          n=$1
          m=$2
  fi
  if [ $# -eq 1 ]
  then 
          n=$1
  fi
  f=$[$n+$m]
  sed -n "${f}p" err.txt >&2
  ```

## 实验体会

​	lab0的考核主要针对基础工具vim、Makefile编写、shell编写及命令行使用，难度并不在于知识本身的难度，而是对知识的掌握熟知程度。指导书并没有详细给出常用指令的所有用法，需要课下作业中查阅学习更完全更准确的用法。

## Tips
+ Ctrl+C 终止当前程序的执行

+ Ctrl+Z 挂起当前程序 (fg [job_spec]，job_spec:挂起编号，默认最近挂起进程)

+ Ctrl+D 终止输入(若正在使用shell，则退出当前 shell)

+ Ctrl+L 清屏

## References

https://c.biancheng.net/view/951.html

[Linux之sed命令详解 - zakun - 博客园 (cnblogs.com)](https://www.cnblogs.com/zakun/p/linux-cmd-sed.html)



​    e->env_status = ENV_FREE;

​         LIST_INSERT_HEAD((&env_free_list), (e), env_link);
  TAILQ_REMOVE(&env_sched_list, (e), env_sched_link);