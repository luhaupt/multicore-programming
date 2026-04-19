SOURCES := $(wildcard assignment*/main.cpp)
TARGETS := $(SOURCES:%.cpp=%)

all: $(TARGETS)

%: %.cpp
	@g++ -std=c++20 -O2 -Wall -Wextra -pedantic -o $@ $<

clean:
	@rm -f $(TARGETS)
