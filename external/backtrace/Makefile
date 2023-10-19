.PHONY: all clean

all : backtrace.dll test.exe

backtrace.dll : backtrace.c
	gcc -O2 -shared -Wall -o $@ $^ -lbfd -liberty -limagehlp -lz -lintl

test.exe : test.c
	gcc -g -Wall -o $@ $^

clean :
	-del -f backtrace.dll test.exe
