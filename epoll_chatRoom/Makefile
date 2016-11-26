# Toolchain
CXX ?= g++
CXXFLAGS ?=  -O2 -Wall -Wextra -Wconversion -Wshadow
CXXLD ?= $(CXX)
LDFLAGS ?= -static -L/usr/local/lib
LIBS = -lm

# Useful objects
BIN = client server

all : $(BIN)

client : client.o 
	$(CXXLD) $(LDFLAGS) $^ $(LIBS) -o $@

server : server.o
	$(CXXLD) $(LDFLAGS) $^ $(LIBS) -o $@

PREFIX ?= /opt/light

install : $(BIN)
	install -v -Dm755 $(BIN) $(PREFIX)/bin

# Depending
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
DEP = $(patsubst %.cpp, %.dep, $(SRC))

sinclude $(DEP)

%.dep : %.cpp
	@echo "Searching dependencies for source $<..."
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@:,g' < $@.$$$$ > $@; \
	rm $@.$$$$

.PHONY : clean
clean:
	rm -f $(BIN) $(OBJ) $(DEP)
