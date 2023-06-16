#include <lib.h>
#include <args.h>

void usage(void) {
	debugf("usage: mkdir <absolute target path>\n");
	exit();
}

int main(int argc, char **argv) {

    if (argc == 1) {
        usage();
        exit();
    } else {
        int r;
        if ((r = mkdir(argv[1])) < 0) {
            printf("create path fail!\n");
        } else {
            printf("created path: ");
            printf("%s\n", argv[1]);
        }
    }

    return 0;
}