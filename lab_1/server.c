#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>


int main(int argc, const char ** argv) {
    int server_to_client = atoi(argv[2]);

    int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0600);
	if (file == -1) {
		const char msg[] = "error: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

    dup2(file, STDOUT_FILENO);
    close(file);

    int32_t bytes;
    char buffer[1024];
    while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        if (bytes == -1) {
            const char msg[] = "error: failed to read client\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }


        if (buffer[0] >= 'A' && buffer[0] <= 'Z') {
            write(STDOUT_FILENO, buffer, bytes);
            const char msg[] = "1";
            write(server_to_client, msg, sizeof(msg));
        }
        else {
            const char msg[] = "0";
            write(server_to_client, msg, sizeof(msg));
        }
    }
    close(file);
    return 0;
}