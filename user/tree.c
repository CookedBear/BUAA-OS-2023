#include <lib.h>

void print_file(char *path) {
    int fdnum = open(path, O_RDONLY);
    struct Fd *fd = (struct Fd*) num2fd(fdnum);
    struct Filefd *ffd = (struct Filefd*) fd;

    // Find all block in the current dir.
    for (int i = 0; i < ffd->f_file.f_size / BY2BLK; i ++) {
        void *blk;
        file_get_block(path, i, &blk);
        struct File *files = (struct File *)blk;

		// Find all file in this block.
		for (struct File *f = files; f < files + FILE2BLK; ++f) {
            if (f->f_type == FTYPE_DIR) {
                print_file(f->f_name);
            } else {
                debugf("%s\n", f->f_name);
            }
        }
    }
}

int main(int argc, char **argv) {
    char *path = "/";
    print_file(path);
}