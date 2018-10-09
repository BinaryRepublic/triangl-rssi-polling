# build helloworld executable when user executes "make"
testcurl: testcurl.o
	$(CC) $(LDFLAGS) testcurl.o -o testcurl

testcurl.o: testcurl.c
	$(CC) $(CFLAGS) -c -lcurl testcurl.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o testcurl

