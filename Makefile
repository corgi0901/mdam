TARGET=mdam.so
CFLAGS=-shared -fPIC -O2 -Wall -Wextra -ldl
$(TARGET): *.c
	gcc *.c $(CFLAGS) -o $(TARGET)

clean:
	rm $(TARGET)
