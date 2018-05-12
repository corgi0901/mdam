TARGET=mdam.so

$(TARGET): *.c
	gcc -shared -fPIC *.c -O2 -o $(TARGET) -ldl

clean:
	rm $(TARGET)