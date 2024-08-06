# Define the compiler
CC = gcc

# Define the compiler flags
CFLAGS = -O2 -fopenmp -DSTREAM_ARRAY_SIZE=2000000000 -DOFFSET=56

# Define the target executable
TARGET = stream

# Define the source files
SRCS = stream.c

# Define the object files (derived from the source files)
OBJS = $(SRCS:.c=.o)

# The default target that builds the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to build the object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean