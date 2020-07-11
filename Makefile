DEBUG ?= 0
TARGET ?= net-collect
INCDIRS = ./inc
SRCDIR = ./src
SRCS := $(TARGET).c
SRCS += argparser.c
BUILDDIR = ./.build


CC = gcc
CFLAGS := -O2 -std=gnu18 -fms-extensions 
#CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -DDEBUG=$(DEBUG)
LDFLAGS := -pthread

.PHONY: all clean tidy

#Dfault target
all: $(TARGET)

#Recipe for object files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) $(addprefix -I,$(INCDIRS)) -c $< -o $@

#Recipe for target
$(BUILDDIR)/$(TARGET): $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ 

$(TARGET): $(BUILDDIR)/$(TARGET)
	ln -sf $< $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)/*.o

tidy: clean
	rm -f $(TARGET)
	
