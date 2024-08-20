#include <lib.h>
#define HISTORY_FILE "/.mosh_history"

int main(int argc, char** argv) {
    char buf;
    if (argc != 1)
    {
        printf("usage: history\n");
        exit();
    }
    
    int r = open(HISTORY_FILE, O_RDONLY);
    
    if (r < 0) 
    {
        printf("open /.mosh_history in err\n");
        exit();
    }
    
    int fd = r;
    while((r = read(fd, &buf, 1)) == 1)
    {
        printf("%c", buf);
    }
    close(fd);
    return 0;
}

