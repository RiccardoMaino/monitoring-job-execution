# Event Tracing Library
CFLAGS= -std=c11 -pedantic
EVENT_TRACING_H = event_tracing_library/include/*.h 
LIST_H = event_tracing_library/src/list.h
COMMON_DEPS = Makefile

# All
runall: run pyrun

cleanall:
	@if [ -f dataset.csv ]; then \
			rm dataset.csv; \
			echo "Removed 'dataset.csv' file"; \
	fi
	@if [ -f py_analysis_module/dataset.csv ]; then \
			rm py_analysis_module/dataset.csv; \
			echo "Removed 'py_analysis_module/dataset.csv' file"; \
	fi
	@if [ -d plots ]; then \
			rm -r plots; \
			echo "Removed 'plots' directory"; \
	fi
	@if [ -d py_analysis_module/plots ]; then \
			rm -r py_analysis_module/plots; \
			echo "Removed 'py_analysis_module/plots' directory"; \
	fi
	@if [ -d results ]; then \
			sudo rm -r results; \
			echo "Removed 'results' directory"; \
	fi
	@if [ -d event_tracing_library/results ]; then \
			sudo rm -r event_tracing_library/results; \
			echo "Removed 'event_tracing_library/results' directory"; \
	fi

# Event Tracing Library
run: app
	sudo event_tracing_library/bin/test_app --param 10000000 --mode 1 --policy SCHED_FIFO --priority 99 --nowait --respath results
	sudo event_tracing_library/bin/test_app --param 1000000 --mode 2 --policy SCHED_FIFO --priority 99 --nowait --respath results
	sudo event_tracing_library/bin/test_app --param 100000 --mode 3 --policy SCHED_FIFO --priority 99 --nowait --respath results

app: event_tracing_library/bin/test_app

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
	@if [ -f event_tracing_library/bin/test_app ]; then \
			rm -f event_tracing_library/bin/test_app; \
			echo "Removed 'event_tracing_library/bin/test_app' file"; \
	fi
	@if [ -d event_tracing_library/build ]; then \
			rm -f event_tracing_library/build/*; \
			echo "Removed 'event_tracing_library/build/*' files"; \
	fi

# Analysis Python Module
pyrun:
	python3 py_analysis_module/app.py --respath results --csvpath dataset.csv --p plots --makeplots

pydeps:
	pip install numpy
	pip install pandas
	pip install matplotlib
	pip install seaborn
