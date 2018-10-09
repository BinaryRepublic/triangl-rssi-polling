# build helloworld executable when user executes "make"

CC=gcc
override CFLAGS := -lcurl $(CFLAGS)

testcurl: 
	$(CC) testcurl.c $(CFLAGS) -o testcurl

# remove object files and executable when user executes "make clean"
clean:
	rm -f testcurl

