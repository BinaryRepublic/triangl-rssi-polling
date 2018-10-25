# build helloworld executable when user executes "make"
rssi_polling: rssi_polling.o
	$(CC) $(LDFLAGS) rssi_polling.o -lcurl -o rssi_polling

rssi_polling.o: rssi_polling.c
	$(CC) $(CFLAGS) -c -lcurl rssi_polling.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o rssi_polling