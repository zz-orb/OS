// task 3: touch
#include <lib.h>

void usage(void) {
	printf("usage: touch [filename]\n");
	exit();
}

int main(int argc, char **argv) {
    if(argc != 2) {
        usage();
        return -1;
    }

    int r = open(argv[1], O_RDONLY); // 打开文件
    if(r >= 0) {
        close(r);
        return 0;
    } else { // 不存在则创建
        if(create(argv[1], FTYPE_REG) < 0) {
            printf("touch: cannot touch \'%s\': No such file or directory\n", argv[1]);
            return -1;
        }
        return 0;
    }
}