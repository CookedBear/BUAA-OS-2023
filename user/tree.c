#include <lib.h>

char buf[8192];

int main(int argc, char **argv) {
	for (int i = 0; i < argc; i++) {
		printf("%s\n", argv[i]);
	}
	if (argc != 1 && strlen(argv[1]) == 2 && argv[1][0] == '-') {
		printf("get available parameter %s!\n", argv[1]);
	} else {
		printf("run pure tree!\n");
	}
	return 0;
}
