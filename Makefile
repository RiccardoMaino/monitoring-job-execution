CFLAGS= -std=c11 -pedantic -O0
COMMON_DEPS = src/*.h  Makefile

all: test_app tracing_process

test_app: bin/test_app

tracing_process: bin/tracing_process

bin/test_app: build/test_app.o build/utils.o $(COMMON_DEPS)
	$(CC) -o bin/test_app build/utils.o build/test_app.o

bin/tracing_process: build/tracing_process.o build/utils.o $(COMMON_DEPS)
	$(CC) -o bin/tracing_process build/utils.o build/tracing_process.o

build/%.o: src/%.c $(COMMON_DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f build/* bin/*