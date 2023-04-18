CFLAGS= -std=c11 -pedantic
COMMON_DEPS = src/*.h  Makefile

app: bin/test_app

bin/test_app: build/test_app.o build/tracing.o build/list.o $(COMMON_DEPS)
	$(CC) -o bin/test_app build/tracing.o build/list.o build/test_app.o

build/%.o: src/%.c $(COMMON_DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f build/* bin/*