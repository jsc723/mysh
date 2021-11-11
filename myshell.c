#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECKED(call, msg) if((call) == -1) {perror(msg);exit(1);}
#define PTRCHECK(pointer, msg) if((pointer) == NULL) {printf(msg);printf("\n");exit(2);}
#define BUFSIZE 4096
char buf[BUFSIZE];
int path_max;

char **parse_args(char *cmd, int *argc);
void free_strs(char **strs, int len);
int split_cmds(char *buf, int n, char ***cmds, int *m);
void run_cmds(char **cmds, int m);
int run_buildin_cmd(char *cmds);

int split_cmds(char *buf, int n, char ***cmds, int *m) {
    int i, j;
    *m = 1;
    for(i = 0; i < n; i++) {
        if(buf[i] == '|') (*m)++;
    }
    *cmds = (char **) malloc(sizeof(char*) * *m);
    i = 0, j = 0;
    while(i < n) {
        int k = i, sz;
        while(i < n && buf[i] != '|') i++;
        sz = i-k+1;
        (*cmds)[j] = (char *) malloc(sz);
        PTRCHECK((*cmds)[j], "malloc cmd");
        memcpy((*cmds)[j], buf + k, sz-1);
        (*cmds)[j][sz-1] = '\0';
        i++;
        j++;
    }
    return 0;
}
int run_buildin_cmd(char *cmd) {
    int argc;
    char **argv = parse_args(cmd, &argc);
    int ret = 0;
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(argv[0], "pwd") == 0) {
        char *path = (char *)malloc((path_max+1) * sizeof(char));
        getcwd(path, path_max);
        printf("%s\n", path);
        free(path);
        ret = 1;
    } else if (strcmp(argv[0], "cd") == 0) {
        char *dir = argc >= 2 ? argv[1] : getenv("HOME");
        if(chdir(dir) == -1) {
            perror("chdir");
        }
        ret = 1;
    }
    free_strs(argv, argc);
    return ret;
}

void run_cmds(char **cmds, int m) {
    int i;
    int fd[50][2];
    if (m == 1 && run_buildin_cmd(cmds[0])) {
        return;
    }
    for(i = 0; i < m; i++) {
        int pid;
        if(i < m-1) {
           CHECKED(pipe(fd[i]), "pipe");
        }
        if((pid = fork()) == 0) {
            int argc, j;
            char **argv;
            if (i > 0) {
                CHECKED(dup2(fd[i-1][0], STDIN_FILENO), "dup2");
            }
            if (i < m-1) {
                CHECKED(dup2(fd[i][1], STDOUT_FILENO), "dup2");
            }
            for(j = 0; j <= i && j < m-1; j++) {
                CHECKED(close(fd[j][0]), "close pipes child");
                CHECKED(close(fd[j][1]), "close pipes child");
            }
            argv = parse_args(cmds[i], &argc);
            CHECKED(execvp(argv[0], argv), "exec");
        }
        CHECKED(pid, "fork");
    }
    for(i = 0; i < m-1; i++) {
        CHECKED(close(fd[i][0]), "close pipes shell");
        CHECKED(close(fd[i][1]), "close pipes shell");
    }
    for(i = 0; i < m; i++) {
        CHECKED(wait(NULL), "wait");
    }
}
void free_strs(char **strs, int m) {
    int i;
    for(i = 0; i < m; i++) {
        free(strs[i]);
    }
    free(strs);
}

char **parse_args(char *cmd, int *argc) {
    int n = strlen(cmd);
    int i = 0, j = 0;
    int cap = 2, ac = 0;
    int redirect_stdin = 0, redirect_stdout = 0;
    char **argv = (char **) malloc(sizeof(char *) * cap);
    PTRCHECK(argv, "malloc argv");
    while(i < n) {
        char *arg;
        while(i < n && cmd[i] == ' ') i++;
        if (i == n) break; //tail space
        if (cmd[i] == '<') {
            redirect_stdin = 1;
            i++;
            continue;
        } else if (cmd[i] == '>') {
            redirect_stdout = 1;
            i++;
            continue;
        }

        j = i + 1;
        while(j < n && cmd[j] != ' ' && cmd[j] != '<' && cmd[j] != '>') j++;

        arg = malloc(sizeof(char) * (j-i+1));
        strncpy(arg, cmd + i, j-i);
        arg[j-i] = '\0';

        if (redirect_stdin) {
            redirect_stdin = 0;
            int fd = open(arg, O_RDONLY);
            CHECKED(fd, "redirect stdin open");
            CHECKED(dup2(fd, STDIN_FILENO), "redirect stdin dup2");
            CHECKED(close(fd), "redirect stdin close");
        } else if (redirect_stdout) {
            redirect_stdout = 0;
            int fd = open(arg, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            CHECKED(fd, "redirect stdout open");
            CHECKED(dup2(fd, STDOUT_FILENO), "redirect stdout dup2");
            CHECKED(close(fd), "redirect stdout close");

        } else {
            if (ac + 1 == cap) {
                cap *= 2;
                argv = (char **) realloc(argv, sizeof(char *) * cap);
                PTRCHECK(argv, "realloc argv");
            }
            argv[ac++] = arg;
        }
        i = j;
    }
    argv[ac] = NULL;
    *argc = ac;
    return argv;
}



int main() {
    buf[0] = '\0';
    char **cmds = NULL;
    int m;
    path_max = fpathconf(0, _PC_PATH_MAX);
    while(1) {
        int n;
        CHECKED(write(STDOUT_FILENO, "mysh>", 5), "write commend");
        CHECKED(n = read(STDIN_FILENO, buf, BUFSIZE-1), "read command");
        if (n == 0) exit(0);
        buf[--n] = '\0';
        if(strlen(buf)==0) continue;
        if(split_cmds(buf, n, &cmds, &m) < 0) {
            printf("cannot parse the commend\n");
            continue;
        }
        run_cmds(cmds, m);
        free_strs(cmds, m);
    }
    return 0;
}
