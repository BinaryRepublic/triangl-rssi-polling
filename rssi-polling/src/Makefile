# build helloworld executable when user executes "make"
rssi-polling: rssi-polling.o
	$(CC) $(LDFLAGS) rssi-polling.o -lcurl -o rssi-polling

rssi-polling.o: rssi-polling.c
	$(CC) $(CFLAGS) -c -lcurl rssi-polling.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o rssi-polling