CC := clang

ifeq ($(DEBUG),1)
	CFLAGS := -Wall -O0 
else
	CFLAGS := -Wall -O1 
endif

LDFLAGS := -lpcap -ljson-c -pthread -Wall -g
# -fsanitize=address

run: main.o hashtable.o statistics.o list.o
	$(CC) -o run main.o hashtable.o statistics.o list.o $(LDFLAGS)
	
main.o: main.c
	$(CC) $(CFLAGS) -c main.c $(LDFLAGS)
hashtable.o: hashtable.c hashtable.h
	$(CC) $(CFLAGS) -c hashtable.c $(LDFLAGS) 
statistics.o: statistics.c statistics.h
	$(CC) $(CFLAGS) -c statistics.c $(LDFLAGS) 
list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c $(LDFLAGS) 
 
clear:
	rm  -f *.o run output.csv statistics.csv