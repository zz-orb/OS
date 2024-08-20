#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

int need_to_send;
void save_history(char *cmd);
int lookup_history(int op, char* buf);
int history_print(int argc, char** argv);

int back_commend; 

typedef struct {
    int job_id;
    int env_id;
    int status;
    char cmd[1024];
} job_t;
#define MAXJOBS 128
#define JOB_RUNNING 1
#define JOB_DONE 0
void runcmd(char *s);
/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	if (*s == '`') {
		// 读取指令并执行
		s++;
		char cmd[1024];
		int i = 0;
    	while (*s && *s != '`') {
			cmd[i++] = *s;
        	s++;
    	}
    	if (s == 0) {
        	printf("syntax error: unmatched \"\n");
        	exit();
    	}
    	*s = 0;
		cmd[i] = 0;
		runcmd(cmd);
		//
		if (s == 0) {
			return 0;
		}

		while (strchr(WHITESPACE, *s)) {
			*s++ = 0;
		}
		if (*s == 0) {
			return 0;
		}
	}

	if (*s == '#') {
        *s = 0;  // Null-terminate the command before the comment
        return 0;
    }

	if (*s == '&' && *(s + 1) == '&') {
        *p1 = s;
        *s = 0;
		s++;
		*s = 0;
        s ++;
        *p2 = s;
        return 'a'; // 'a' stands for &&
    }

    if (*s == '|' && *(s + 1) == '|') {
        *p1 = s;
        *s = 0;
        s += 2;
        *p2 = s;
        return 'o'; // 'o' stands for ||
    }

	if (*s == '"') {
    	s++;
    	*p1 = s;
    	while (*s && *s != '"') {
        	s++;
    	}
    	if (s == 0) {
        	printf("syntax error: unmatched \"\n");
        	exit();
    	}
    	*s = 0;
		s++;
    	*p2 = s;
    	return 'w';
	}

	if (*s == '>' && *(s + 1) == '>') {
        *p1 = s;
        *s = 0;
        s += 2;
        *p2 = s;
        return 'A'; // 'A' stands for APPEND
    }
	
	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s) && !strchr("#", *s) && !strchr("`", *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		need_to_send = 0;	// 发回执行消息
		back_commend = 0;	// 是否是后台指令
		char *t;
		int fd, r;
		int left;				// 条件指令
		// int before_instr;		// 条件指令
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (1/3) */
			if ((r = open(t, O_RDONLY)) < 0) {
				debugf("syntax error: > followed the word: %s cannot open\n", t);
				exit();
			}
			fd = r;
			dup(fd, 0);
			close(fd);
			//user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, create it if not exist and trunc it if exist, dup
			// it onto fd 1, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (2/3) */
			if ((r = open(t, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
				debugf("syntax error: > followed the word: %s cannot open\n", t);
				exit();
			}
			fd = r;
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			pipe(p);
			*rightpipe = fork();
			if (*rightpipe == 0) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			}  else if (*rightpipe > 0) {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			//user_panic("| not implemented");
			break;
		case ';':;
			left = fork();
			if (left > 0) {
				wait(left);
				return parsecmd(argv, rightpipe);
			} else {
				return argc;
			}
			break;
		case 'a':  // &&
			left = fork();
			if (left > 0) {
				r = ipc_recv(0, 0, 0);
				if (r == 0) {
					return parsecmd(argv, rightpipe);
				} else {
					while (1) {
						int op = gettoken(0, &t);
						if (op == 0) {
							return 0;
						} else if (op == 'o') {
							return parsecmd(argv, rightpipe);
						}
					}
				}
			} else {
				need_to_send = 1;
				return argc;
			}
			break;
        case 'o':  // ||
			left = fork();
			if (left > 0) {
				r = ipc_recv(0, 0, 0);
				if (r != 0) {
					return parsecmd(argv, rightpipe);
				} else {
					while (1) {
						int op = gettoken(0, &t);
						if (op == 0) {
							return 0;
						} else if (op == 'a') {
							return parsecmd(argv, rightpipe);
						}
					}
				}
			} else {
				need_to_send = 1;
				return argc;
			}
			break;
		case 'A':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, create it if not exist and trunc it if exist, dup
			// it onto fd 1, and then close the original fd.
			// If the 'open' function encounters an error,
			// utilize 'debugf' to print relevant messages,
			// and subsequently terminate the process using 'exit'.
			/* Exercise 6.5: Your code here. (2/3) */
			if((fd = open(t, O_RDONLY)) < 0) {
				fd = open(t, O_CREAT);
			    if (fd < 0) {
        			debugf("error in open %s\n", t);
					exit();
				}
			}
			close(fd);

			if ((fd = open(t, O_WRONLY | O_APPEND)) < 0) {
				debugf("syntax error: >> followed the word: %s cannot open\n", t);
				exit();
			}
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");

			break;
		case '&':
        	left  = fork();
			if (left < 0) {
				printf("err in fork\n");
				exit();
			}
			if (left == 0) {
				back_commend = 1;
				return argc;
			} else {
				return parsecmd(argv, rightpipe);
			}
			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	char *cmd_ptr = s;
	char cmd[1025];
	strcpy(cmd, s);
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

	if(strcmp(argv[0] , "history") == 0) {
		history_print(argc, argv);
		if (rightpipe) {
			wait(rightpipe);
		}
		exit();
	}

	if(strcmp(argv[0] , "jobs") == 0) {
		syscall_list_jobs();
		if (rightpipe) {
			wait(rightpipe);
		}
		exit();
	}

	if(strcmp(argv[0] , "kill") == 0) { 
		int job_id = 0;
		for (int i = 0; argv[1][i]; i++) {
			job_id = 10 * job_id + argv[1][i] - '0';
		}
		syscall_kill_job(job_id);
		if (rightpipe) {
			wait(rightpipe);
		}
		exit();
	}

	if(strcmp(argv[0] , "fg") == 0) {
		int job_id = 0;
		for (int i = 0; argv[1][i]; i++) {
			job_id = 10 * job_id + argv[1][i] - '0';
		}
		int env_id = syscall_find_job_envid(job_id);
		if (env_id >= 0) {
			wait(env_id);
		}
		if (rightpipe) {
			wait(rightpipe);
		}
		exit();
	}

	char back_cmd[1024];
	if (back_commend == 1) {
		int begin_index = argv[0] - cmd_ptr;
		int end_index = 1024;
		for (int i = argv[argc - 1] - cmd_ptr + strlen(argv[argc - 1]); i < 1024; i++) {
			if (cmd[i] == '&') {
				end_index = i + 1;
				break;
			}
		}
		strcpy(back_cmd, cmd + begin_index);
		back_cmd[end_index - begin_index] = 0;
	}

	int child = spawn(argv[0], argv);
	if (back_commend == 1) {
		syscall_add_job(child, back_cmd);
	}

	close_all();
	if (child >= 0) {
		int r = ipc_recv(0, 0, 0);
		if (need_to_send) {
			ipc_send(env->env_parent_id, r, 0, 0);
		}
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

void readline(char *buf, u_int n) {
	int r;
	for (int i = 0; i < n; i++) {
		if ((r = read(0, buf + i, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}

		if (buf[i] == 27) {
			char tmp;
			buf[i] = 0;
			read(0, &tmp, 1);
			if (tmp == '[') {
				read(0, &tmp, 1);
				if (tmp == 'A') { //up
					i = lookup_history(1, buf);
				}
				if (tmp == 'B') { //down
					i = lookup_history(0, buf);
				}
			}
			i--;
		}
		
		if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (i > 0) {
				i -= 2;
			} else {
				i = -1;
			}
			if (buf[i] != '\b') {
				printf("\b");
			}
		}
		if (buf[i] == '\r' || buf[i] == '\n') {
			buf[i] = 0;
			return;
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	printf("usage: sh [-ix] [script-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	printf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	printf("::                                                         ::\n");
	printf("::                     MOS Shell 2024                      ::\n");
	printf("::                                                         ::\n");
	printf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[0], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[0], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);

		save_history(buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}

/*---------------------------HISTORY--------------------------*/
#define HISTORY_FILE "/.mosh_history"
#define HISTFILESIZE 20

char history[HISTFILESIZE][1025];
int history_count = 0;
int history_file_fd = -1;
static int current_history_index = -1;
char input_cmd[1025];
char input_flag = 0;


void save_history(char *cmd) {
    int flag_full = 0;
	if (history_count == HISTFILESIZE) {
		flag_full = 1;
        for (int i = 1; i < HISTFILESIZE; i++) {
            strcpy(history[i - 1], history[i]);
        }
        history_count--;
    }
    strcpy(history[history_count], cmd);
	history_count++;
	current_history_index = history_count - 1;
	input_flag = 0;

	if (!flag_full) {
        history_file_fd = open(HISTORY_FILE, O_WRONLY | O_CREAT | O_APPEND);
		if (history_file_fd < 0) {
			debugf("/.mosh_history open in err\n");
			return;
		}
		write(history_file_fd, cmd, strlen(cmd));
    	write(history_file_fd, "\n", 1);
		close(history_file_fd);
	} else {
		ftruncate(history_file_fd, 0);
        history_file_fd = open(HISTORY_FILE, O_WRONLY | O_CREAT);
		if (history_file_fd < 0) {
			debugf("/.mosh_history open in err\n");
			return;
		}
		for (int i = 0; i < history_count; i++) {
            write(history_file_fd, history[i], strlen(history[i]));
            write(history_file_fd, "\n", 1);
        }
		close(history_file_fd);
	}
}

int lookup_history(int op, char* buf) {
	// up:1 down:0
	int flag = 0;
	if (input_flag == 0) {
		input_flag = 1;
		flag = 1;
		memset(input_cmd, 0 ,1024);
		strcpy(input_cmd, buf);
	}
	
	int len = strlen(buf);
	for (size_t i = 0; i < len; i++) {
	 	printf("\033[D");
	 	printf(" ");
	 	printf("\033[D");
	}

	if (op == 1) {
		printf("%c[B", 27); // 不许乱动
		if (flag == 0) {
			current_history_index = current_history_index - 1 < 0 ? 0 : current_history_index - 1;
		}
		if (current_history_index != -1) {
			len = strlen(history[current_history_index]);
			strcpy(buf, history[current_history_index]);
			buf[len] = 0;
		} else {
			strcpy(buf, input_cmd);
		}
	} else {
		if (current_history_index + 1 > history_count - 1) {
			strcpy(buf, input_cmd);
		}
		else {
			current_history_index = current_history_index + 1;
			len = strlen(history[current_history_index]);
			strcpy(buf, history[current_history_index]);
			buf[len] = 0;
		}
	}
	for (int i = 0; i < len; i++) {
		printf("%c", buf[i]);
	}
	return len;
}

int history_print(int argc, char** argv) {
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