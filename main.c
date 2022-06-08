#include "file.h"
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#define FNMAX 100
#define PIPEMAX 10
#define PATHMAX 10
#define BGMAX 5
int shell(int, char **, char);
void findpath(char *);
void sigch_handler();
void sigint_handler();
char iofile[FNMAX];
int bgp, chpid = 0, ppid = 0;
char bgcmd[BGMAX][FNMAX];
char PATH[FNMAX];
extern char **environ;
int main()
{
    int fd = 0, argc = 0, n = 0, m = 0, cmd = 0, k = 0, fp = -1, lp = 0, pc = 0, l = 0, flag = 0, pid = 0, pipenum = 0, bgindex = -1, pgid = 0, fgpgid = 0;
    char *argv[COMMANDMAX], mode = 0, tmp = 0, nextmode = 0;
    char **p, pipebuf[PIPEMAX];
    int pfd[PIPEMAX][2], stat1, stat2, indexbuf[PIPEMAX], stat[PIPEMAX], rfd[2];
    ppid = getpid();

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigint_handler);
    signal(SIGTTOU, SIG_IGN);

    while (1)
    {
        argc = 0, m = 0, stat1 = 0, stat2 = 0, fp = -1, lp = 0, pc = 0, pid = 0, pipenum = 0, bgindex = -1;
        memset(iofile, 0, FNMAX);
        for (k = 0; k < PIPEMAX; k++)
        {
            pipebuf[k] = 0;
            indexbuf[k] = 0;
        }
        printf("mysh $ ");
        fflush(stdout);
        getargs(&argc, argv);
        for (n = 0; n < argc; n++)
        {
            tmp = argv[n][0];
            if (tmp == '|' || tmp == '<' || tmp == '>')
            {

                if (tmp == '|')
                {
                    if (fp < 0)
                    {
                        fp = m;
                    }
                    lp = m;
                    pc++;
                }
                pipebuf[m] = tmp;
                indexbuf[m] = n;
                m++;
            }
            if (tmp == '&')
            {
                bgindex = n;
            }
        }

        if (bgindex > -1)
        {
            safecpy(bgcmd[bgp], argv[0], FNMAX);
            bgp++;
            signal(SIGCHLD, sigch_handler);
            if ((chpid = fork()) < 0)
            {
                perror("pid");
            }
            else if (chpid == 0)
            {
                signal(SIGTTOU, SIG_IGN);
                argv[bgindex] = NULL;
                chpid = getpid();
                setpgid(chpid, chpid);
                findpath(argv[0]);
                execve(PATH, argv, environ);
                exit(0);
            }
            else
            {
                printf("[%d] %d\n", bgp, chpid);
            }
        }

        if (m != 0)
        {
            for (l = 0; l < pc; l++)
            {
                if ((flag = pipe(pfd[l])) == -1)
                {
                    perror("pipe");
                }
            }
            l = 0;
            for (k = 0; k < m; k++)
            {
                mode = pipebuf[k];
                if (k < m - 1)
                {
                    nextmode = pipebuf[k + 1];
                }
                cmd = indexbuf[k];
                if (mode == '|')
                {
                    if (k == fp)
                    {
                        if ((pid = fork()) < 0)
                        {
                            perror("pid");
                        }
                        else if (pid == 0)
                        {
                            pid = getpid();
                            setpgid(pid, pid);
                            if (k > 0)
                            {
                                p = argv + indexbuf[k - 1] + 2;
                                dup2(rfd[0], 0);
                                close(rfd[1]);
                            }
                            else
                            {
                                p = argv;
                            }
                            argv[indexbuf[k]] = NULL;

                            dup2(pfd[0][1], 1);
                            close(pfd[0][0]);
                            close(pfd[0][1]);
                            findpath(p[0]);
                            execve(PATH, p, environ);
                            exit(0);
                        }
                        else
                        {
                            if (k > 0)
                            {
                                close(rfd[0]);
                                close(rfd[1]);
                            }
                            l++;
                            pgid = pid;
                            if ((fd = open("/dev/tty", O_RDWR)) < 0)
                            {
                                perror("file");
                            }
                            tcsetpgrp(fd, pgid);
                            close(fd);
                        }
                    }
                    if (k != lp)
                    {
                        if ((pid = fork()) < 0)
                        {
                            perror("pid");
                        }
                        else if (pid == 0)
                        {
                            pid = getpid();
                            setpgid(pid, pgid);
                            p = argv + indexbuf[k] + 1;
                            argv[indexbuf[k + 1]] = NULL;
                            dup2(pfd[l - 1][0], 0);
                            dup2(pfd[l][1], 1);
                            close(pfd[l - 1][0]);
                            close(pfd[l - 1][1]);
                            close(pfd[l][0]);
                            close(pfd[l][1]);
                            findpath(p[0]);
                            execve(PATH, p, environ);
                            exit(0);
                        }
                        else
                        {
                            close(pfd[l - 1][0]);
                            close(pfd[l - 1][1]);
                            l++;
                        }
                    }
                    if (k == lp)
                    {
                        if ((m - 1) > k && (indexbuf[k + 1] + 1) == argc)
                        {
                            fprintf(stderr, "mysh: no redirectfile\n");
                        }

                        if ((pid = fork()) < 0)
                        {
                            perror("pid");
                        }

                        else if (pid == 0)
                        {
                            pid = getpid();
                            setpgid(pid, pgid);
                            p = argv + indexbuf[k] + 1;
                            dup2(pfd[pc - 1][0], 0);
                            close(pfd[pc - 1][0]);
                            close(pfd[pc - 1][1]);
                            if ((m - 1) > k)
                            {
                                argv[indexbuf[k + 1]] = NULL;
                                if ((indexbuf[k + 1] + 1) != argc)
                                {
                                    safecpy(iofile, argv[indexbuf[k + 1] + 1], FNMAX);
                                    myredirect(nextmode, iofile);
                                }
                                else
                                {
                                    exit(1);
                                }
                            }
                            else
                            {
                                argv[argc] = NULL;
                            }
                            findpath(p[0]);
                            execve(PATH, p, environ);
                            exit(0);
                        }
                        else
                        {
                            close(pfd[pc - 1][0]);
                            close(pfd[pc - 1][1]);
                        }
                    }
                }
                if (mode == '>' || mode == '<')
                {

                    if (k == 0)
                    {

                        if (nextmode != 0)
                        {
                            pipe(rfd);
                        }
                        if ((indexbuf[k] + 1) == argc)
                        {
                            fprintf(stderr, "mysh: no redirectfile\n");
                        }
                        else if ((pid = fork()) < 0)
                        {
                            perror("pid");
                        }
                        else if (pid == 0)
                        {
                            pid = getpid();
                            setpgid(pid, pid);
                            p = argv;
                            argv[indexbuf[k]] = NULL;
                            safecpy(iofile, argv[indexbuf[k] + 1], FNMAX);
                            myredirect(mode, iofile);
                            if (nextmode != 0)
                            {
                                dup2(rfd[1], 1);
                                close(rfd[0]);
                                if (fp < 0)
                                {
                                    if ((indexbuf[k + 1] + 1) == argc)
                                    {
                                        fprintf(stderr, "mysh: no redirect file\n");
                                    }
                                    else
                                    {
                                        safecpy(iofile, argv[indexbuf[k + 1] + 1], FNMAX);
                                        if (nextmode == '<')
                                        {
                                            fprintf(stderr, "mysh: redirect error\n");
                                        }
                                        else
                                        {
                                            myredirect(nextmode, iofile);
                                        }
                                    }
                                }
                            }
                            findpath(p[0]);
                            execve(PATH, p, environ);
                            exit(0);
                        }
                        else
                        {
                            pgid = pid;
                            if ((fd = open("/dev/tty", O_RDWR)) < 0)
                            {
                                perror("file");
                            }
                            tcsetpgrp(fd, pgid);
                            close(fd);
                        }
                    }
                    else if (fp < 0)
                    {
                        close(rfd[0]);
                        close(rfd[1]);
                    }
                }
            }

            for (l = 0; l < pc + 1; l++)
            {
                wait(&stat[l]);
            }
            if ((fd = open("/dev/tty", O_RDWR)) < 0)
            {
                perror("file");
            }
            pid = getpid();
            tcsetpgrp(fd, pid);
            close(fd);
        }

        else if (strcmp(argv[0], "\n") != 0 && bgindex < 0)
        {
            mode = 0;
            shell(argc, argv, mode);
        }
        for (n = 0; n < argc; n++)
        {
            argv[n] = NULL;
            free(argv[n]);
        }
    }
    return 0;
}

int shell(int argc, char *argv[], char mode)
{
    int pcid = 0, status, n, pgid = 0, fd = 0;
    char dc[FNMAX];
    argv[argc] = NULL;
    if (strcmp(argv[0], "exit") == 0)
    {
        exit(0);
    }

    if (argc > 1 && strcmp(argv[0], "cd") == 0)
    {
        chdir(argv[1]);
        return 0;
    }
    if (strcmp(argv[0], "echo") == 0)
    {
        if (argc > 1)
        {
            printf("%s\n", argv[1]);
        }
        else
        {
            printf("\n");
        }
        return 0;
    }

    if (strcmp(argv[0], "pwd") == 0)
    {
        getcwd(dc, FNMAX);
        printf("%s\n", dc);
        return 0;
    }

    if ((pcid = fork()) < 0)
    {
        perror("pid");
        exit(1);
    }
    else if (pcid == 0)
    {
        pcid = getpid();
        setpgid(pcid, pcid);
        if (mode != 0)
        {
            myredirect(mode, iofile);
        }
        findpath(argv[0]);
        execve(PATH, argv, environ);
        exit(0);
    }
    else
    {
        pgid = pcid;
        if ((fd = open("/dev/tty", O_RDWR)) < 0)
        {
            perror("file");
        }
        tcsetpgrp(fd, pgid);
        wait(&status);
        pcid = getpid();
        tcsetpgrp(fd, pcid);
        close(fd);
    }
    return 0;
}

void findpath(char *cmd)
{
    char pathlist[PATHMAX][FNMAX], *buf, *place, *pathenv;
    int n = 0, find = 0;
    DIR *dir;
    struct dirent *dp;
    int pathnum = 0;
    pathenv = getenv("PATH");
    buf = strtok(pathenv, ":");
    safecpy(pathlist[pathnum++], buf, FNMAX);
    while (buf != NULL)
    {
        buf = strtok(NULL, ":");
        if (buf != NULL)
        {
            safecpy(pathlist[pathnum++], buf, FNMAX);
        }
    }

    place = "";

    for (n = 0; n < pathnum; n++)
    {
        dir = opendir(pathlist[n]);
        for (dp = readdir(dir); dp != NULL; dp = readdir(dir))
        {
            place = dp->d_name;
            if (strcmp(place, cmd) == 0)
            {
                find = 1;
                break;
            }
        }
        if (find)
        {
            safecpy(PATH, pathlist[n], FNMAX);
            strncat(PATH, "/", FNMAX);
            strncat(PATH, place, FNMAX);
            break;
        }
        closedir(dir);
    }
    if (find == 0)
    {
        fprintf(stderr, "mysh: command not found: %s\n", cmd);
    }
}

void sigch_handler()
{
    pid_t pid;
    int status;
    pid = waitpid(chpid, &status, WNOHANG);
    if (pid == chpid)
    {
        printf("\n[%d] + done       %s\n", bgp, bgcmd[bgp - 1]);
        bgp = 0;
        signal(SIGCHLD, SIG_DFL);
    }
}

void sigint_handler()
{
    pid_t pid;
    pid = getpid();
    if (pid != ppid)
    {
        exit(1);
    }
}