CC = gcc

OPTFLAGS = -O2 -pipe -march=armv6zk -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
DBUG = -g -Wall -Wextra -Wcast-align
CCFLAGS = -fPIC $(OPTFLAGS) $(DBUG)

LIBS = -levent -lrt

chat-server: chat-server.c
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f chat-server *~
