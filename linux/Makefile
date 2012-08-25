CC   = gcc
OPTS = -Wall -g

#all: libmfs server client
all: libmfs server client tester

# this generates the target executables
server: server.o udp.o
	$(CC) -o server server.o udp.o 

client: client.o udp.o
	$(CC) -o client client.o udp.o  libmfs.so

tester: tester.o udp.o
	$(CC) -o tester tester.o udp.o libmfs.so

libmfs: udp.o mfs.o
	$(CC) -shared -g -Wall -o libmfs.so udp.o mfs.o
udp.o:
	$(CC) $(OPTS) -c -fpic udp.c -o udp.o
mfs.o:
	$(CC) $(OPTS) -c -fpic mfs.c -o mfs.o

#servermfs:
#	$(CC) $(OPTS) -c mfs.c -o libmfs.so

# this is a generic rule for .o files 
%.o: %.c 
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm -f server.o udp.o client.o server client libmfs.so disk mfs.o tester.o tester
