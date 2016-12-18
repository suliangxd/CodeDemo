# Do not make this directly.
# Need variables in ../Makefile.

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))
EXE = $(patsubst %.cpp, %.exe, $(SRC))

all : $(EXE)

%.o : %.cpp
	@echo "Compiling object $@ ..."
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I../include

%.exe : %.o
	@echo "Linking executable $@ ..."
	$(CXXLD) $(LDFLAGS) $< -o $@ -lgtest -lpthread -L.. -ltableparser \
	 -Wl,-rpath=..
	@echo "Running test $@ ..."
	./$@

.PHONY: clean
clean :
	@rm -fv $(OBJ) $(EXE)
