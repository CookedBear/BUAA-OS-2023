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
            char cwd[1024] = {0};
            getcwd(cwd);
            printf("created path: %s", cwd);
            if (strlen(cwd) != 1) {
                printf("/");
            }
            printf("%s\n", argv[1]);
        }
    }

    return 0;
}