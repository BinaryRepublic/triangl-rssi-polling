# build helloworld executable when user executes "make"
#override CFLAGS := -I/usr/include/x86_64-linux-gnu $(CFLAGS) -lcurl


override CFLAGS := $(CFLAGS) -lcurl

helloworld: 
	$(CC) helloworld.c $(CFLAGS) -o helloworld

# remove object files and executable when user executes "make clean"
clean:
	rm -f helloworld

