#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "getargs.h"


#define FNSIZE 80
#define MAXLEN 1000

int bufcpy(char *fn, char * contents)
{
    int fd;
    char filename[FNSIZE];
    safecpy(filename, fn, FNSIZE);
    if ((fd = open(filename, O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    } else {
        memset(contents, 0, MAXLEN);
        if (read(fd, contents, MAXLEN - 1) < 0) {
            perror("copy");
            exit(1);
        }
    }
    return fd;    
}

void mycat(char *fn)
{
    int fd;
    char contents[MAXLEN];
    fd = bufcpy(fn, contents);
    printf("%s", contents); 
    close(fd);
}

void mycp (char *so, char *de, int n)
{
    int fs, fd, m;
    char dest[FNSIZE];
    char buff[MAXLEN];
    char ans;
    safecpy(dest, de, FNSIZE);
    fs = bufcpy(so, buff);
    if ((fd = open(dest, O_WRONLY|O_CREAT|O_EXCL, 0644)) < 0) {
        if (errno != EEXIST) {
            perror("open dest file");
            exit(1);
        }
        fprintf(stderr, "overwrite (y/n)?: ");
        if ((ans = getchar()) == 'y') {
            if ((fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
                perror("open");
                exit(1);
            }
        } else {
            exit(0);
        }
    }
    for (m = 0; m < MAXLEN; m += n) {
        write(fd, buff, n);
        lseek(fd, n, SEEK_CUR);
    }   
    close(fs);
    close(fd);  
}
