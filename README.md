# A simple shell program
- run commands
- redirect stdin and stdout
- pipe
- run in background using `&`
- basic jobs control
- buildin commands
    - cd
    - pwd
    - exit
    - jobs

# Build
```
make
```

# Example
```
$./mysh
mysh>ls -l
total 60
-rw-r--r-- 1 ubuntu20_04 ubuntu20_04   357 Nov 10 18:13 Makefile
-rw-r--r-- 1 ubuntu20_04 ubuntu20_04     6 Nov 10 21:09 input
-rwxr-xr-x 1 ubuntu20_04 ubuntu20_04 22472 Nov 10 22:39 mysh
-rw-r--r-- 1 ubuntu20_04 ubuntu20_04  4205 Nov 10 22:39 myshell.c
-rw-r--r-- 1 ubuntu20_04 ubuntu20_04 14504 Nov 10 22:39 myshell.o
-rw------- 1 ubuntu20_04 ubuntu20_04   389 Nov 10 21:44 res
mysh>ls -l | grep Make
-rw-r--r-- 1 ubuntu20_04 ubuntu20_04   357 Nov 10 18:13 Makefile
mysh>ls -l | grep Make | wc
      1       9      65
mysh>ls -l | grep Make | wc > result
mysh>cat result
      1       9      65
mysh>exit
$
```