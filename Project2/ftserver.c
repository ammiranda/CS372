/**
 * Name: Alexander Miranda
 * Assignment: Project 2 File Transfer client/server
 * Due Date: 11/26/2017
 * 
 * This file is my implementation of the FTP server in the C language.
 * 
 * References: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
*/

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>

// Constants that specify the lower and upper bounds for which port 
// the server process can listen to when running
int const MIN_PORT = 1024;
int const MAX_PORT = 65535;
char * const WORKING_DIR = ".";

/**
 * Method that will output an error message to the stderr of the server process
 * Partially inspired by: http://www.linuxhowtos.org/C_C++/socket.htm
 * 
 * err_text: {Char *} The error message that will be outputted to the console
*/
void err_output(char * err_text) {
    perror(err_text);
    exit(1);
}

/**
 * Function that starts the server and binds to the provided port_num param
 * This method was inspired by the post: http://www.linuxhowtos.org/C_C++/socket.htm
 * 
 * port_num: {Integer} The port for the server process to listen to
 * 
 * return: {Integer} An integer that represents the socket file description
*/
int initialize_server(int port_num) {
    int socket_file_des = socket(AF_INET, SOCK_STREAM, 0);

    // Fail out of the function if socket creation fails
    if (socket_file_des < 0) {
        return -1;
    }

    struct sockaddr_in server_obj;
    server_obj.sin_family = AF_INET;
    server_obj.sin_port = htons(port_num);
    server_obj.sin_addr.s_addr = INADDR_ANY;

    int opt_val = 1;

    setsockopt(socket_file_des, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    int bind_status = bind(socket_file_des, (struct sockaddr *) &server_obj, sizeof(server_obj));

    if (bind_status < 0) {
        error("Could not bind to specified port");
    }

    int listen_status = listen(socket_file_des, 10);

    if (listen_status < 0) {
        error("Could not listen to the specified port");
    }

    return socket_file_des;
}

/**
 * Receives the message data sent from the client to the server and copies
 * that data to the data pointer that is passed in to then be read elsewhere.
 * 
 * socket_file_des: {Integer} - Integer representing the socket file description
 * data: {Char *} - Data buffer that will receive the message contents that was
 *  sent by the client
 * size: {size_t} - The size of the data being received
 * 
*/
void read_message(int socket_file_des, char * data, size_t size) {
    char temp_data[size + 1];
    ssize_t chunk_tracker;
    size_t data_read = 0;

    while (data_read < size) {
        chunk_tracker = read(socket_file_des, temp_data + data_read, size - data_read);
        data_read += chunk_tracker;

        if (chunk_tracker < 0) {
            error("Error retrieving the message!");
            exit(1);
        }
    }

    strncpy(data, temp_data, size);
}

/**
 * Getter for the current working directory's file contents
 * This method was inspired by the page: http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
 * 
 * file_path: {Char *}
 * 
*/
int grab_dir(char * file_path[]) {
    DIR * dir;
    struct dirent * dir_obj;
    int dir_size = 0;
    int file_count = 0;

    dir = opendir(WORKING_DIR);

    if (dir) {
        int i = 0;

        while ((dir_obj = readdir(dir)) != NULL) {
            if (dir_obj->d_type == DT_REG) {
                file_path[i] = dir_obj->d_name;
                dir_size += strlen(file_path[i]);
                i++;
            }
        }

        file_count = i - 1;
    }

    closedir(dir);
    
    return dir_size + file_count;
}

/**
 * 
 * 
*/
char * read_file(char * file_name) {
    char * src = NULL;

    FILE * file_ptr = fopen(file_name, "r");

    if (file_ptr == NULL) {
        error("Could not open file");
    }

    if (file_ptr != NULL) {
        if (fseek(file_ptr, 0L, SEEK_END) == 0) {
            long buffer_size = ftell(file_ptr);

            if (buffer_size == -1) {
                error("Invalid file!");
                exit(1);
            }

            src = malloc(sizeof(char) * (buffer_size + 1));

            if (fseek(file_ptr, 0L, SEEK_SET) != 0) {
                error("Unable to read file");
            }

            size_t file_length = fread(src, sizeof(char), buffer_size, file_ptr);

            if (ferror(file_ptr) != 0) {
                fputs("Error reading the file", stderr);
            } else {
                // Terminating the file contents char array with a null terminator
                src[file_length++] = '\0';
            }
        }
    }

    fclose(file_ptr);

    return src;
}

/**
 * Receives numbers that are passed through the socket connection
 * 
 * socket_file_des: {Integer} - Integer representing the socket file description
 * 
 * return: {Integer} - The number that was passed through the socket
 * 
*/
int get_number(int socket_file_des) {
    int passed_num;

    ssize_t file_chunk = 0;
    file_chunk = read(socket_file_des, &passed_num, sizeof(int));

    if (file_chunk < 0) {
        error("Unable to retrieve the number passed into the socket.");
    }

    return passed_num;
}

/**
 * Method that transmits an integer to the passed in socket connection
 * 
 * socket_file_des: {Integer} - Integer representing the socket file description
 * passed_num: {Integer} - The number to be transmitted to the socket connection
*/
void transmit_number(int socket_file_des, int passed_num) {
    ssize_t file_chunk = 0;

    file_chunk = write(socket_file_des, &passed_num, sizeof(int));

    if (file_chunk < 0) {
        error("Unable to send number through socket.");
    }
}

/**
 * This parses the original client request to determine if the client
 * is either requesting the file contents list or requesting to download a
 * specific file that the server can transmit
 * 
 * socket_file_des: {Integer} - Integer representing the socket file description
 * data_port: {Integer *} - Integer pointer that references the 
 * 
 * return: {Integer} - The number denoting what the client requested
 *  if the number is 1 the client requested the contents of the current directory
 *  on the server. If the number is 2 the client is requesting to download
 *  one of the files in the server's directory
*/
int request_handler(int socket_file_des, int * data_port) {
    char user_input[3] = '\0';

    read_message(socket_file_des, user_input, 3);
    *data_port = get_number(socket_file_des);

    if (strcmp(user_input, "-l") == 0) {
        return 1;
    }

    if (strcmp(user_input, "-g") == 0) {
        return 2;
    }

    return 0;
}

/**
 * This method sends the data from the server to the client.
 * 
 * sock_num: {Integer} - Integer representing the socket file description
 * data: {Char *} - Data buffer containing the data to be transmitted to the client
 * 
*/
void transmit_message(int sock_num, char * data) {
    ssize_t chunk_tracker;
    size_t size = strlen(data) + 1;
    size_t data_sent = 0;

    while (data_sent < size) {
        chunk_tracker = write(sock_num, data, size - data_sent);

        data_sent += chunk_tracker;

        if (chunk_tracker < 0) {
            error("Error when sending message data");
            exit(1);
        } else if (chunk_tracker == 0) {
            data_sent = size - data_sent;
        }
    }
}

/**
 * Sends a file with the passed in file name to the client.
 * 
 * socket_file_des: {Integer} - Integer representing the socket file description
 * file_name: {Char *} - File name of the file that is transmitted to the client
*/
void send_file(int socket_file_des, char * file_name) {
    char * file_to_send = read_file(file_name);

    transmit_number(socket_file_des, strlen(file_to_send));
    transmit_message(socket_file_des, file_to_send);
}

/**
 * Main method for the ftserver that parses the command-line arguments
 * to properly configure the server socket.
 * 
 * argc: An integer value denoting how many command-line arguments
 * were provided
 * argv: An array of the strings passed as command-line arguments
 * when this executable was run
 * 
*/
int main(int argc, char *argv[]) {
    // Declaring necessary variables
    int socket_file_des;
    int new_socket_file_des;
    int data_socket_file_des;
    int port_num;
    int process_id;

    // Checking that the necessary args were passed to the
    // executable
    if (argc < 2) {
        error("Usage: ftserver <port_num>\n");
        exit(1);
    }

    // Casting the port_num passed in to be an integer
    port_num = atoi(argv[1]);

    if (port_num < MIN_PORT || port_num > MAX_PORT) {
        error("Port number selected is outside the desired range\n");
    }

    socket_file_des = initialize_server(port_num);
    printf("Server open and listening on %d\n", port_num);

    while(1) {
        new_socket_file_des = accept(socket_file_des, NULL, NULL);

        if (new_socket_file_des < 0) {
            error("Error on accept\n");
        }

        process_id = fork();

        if (process_id < 0) {
            error("Could not fork the process properly\n");
        }

        if (process_id == 0) {
            close(socket_file_des);

            int cmd = 0;
            int data_port;
            int new_socket;

            
        }
    }
}