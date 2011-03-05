CXX ?= g++
CXXFLAGS ?= -Wall -g
MKDIR ?= mkdir -p
DEPDIR ?= .deps
OBJECTS := macexe.o macresfork.o util.o main.o
BIN := macloader

$(BIN): $(OBJECTS)
	$(CXX) $+ -o $@

%.o: %.cpp
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CXX) -MMD -MF "$(*D)/$(DEPDIR)/$(*F).d" -MQ "$@" -MP $(CPPFLAGS) -c $(<) -o $*.o

clean:
	rm -f $(BIN)
	rm -f $(OBJECTS)

all: $(BIN)
