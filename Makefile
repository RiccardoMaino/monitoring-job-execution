CFLAGS= -std=c11 -pedantic
COMMON_DEPS = src/*.h  Makefile

app: bin/test_app

bin/test_app: build/test_app.o build/utils.o $(COMMON_DEPS)
	$(CC) -o bin/test_app build/utils.o build/test_app.o

build/%.o: src/%.c $(COMMON_DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f build/* bin/*