#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#define DEVICE "/dev/poll_device"
#define BUFFER_SIZE 1024

int main()
{
	int fd;
	char buffer[BUFFER_SIZE];

	fd = open(DEVICE, O_WRONLY);
	if (fd < 0) {
		perror("Failed to open device");
		return EXIT_FAILURE;
	}

	while (1) {
		printf("Enter data to send to kernel: ");
		if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
			perror("Failed to read input");
			close(fd);
			return EXIT_FAILURE;
		}

		size_t len = strlen(buffer);
		if (buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';
			len--;
		}

		if (write(fd, buffer, len) < 0) {
			perror("Failed to write to device");
			close(fd);
			return EXIT_FAILURE;
		}

		printf("Data sent to kernel: %s\n", buffer);
	}

	close(fd);
	return EXIT_SUCCESS;
}