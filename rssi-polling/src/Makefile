# build helloworld executable when user executes "make"
override CFLAGS := -I/usr/include/x86_64-linux-gnu/curl $(CFLAGS)

testcurl: 
	$(CC) testcurl.c $(CFLAGS) -o testcurl

# remove object files and executable when user executes "make clean"
clean:
	rm -f testcurl

