#include <lib.h>
#include <args.h>

void usage(void) {
	debugf("usage: touch <absolute target path>\n");
	exit();
}

int main(int argc, char **argv) {

    if (argc == 1) {
        usage();
        exit();
    } else {
        int r;
        if ((r = touch(argv[1])) < 0) {
            printf("create file fail!\n");
        } else {
            printf("created file: %s\n", argv[1]);
        }
    }

    return 0;
}