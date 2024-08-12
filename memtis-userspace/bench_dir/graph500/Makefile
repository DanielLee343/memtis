# Compiler and flags
CC = clang
CFLAGS = -g -std=c99 -Wall -O3 -march=native -fopenmp -I/usr/lib/gcc/x86_64-linux-gnu/9/include -I./generator
LDFLAGS = -flto -fopenmp -lm -lrt

# Instrumentation flags
INSTRUMENTATION_FLAGS = -fpass-plugin=/mnt/newdrive/compiler_assisted/build/passes/RuntimeAddrPass.so -lrtlib -L/mnt/newdrive/compiler_assisted

COMMON_SRC = omp-csr/omp-csr.c options.c rmat.c kronecker.c verify.c prng.c timer.c xalloc.c \
      generator/splittable_mrg.c generator/graph_generator.c generator/make_graph.c generator/utils.c

SRC = graph500.c $(COMMON_SRC)
INSTRU_SRC = graph500_instru.c $(COMMON_SRC)

OBJ = $(SRC:.c=.o)

INSTRU_OBJ = $(INSTRU_SRC:.c=_instru.o)

# Targets
TARGET = omp-csr/omp-csr
TARGET_INSTRU = omp-csr/omp-csr_instru

# Default target
all: $(TARGET) $(TARGET_INSTRU)

# Build regular target
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

# Build instrumented target
$(TARGET_INSTRU): $(INSTRU_OBJ)
	$(CC) $(INSTRU_OBJ) $(LDFLAGS) $(INSTRUMENTATION_FLAGS) -o $@

# Compile regular object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile instrumented object files
%_instru.o: %.c
	$(CC) $(CFLAGS) $(INSTRUMENTATION_FLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ) $(INSTRU_OBJ) $(TARGET) $(TARGET_INSTRU)