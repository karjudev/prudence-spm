CXX = g++ -std=c++17 -O3
CXXFLAGS = -W -Wall -Wextra -Werror -pedantic -pedantic-errors -pthread

# External header files
LIB = ./lib
# All the files in the "prudence" directory
PRUDENCE = $(wildcard $(LIB)/prudence/*)
# All the executables
ALL = prudence_threads_static prudence_threads_dynamic prudence_fastflow_static prudence_fastflow_dynamic
# Path for the input CSV
INPUT = ./data/enron-reduced.csv
# Path for the output
OUTPUT = ./data/risk-reduced.csv
# Background knowledge size
H = 2

# Name of the current machine
MACHINE = thinkpad
# FastFlow library path
FFLIB = ./fastflow
# Maximum number of workers
NW_MAX = 8

.PHONY = all benchmark clean

all: $(ALL)

prudence_threads_static: prudence_threads_static.cpp $(LIB)/combinations.hpp $(LIB)/utimer.hpp
	$(CXX) $(CXXFLAGS) -I $(LIB) $< -o $@

prudence_threads_dynamic: prudence_threads_dynamic.cpp $(PRUDENCE) $(LIB)/entities.hpp $(LIB)/safe_queue.hpp $(LIB)/combinations.hpp $(LIB)/utimer.hpp
	$(CXX) $(CXXFLAGS) -I $(LIB) $< -o $@

prudence_fastflow_static: prudence_fastflow_static.cpp $(PRUDENCE) $(LIB)/combinations.hpp
	$(CXX) $(CXXFLAGS) -I $(FFLIB) -I $(LIB) $< -o $@

prudence_fastflow_dynamic: prudence_fastflow_dynamic.cpp $(LIB)/combinations.hpp
	$(CXX) $(CXXFLAGS) -I $(FFLIB) -I $(LIB) $< -o $@

benchmark: benchmark.sh $(ALL)
	./$< $(NW_MAX) $(H) $(INPUT) $(OUTPUT) | tee bench-$(MACHINE).csv

plots: plots.py
	mkdir -p plots/thinkpad
	mkdir -p plots/xeonphi
	python ./plots.py bench-thinkpad.csv plots/thinkpad
	python ./plots.py bench-xeonphi.csv plots/xeonphi

clean:
	rm $(ALL) $(OUTPUT)