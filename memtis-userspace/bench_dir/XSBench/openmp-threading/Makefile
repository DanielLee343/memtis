# Compiler and flags
CC = clang
CFLAGS = -std=gnu99 -Wall -fopenmp -DOPENMP -O3 -I/usr/lib/gcc/x86_64-linux-gnu/9/include
LDFLAGS = -lm
INSTRUMENTATION_FLAGS = -fpass-plugin=/mnt/newdrive/compiler_assisted/build/passes/RuntimeAddrPass.so -lrtlib -L/mnt/newdrive/compiler_assisted

# Source and object files
SRC = Main.c io.c Simulation.c GridInit.c XSutils.c Materials.c
OBJ = $(SRC:.c=.o)

# Instrumented source files and corresponding object files
INSTRU_SRC = Main_instru.c io.c Simulation.c GridInit.c XSutils.c Materials.c
INSTRU_OBJ = $(INSTRU_SRC:.c=_instru.o)

# Targets
TARGET = XSBench
TARGET_INSTRU = XSBench_instru

# Default target
all: $(TARGET) $(TARGET_INSTRU)

# Build the regular target
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

# Build the instrumented target
$(TARGET_INSTRU): $(INSTRU_OBJ)
	$(CC) $(CFLAGS) $(INSTRU_OBJ) $(LDFLAGS) $(INSTRUMENTATION_FLAGS) -o $@

# Compile regular object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile instrumented object files
%_instru.o: %.c
	$(CC) $(CFLAGS) $(INSTRUMENTATION_FLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJ) $(INSTRU_OBJ) $(TARGET) $(TARGET_INSTRU)