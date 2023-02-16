#include <blib.h>

size_t strlen(const char *s) {
	//panic("please implement");
	size_t l = 0;
	char* p = (char*)s;
	while(*p != '\0'){
		l++;
		p = p + 1;
	}
	return l;
}

char *strcpy(char *dst, const char *src) {
	//panic("please implement");
	char* p1 = dst;
	char* p2 = (char*)src;
	if(p1 == NULL || p2 == NULL){
		return NULL;
	}
	while(*p2 != '\0'){
		*p1 = *p2;
		p1++;
		p2++;	
	}
	*p1 = '\0';
	return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *res = dst;
	while (*src && n--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strcat(char *dst, const char *src) {
	//panic("please implement");
	char* p1 = dst;
	while(*p1 != '\0'){
		p1++;
	}
	char* p2 = (char*)src;
	while(*p2 != '\0'){
		*p1++ = *p2++;
	}
	*p1 = '\0';
	p1 = dst;
	return p1;
}

int strcmp(const char *s1, const char *s2) {
	//panic("please implement");
	char* s11 = (char*)s1;
	char* s12 = (char*)s2;
	int flag = 0;	
	while((flag=*s11-*s12) == 0 && *s11 != '\0'){
		s11++;
		s12++;
	}
	if(flag < 0) return -1;
	else if(flag > 0) return 1;
	else return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n--) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		if (*s1 == 0) {
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	//panic("please implement");
	if(n < 0 || s == NULL){
		return NULL;
	}
	char* p1 = s;
	while(n-- > 0){
		*p1++ = c;
	}
	return s;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in;
	char *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	//panic("please implement");
	char* s11 = (char*)s1;
	char* s12 = (char*)s2;

	int flag = *s11 - *s12;
	while(flag == 0 && --n > 0){
		s11++;
		s12++;
		flag = *s11 - *s12;
	}
	if(flag > 0) return 1;
	else if(flag < 0) return -1;
	else return 0;
}
