# $^ all dependencies
# $< first dependency
# $@ target
CC = gcc
CFLAGS = -g -Wall
CPPFLAGS = 
LDFLAGS = 

src = $(wildcard *.c)
obj = $(patsubst %.c,%.o,$(src))
target = mysh

$(target):$(obj)
	$(CC) $^ $(LDFLAGS) -o $@

%.o:%.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

.PHONY:clean
clean:
	-rm -f $(obj)
	-rm -f $(target)

install:
	cp $(target) /usr/bin