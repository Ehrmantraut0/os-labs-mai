#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>


int main() {
    const char program_name[] = "server";
    char file_name[260];

    ssize_t bytes;

    bytes = read(STDIN_FILENO, file_name, sizeof(file_name));
    
    if (bytes < 0) {
        const char msg[] = "Failed to read file name\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE); 
    }
    file_name[bytes - 1] = '\0';

    
    
    char progpath[1024];
	{
		ssize_t len = readlink("/proc/self/exe", progpath,
            sizeof(progpath) - 1);
            if (len == -1) {
			const char msg[] = "error: failed to read full program path\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}
        
		while (progpath[len] != '/')
        --len;
		progpath[len] = '\0';
	}
    
    int client_to_server[2];
    int server_to_client[2];

    
    if (pipe(client_to_server) == -1) {
        const char msg[] = "Failed to create pipe\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE);
    }
    if (pipe(server_to_client) == -1) {
        const char msg[] = "Failed to create pipe\n";
        write(STDOUT_FILENO, msg, sizeof(msg));
        exit(EXIT_FAILURE); 
    }

    pid_t child = fork();

    switch (child) {
        case -1: 
        { 
            const char msg[] = "error: failed to spawn new process\n";
            write(STDOUT_FILENO, msg, sizeof(msg));
            break;
        }
        case 0: 
        {
            close(client_to_server[1]);
            close(server_to_client[0]);

            dup2(client_to_server[0], STDIN_FILENO);
            close(client_to_server[0]);

            char fd[20];
            snprintf(fd, sizeof(fd), "%d", server_to_client[1]);

            char * args[] = {(char *)program_name, file_name, fd, NULL};
            
            char path[1024];
			snprintf(path, sizeof(path) - 1, "%s/%s", progpath, program_name);

            int32_t status = execv(path, args);
            if (status == -1) {
                const char msg[] = "error: failed to exec into new exectuable image\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
            }
            break;
        }
        default: 
        { 
            close(server_to_client[1]);
            close(client_to_server[0]);

            char buffer[4096];
            ssize_t bytes;

            while ((bytes = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
                if (bytes < 0) {
                    const char msg[] = "error: failed to read from stdin\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                    exit(EXIT_FAILURE);
                }
                else if (buffer[0] == '\n') {
                    break;
                }

                write(client_to_server[1], buffer, bytes);

                bytes = read(server_to_client[0], buffer, sizeof(buffer));
                if (bytes < 0) {
                    char msg[] = "Failed to read server\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    break;
                }
            
                if(buffer[0] == '0') {
                    char msg[] = "Wrong string!\n";
                    write(STDOUT_FILENO, msg, sizeof(msg));
                }
            }

            close(client_to_server[1]);
            close(server_to_client[0]);

            wait(NULL);
        }
    }
    exit(EXIT_SUCCESS);
}