# directories
INCLUDEDIR = include
SRCDIR = src
OBJDIR = obj
EXECUTABLEDIR = bin

# compiler
CC = g++
CFLAGS = -c -g -Wall -I$(INCLUDEDIR)

#linker
LINKER = g++
LDFLAGS =

# the := expands the meaning of the expression in the variable assignment
SOURCES := $(wildcard $(SRCDIR)/*.cc) # take all the .cc files in the src folder
OBJECTS := $(SOURCES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o) # in the SOURCES (variable content) what matches $(SRCDIR)/%.cc becomes $(OBJDIR)/%.o where the % is used to create an "entry to entry" correspondance
TARGETS_SOURCES := $(wildcard $(SRCDIR)/*.cpp)
TARGETS_OBJECTS := $(TARGETS_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGETS := $(TARGETS_SOURCES:$(SRCDIR)/%.cpp=$(EXECUTABLEDIR)/%)

all: $(TARGETS)

$(TARGETS): $(EXECUTABLEDIR)/%: $(OBJDIR)/%.o $(OBJECTS)
	@echo "\tLinking "$@
	@$(LINKER) $< $(OBJECTS) $(LDFLAGS) -o $@

$(TARGETS_OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "\tCompiling "$<
	@$(CC) $(CFLAGS) $< -o $@

# $< is the current input file and $@ is the target of this action the @ at the beginning of the line is to not print out the line
$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cc # create a object $(OBJDIR)/%.o from the file $(SRCDIR)/%.cc for the name matching $(OBJDIR)/%.o in the OBJECT variable
	@echo "\tCompiling "$<
	@$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS) $(wildcard $(OBJDIR)/*)

clear:
	rm -f $(wildcard $(INCLUDEDIR)/*~) $(wildcard $(SRCDIR)/*~) $(wildcard macros/*~)
