#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUF_LEN  80
#define N_ITER   20

int main() {
    char buf[BUF_LEN] = "Hello NetBSD!";
    int i, fd, str_len;
    
    str_len = strlen(buf);
    fd = open("/dev/rperm", O_RDWR);
    write(fd, buf, str_len);
    for (i = 0; i < N_ITER; i++) {
	read(fd, buf, BUF_LEN);
	printf("%s\n", buf);
    }
    close(fd);
    return 0;
}
