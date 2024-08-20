// task 3: mkdir
#include <lib.h>
int flag[256];
void usage(void) {
	printf("usage: mkdir <dir>\n\
    -p: no error send if existing \n");
	exit();
}

int main(int argc, char **argv) {
    ARGBEGIN {
	case 'p':
		flag[(u_char)ARGC()]++;
		break;
	}
	ARGEND

    if(argc == 0) {
        usage();
        return -1;
    }

    int i = 0;
    int r = open(argv[i], O_RDONLY);
    if (r >= 0) {
        if (!flag['p']) {
            printf("mkdir: cannot create directory \'%s\': File exists\n", argv[i]);
        }
        close(r);
        return -1;
    } else {
        if(create(argv[i], FTYPE_DIR) < 0) {
            if (!flag['p']) {
                printf("mkdir: cannot create directory \'%s\': No such file or directory\n", argv[i]);
                return -1;
            }

            char path[1024];
            strcpy(path, argv[i]);
            for (int i = 0; path[i] != '\0'; ++i) {
                if (path[i] == '/') {
                    path[i] = '\0';
                    r = open(path, O_RDONLY);
                    if (r >= 0) {
                        close(r);
                    } else {
                        r = create(path, FTYPE_DIR);
                        if (r < 0) {
                            printf("some err\n");
                        }
                    }
                    path[i] = '/';
                } 
            }
            r = create(path, FTYPE_DIR);
            if (r < 0) {
                printf("some err\n");
            }
        }
    }
    return 0;
}