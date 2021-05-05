CC=g++
CFLAGS= -o


.cpp.o:
	$(CC) $(CFLAGS) 
	
main: oss.cpp user
	$(CC) $(CFLAGS) oss oss.cpp

user : user.cpp
	$(CC) $(CFLAGS) user user.cpp
	
clean:
	rm oss.o user.o