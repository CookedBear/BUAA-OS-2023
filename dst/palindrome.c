#include <stdio.h>
int main() {
	int nn;
	int n;
	int p = 0;
	int pp = 0;
	scanf("%d", &n);
	nn = n;
	while (n != 0) {
		p = n % 10;
		n /= 10;
		pp = (pp * 10) + p;
	}
	if (pp == nn) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}
