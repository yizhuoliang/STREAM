ROOT_PATH = /home/yliang/llvm14-ldb
CC = $(ROOT_PATH)/build/bin/clang
LIBLDB = $(ROOT_PATH)/libldb/libldb.a

# Compiler flags
CFLAGS = -g -O3 -fno-omit-frame-pointer -fdebug-default-version=3 \
         -I$(ROOT_PATH)/build/lib -I$(ROOT_PATH)/libldb/include

LDFLAGS = -lpthread

# Target executable
all: stream_c.exe

stream_c.exe: stream.c $(LIBLDB)
	$(CC) $(CFLAGS) stream.c $(LIBLDB) $(LDFLAGS) -o stream_c.exe

clean:
	rm -f stream_c.exe *.o
