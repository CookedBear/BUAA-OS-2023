#include <lib.h>
#include <args.h>

int directory = 0;
int dircCount = 0;
int fileCount = 0;

void printFile(char *name, int depth, int pos, int isDir) {
    for (int i = 0 ; i < depth; i++) {
        printf("    ");
    }

    if (pos == 0) { printf("├── "); } else { printf("└── "); }

    if (isDir == 1) {
        printf("\033[0;34m%s\033[0m\n", name);
    } else {
        printf("%s\n", name);
    }
}

// pos = 0: normal
// pos = 1: last 
void dfsFile(char *path, int depth) {
    int fdnum, size, va, j, len;
    struct Fd *fd;

    if ((fdnum = open(path, O_RDONLY)) < 0) { user_panic("open %s: %d", path, fdnum); }
    fd = (struct Fd *) num2fd(fdnum);

    if (((struct Filefd *) fd)->f_file.f_type != FTYPE_DIR) {
        // printFile(((struct Filefd *) fd)->f_file.f_name, depth);
        fileCount++;
        return;
    } else {
        dircCount++;
    }

    size = ((struct Filefd *)fd)->f_file.f_size;
    va = (int) fd2data(fd);
    for (int i = 0; i < size; i += BY2FILE) {
        struct File *file = (struct File *) (va + i);
        if (file->f_name[0] == 0) { break; }

        char fullPath[MAXPATHLEN] = {0};
        strcpy(fullPath, path);
        fullPath[strlen(fullPath) + 1] = '\0';
        fullPath[strlen(fullPath)] = '/';
        len = strlen(fullPath);
        for (j = 0; j < strlen(file->f_name); j++) {
            fullPath[len + j] = file->f_name[j];
        }
        fullPath[len + j] = '\0';

        // getFullName(&path, &(file->f_name), &fullPath);
        int pos = (i == size || (file + 1)->f_name[0] == 0) ? 1 : 0;
        if (directory != 1 || file->f_type == FTYPE_DIR) {
            printFile(file->f_name, depth, pos, (file->f_type == FTYPE_DIR));
        }
        dfsFile(fullPath, depth + 1);
        // printf("%s\n", fullPath);
    }
}

void tree(char *path) {
	int r;
    struct Stat st;
    
    if ((r = stat(path, &st)) < 0) { user_panic("stat %s: %d", path, r); }
    
    if (!st.st_isdir) { user_panic("%s is not a directory!", path); }
	
    // dircnt += 1;
    printf("%s\n", path);
    dfsFile(path, 0);
    if (directory == 1) {
        printf("\n%d directories\n", dircCount);
    } else {
        printf("\n%d directories, %d files\n", dircCount, fileCount);
    }
}

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {

    ARGBEGIN {
        case 'd':
            directory = 1;
            break;
        default:
            usage();
            break;
    } ARGEND
    if (argc == 0) {
        tree("/");
    } else {
        tree(argv[0]);
    }

    return 0;
}