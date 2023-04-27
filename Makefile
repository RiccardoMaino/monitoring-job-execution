CFLAGS= -std=c11 -pedantic
INCLUDE_DEPS = event_tracing_library/include/*.h 
LIST_DEPS = event_tracing_library/src/list.h
COMMON_DEPS = Makefile

create_folder: event_tracing_library/build/.dirstamp event_tracing_library/bin/.dirstamp

event_tracing_library/build/.dirstamp:
	mkdir -p event_tracing_library/build
	touch event_tracing_library/build/.dirstamp

event_tracing_library/bin/.dirstamp:
	mkdir -p event_tracing_library/bin
	touch event_tracing_library/bin/.dirstamp

run: app
	event_tracing_library/bin/sampling.sh

app: create_folder bin/test_app

bin/test_app: build/test_app.o build/event_tracing.o build/list.o
	$(CC) -o event_tracing_library/bin/test_app event_tracing_library/build/event_tracing.o event_tracing_library/build/list.o event_tracing_library/build/test_app.o

build/test_app.o: event_tracing_library/src/test_app.c $(INCLUDE_DEPS) $(LIST_DEPS) $(COMMON_DEPS)
	$(CC) $(CFLAGS) -c event_tracing_library/src/test_app.c -o event_tracing_library/build/test_app.o

build/event_tracing.o: event_tracing_library/include/event_tracing.c $(INCLUDE_DEPS) $(COMMON_DEPS)
	$(CC) $(CFLAGS) -c event_tracing_library/include/event_tracing.c -o event_tracing_library/build/event_tracing.o

build/list.o: event_tracing_library/src/list.c $(LIST_DEPS) $(COMMON_DEPS)
	$(CC) $(CFLAGS) -c event_tracing_library/src/list.c -o event_tracing_library/build/list.o

clean:
	rm -f event_tracing_library/build/* event_tracing_library/bin/test_app


# build/%.o: src/%.c $(COMMON_DEPS) $(LIST_DEPS)
# 	$(CC) $(CFLAGS) -c $< -o $@

# CFLAGS= -std=c11 -pedantic
# COMMON_DEPS = src/*.h  Makefile

# app: bin/test_app

# bin/test_app: build/test_app.o build/event_tracing.o build/list.o $(COMMON_DEPS)
# 	$(CC) -o bin/test_app build/event_tracing.o build/list.o build/test_app.o

# event_tracing_library/build/%.o: event_tracing_library/src/%.c $(COMMON_DEPS)
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f build/* bin/test_app