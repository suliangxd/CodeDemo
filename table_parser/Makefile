CXX ?= g++
CXXLD ?= $(CXX)
CXXFLAGS ?= -O2 -std=c++11 -Wall -Wextra -Wformat=2 -Wconversion
LDFLAGS ?= -fuse-ld=gold

export CXX CXXLD CXXFLAGS LDFLAGS

CXXFLAGS += "-I./include"

all : libtableparser.so

libtableparser.so : table_parser.o
	@echo "Linking shared object $@ ..."
	$(CXXLD) $(LDFLAGS) -shared $^ -o $@

table_parser.o : src/table_parser.cpp include/table_parser.h
	@echo "Compiling object $@ ..."
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

demo : demo.o libtableparser.so
	@echo "Compiling executable $@ ..."
	$(CXXLD) $(LDFLAGS) $< -o $@ -L. -ltableparser -Wl,-rpath=.

demo.o : src/main.cpp include/table_parser.h
	@echo "Compiling object $@ ..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean :
	@rm -vf table_parser.o demo.o libtableparser.so demo
	@$(MAKE) -C unittest clean

.PHONY: install

PREFIX ?= /usr/local

install : libtableparser.so
	install -v -Dm755 libtableparser.so $(PREFIX)/lib$(LIB_SUFFIX)
	install -v -Dm644 include/table_parser.h $(PREFIX)/include
	install -v -dm755 $(PREFIX)/share/doc/table_parser
	install -v -m644 readme $(PREFIX)/share/doc/table_parser
	install -v -m644 src/main.cpp $(PREFIX)/share/doc/table_parser/demo.cpp

.PHONY: check

check : demo
	@echo "Checking demo output ..."
	@./demo > demo.out
	@diff demo.out demo.std && touch demo.ac
	@rm -f demo.out
	@rm demo.ac
	@echo "demo output is correct."
	@echo "Doing unit tests..."
	@$(MAKE) -C unittest -B
