#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

static int hisCount, curLine;
static int hisBuf[1024];

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

	if (*s == '\"') {
		debugf("parsed '\"': begin\n");
		s++;
		*p1 = s;
		debugf("parsed: ");
		while (*s && *(s++) != '\"') {
			debugf("%c", *(s - 1));
		}
		*(s - 1) = 0;
		*p2 = s;
		debugf("\nparsed '\"': end\n");
		return 'w';
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
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
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc; // parse end
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
			/* Exercise 6.5: Your code here. (1/3) */
			if ((r = open(t, O_RDONLY)) < 0) {
				user_panic("redirction_1: open file in shell failed!");
			}
			fd = r;
			dup(fd, 0);
			close(fd);

			// user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			if ((r = open(t, O_WRONLY)) < 0) {
				if ((r = touch(t)) < 0) {
					user_panic("redirction_2: create file in shell failed!");
				} else {
					if ((r = open(t, O_WRONLY)) < 0) {
						user_panic("redirction_2: open file in shell failed!");
					}
				}
			}
			fd = r;
			dup(fd, 1);
			close(fd);
			
			// user_panic("> redirection not implemented");

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
			if ((*rightpipe = fork()) == 0) { // right side
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			} else {						  // lest side
				dup(p[1], 1);
				close(p[0]);
				close(p[1]);
				return argc;
			}
			user_panic("| not implemented");

			break;
		case ';':
			if ((*rightpipe = fork()) == 0) {
				return argc; // parse end
			} else {
				debugf("parsed ';', created %x\n", *rightpipe);
				wait(*rightpipe);
				return parsecmd(argv, rightpipe);
			}
		case '&':
			if ((r = fork()) == 0) {
				return argc; // parse end
			} else {
				debugf("parsed '&', created %x\n", r);
				return parsecmd(argv, rightpipe);
			}
		}
	}

	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	struct Stat st = {0};

	if (strcmp("cd", argv[0]) == 0) {
		int r;
		char cur[1024] = {0};
		char *p = argv[1];

		if (argv[1][0] != '/') {
			if (argv[1][0] == '.') { p += 2; }

			syscall_get_rpath(cur);
			int len1 = strlen(cur);
			int len2 = strlen(p);
			if (len1 == 1) { // cur: '/'
				strcpy(cur + 1, p);
			} else {         // cur: '/a'
				cur[len1] = '/';
				strcpy(cur + len1 + 1, p);
				cur[len1 + 1 + len2] = '\0';
			}
			
		} else {
			strcpy(cur, argv[1]);
		}
		printf("cur:%s\n", cur);

		if ((r = stat(cur, &st)) < 0) { printf("4");exit(); }
		if (!st.st_isdir) {
			printf("%s is not a directory\n", cur);
			printf("5");exit();
		}
		if ((r = chdir(cur)) < 0) { printf("6");exit(); }
		return;
	}

	int child;
	if ((child = spawn(argv[0], argv)) < 0) {
		char name[1024] = {0};
		int len = strlen(argv[0]);
		strcpy(name, (const char *) argv[0]);
		name[len] = '.';
		name[len + 1] = 'b';
		name[len + 2] = '\0';
		child = spawn(name, argv);
	}
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

void dealBackSpace(char *buf[], int cur, int len) {
	for (int i = cur + 1; i <= len - 2; i++) {
		(*buf)[i] = (*buf)[i + 2];
	}

}

int readPast(int target, char *code) {
	int r, fd, spot = 0;
	char buff[10240];
	if ((fd = open("/.history", O_RDONLY)) < 0) { printf("G1");return fd; }
	for (int i = 0; i < target; i++) {
		spot += (hisBuf[i] + 1); // + '\n'
	}
	if ((r = readn(fd, buff, spot)) != spot) { printf("G2");return r; }
	if ((r = readn(fd, code, hisBuf[target])) != hisBuf[target]) { printf("G3");return r; }
	if ((r = close(fd)) < 0) { printf("G4");return r; }

	code[hisBuf[target]] = '\0';
	return 0;
}

void readline(char *buf, u_int n) {
	char curIn[1024];
	int r, len = 0;
	char temp = 0;
	for (int i = 0; len < n;) {
		if ((r = read(0, &temp, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		switch (temp) {
			case 0x7f:
				if (i <= 0) { break; } // cursor at left bottom, ignore backspace

				for (int j = (i--); j <= len - 1; j++) {
					buf[j] = buf[j + 1];
				}
				buf[--len] = 0;
				printf("\033[%dD%s \033[%dD", (i + 1), buf, (len - i + 1));
				break;
			case '\033':
				read(0, &temp, 1);
				if (temp == '[') {
					read(0, &temp, 1);
					if (temp == 'D') { // have space for left
						if (i > 0) {
							i -= 1;
						} else {
							printf("\033[C");
						}
					} else if (temp == 'C') {
						if (i < len) {
							i += 1;
						} else {
							printf("\033[D");
						}
					} else if (temp == 'A') { // up
						printf("\033[B");
						if (curLine != 0) {
							buf[len] = '\0';
							if (curLine == hisCount) {
								strcpy(curIn, buf);
							}
							if (i != 0) { printf("\033[%dD", i); }
							for (int j = 0; j < len; j++) { printf(" "); }
							if (len != 0) { printf("\033[%dD", len); }

							if ((r = readPast(--curLine, buf)) < 0) { printf("G");exit(); }
							printf("%s", buf);
							i = strlen(buf);
							len = i;
							// redirect cursor
						}
					} else if (temp == 'B') {
						buf[len] = '\0';
						if (i != 0) { printf("\033[%dD", i); }
						for (int j = 0; j < len; j++) { printf(" "); }
						if (len != 0) { printf("\033[%dD", len); }
						if (1 + curLine < hisCount) {
							if ((r = readPast(++curLine, buf)) < 0) { printf("G");exit(); }
						} else {
							strcpy(buf, curIn);
							curLine = hisCount;
						}
							printf("%s", buf);
							i = strlen(buf);
							len = i;
							// redirect cursor
						
					}
				}
				break;
			case '\r':
			case '\n':
				buf[len] = '\0';
				// printf("hisCount: %d\n", hisCount);
				if (hisCount == 0) {
					if ((r = touch("/.history")) != 0) { exit(); } 
				}
				int hisFd;
				if ((hisFd = open("/.history", O_APPEND | O_WRONLY)) < 0) { exit(); }
				if ((r = write(hisFd, buf, len)) != len) { exit(); }
				if ((r = write(hisFd, "\n", 1)) != 1) { exit(); }
				if ((r = close(hisFd)) < 0) { exit(); }
				hisBuf[hisCount++] = len;
				curLine = hisCount; // cannot 'curLine++', otherwise usable instrctions will be [0, curLine + 1]
				memset(curIn, '\0', sizeof(curIn));
				return;
			default:
				buf[len + 1] = 0;
				for (int j = len; j >= i + 1; j--) {
					buf[j] = buf[j - 1];
				}
				buf[i++] = temp;
				printf("\033[%dD%s", i, buf);
				if ((r = len++ + 1 - i) != 0) {
					printf("\033[%dD", r);
				}
			break;
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
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	char curPath[256] = {0};
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
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
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			if ((r = getcwd(curPath)) < 0) { printf("G");exit(); }
			printf("\n[%04x] %s $ ", syscall_getenvid(), curPath);
		}
		readline(buf, sizeof buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if (!(buf[0] == 'c' && buf[1] == 'd' && (buf[2] == ' ' || strlen(buf) == 2))) {
			if ((r = fork()) < 0) {
				user_panic("fork: %d", r);
			}
		} else {
			runcmd(buf);
			continue;
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
