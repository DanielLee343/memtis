#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define DEVICE_PATH "/dev/shared_addr_dev"
#define BUFFER_SIZE (4096) // Adjust based on your needs
#define NUM_ADDRESSES 10

int main()
{
	int *buffer_size;
	int fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	unsigned long *shared_mem = (unsigned long *)mmap(
		NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (shared_mem == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return EXIT_FAILURE;
	}
	buffer_size = (int *)((char *)shared_mem + BUFFER_SIZE - sizeof(int));
	// Example array of addresses to write to the shared memory
	unsigned long addresses[NUM_ADDRESSES] = {
		0x12345678, 0x87654321, 0xAABBCCDD, 0xDDBBCCAA, 0x11111111,
		0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666
	};

	// Write the addresses into shared memory
	memcpy(shared_mem, addresses, sizeof(addresses));
	*buffer_size = 10;
	printf("Data written to shared memory:\n");
	for (int i = 0; i < NUM_ADDRESSES; ++i) {
		printf("Address %d: 0x%lx\n", i, shared_mem[i]);
	}

	// Cleanup
	munmap(shared_mem, BUFFER_SIZE);
	close(fd);
	return EXIT_SUCCESS;
}