# Define the compiler and flags
CC = clang
CFLAGS = -O2 -fopenmp -DSTREAM_ARRAY_SIZE=2000000000 -DOFFSET=56
#  -fopenmp -DSTREAM_ARRAY_SIZE=2000000000 -DOFFSET=56

# Define the source files and object files
STREAM_SRC = stream.c
STREAM_OBJ = stream.o

STREAM_INSTRU_SRC = stream_instru.c
STREAM_INSTRU_OBJ = stream_instru.o
STREAM_INSTRU_PASS = /mnt/newdrive/compiler_assisted/build/passes/RuntimeAddrPass.so
STREAM_INSTRU_LIB = -lrtlib -L/mnt/newdrive/compiler_assisted

# Default target
all: stream stream_instru

# Target to generate the stream executable
stream: $(STREAM_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Target to compile stream.o
$(STREAM_OBJ): $(STREAM_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Target to generate the stream_instru executable
stream_instru: $(STREAM_INSTRU_SRC)
	$(CC) -fpass-plugin=$(STREAM_INSTRU_PASS) $< $(STREAM_INSTRU_LIB) $(CFLAGS) -o $@

# Phony targets to avoid conflicts with files of the same name
.PHONY: all clean

# Target to clean up the generated files
clean:
	rm -f stream stream_instru $(STREAM_OBJ)