# Event Tracing Library
CFLAGS= -std=c11 -pedantic
EVENT_TRACING_H = event_tracing_library/include/*.h 
LIST_H = event_tracing_library/src/list.h
COMMON_DEPS = Makefile

app: event_tracing_library/bin/test_app

samplerun: app
	event_tracing_library/bin/sampling.sh

event_tracing_library/bin/test_app: event_tracing_library/build/test_app.o event_tracing_library/build/event_tracing.o event_tracing_library/build/list.o | event_tracing_library/bin
	$(CC) -o event_tracing_library/bin/test_app event_tracing_library/build/event_tracing.o event_tracing_library/build/list.o event_tracing_library/build/test_app.o

event_tracing_library/build/test_app.o: event_tracing_library/src/test_app.c $(EVENT_TRACING_H) $(LIST_H) $(COMMON_DEPS) | event_tracing_library/build
	$(CC) $(CFLAGS) -c $< -o $@

event_tracing_library/build/event_tracing.o: event_tracing_library/include/event_tracing.c $(EVENT_TRACING_H) $(COMMON_DEPS) | event_tracing_library/build
	$(CC) $(CFLAGS) -c $< -o $@

event_tracing_library/build/list.o: event_tracing_library/src/list.c $(LIST_H) $(COMMON_DEPS) | event_tracing_library/build
	$(CC) $(CFLAGS) -c $< -o $@

event_tracing_library/build:
	mkdir -p event_tracing_library/build

event_tracing_library/bin:
	mkdir -p event_tracing_library/bin

clean:
	rm -f event_tracing_library/build/* event_tracing_library/bin/test_app

# Analysis Python Module
pyrun:
	python3 py_analysis_module/app.py --makeplots

pydeps:
	pip install numpy
	pip install pandas
	pip install matplotlib
	pip install seaborn