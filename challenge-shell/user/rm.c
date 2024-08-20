// task 3: rm
#include <lib.h>
int flag[256];
void usage(void) {
	printf("usage: rm <file>\n\
    rm -r <dir>|<file>\n\
    rm -rf <dir>|<file>\n");
	return;
}

void rm(char *path) {
    struct Stat st;
    if (stat(path, &st) < 0) {
        if (!flag['f']) {
            printf("rm: cannot remove \'%s\': No such file or directory\n", path);
        }
        return;
    }
    if (st.st_isdir == 1) {
        if (flag['r']) {
            if (remove(path) < 0) {
                printf("rm: cannot remove '%s': No such file or directory\n", path);
                return;
            }
        } else {
            printf("rm: cannot remove '%s': Is a directory\n", path);
            return;
        }
    } else {
        if (remove(path) < 0) {
            printf("rm: cannot remove '%s': No such file or directory\n", path);
            return;
        }
    }
}

int main(int argc, char **argv) {
    ARGBEGIN {
    case 'r':
    case 'f':
        flag[(u_char)ARGC()]++;
        break;
	}
	ARGEND

    if(argc == 0) {
        usage();
        return -1;
    }

    u_int i;
    for (i = 0; i < argc; i++) {
        rm(argv[i]);
    }
    return 0;
}