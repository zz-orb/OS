# Note lab0

## vim

![](./img/note0.1.png)



### ����ģʽ��

H J K L--��������  W E B--�Ե���Ϊ��λ�����ƶ�

![](./img/note0.2.png)

![](./img/note0.3.png)

/<word> ������ N�����·�����shift + N���Ϸ���

:noh ��ȡ������

pճ���������е���һ�� Pճ���������е���һ��

![](./img/note0.4.png)



### ����ģʽ

���⣬���п���ģʽ�����ǿ���ѡ���ı�����в���

1. ��v���������ַ����ӻ�ģʽ��Characterwise visual mode)���ı�ѡ�������ַ�Ϊ��λ�ġ�
2. ��V (shift+v) ���������п��ӻ�ģʽ��Linewise visual mode)���ı�ѡ��������Ϊ��λ�ġ���дI�������ģʽ��
3. ��ctrl+v����Ŀ���ӻ�ģʽ��Blockwise visual mode����ʹ�ð���j/k/h/l����ѡ�ж��ж���,һ�������ڵ��ı���![](./img/note0.5.png)

## Makefile

$<:dependency����

$@:target����

*.o:���е�.o�ļ�

*.c:���е�.c�ļ�

���Makefile�����ڲ�Makefile

```shell
all: test.o funcdir
	$(MAKE) -C funcdir	//�����ڲ��ļ���
	gcc funcdir/function.o test.o -o test
.PHONY: clean
clean:
	rm -f *.o test
	$(MAKE) clean -C funcdir
```

## shell

�� ���е�$1�������

���� ���е�$1���ᱻ����

## ����

**find**

**grep**

![](./img/note0grep.png)

**sed**

**awk**

