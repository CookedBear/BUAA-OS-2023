#include <lib.h>
#include <args.h>

void usage(void) {
	debugf("usage: history\n");
	exit();
}

int main(int argc, char **argv) {

    if (argc != 1) {
        usage();
        exit();
    }
    printf("history instruction:\n\n");
    int fd, r, line = 1;
    char temp[1], print;
    if ((fd = open("/.history", O_RDONLY)) < 0) {
        user_panic("history: %d", fd);
    }
    if ((r = read(fd, &temp, 1)) != 1) {
        printf("no history instruction.\n");
        exit();
    }
    print = temp[0];
    printf(" %4d : ", line);
    while ((r = read(fd, &temp, 1)) == 1) {
        printf("%c", print);
        if (print == '\n') {
            printf(" %4d : ", ++line);
        }
        print = temp[0];
    }
    printf("\n\ntotal instruction: %d\nhistory finished.\n\n", line);
    return 0;
}