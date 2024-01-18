#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>

#define MAX_PORT_NAME_LEN 20

// Use std::string instead of string
std::string controller_identifier = "CNC Arduino controller";
bool controller_state = false;

int try_open_serial_port(const char *port) {
    int fd = open(port, O_RDWR | O_NOCTTY);
    if (fd != -1) {
        return fd;  // Successfully opened the port
    }
    return -1;  // Failed to open the port
}

void configure_serial_port(int fd) {
    struct termios serial_port_settings;

    if (tcgetattr(fd, &serial_port_settings) == -1) {
        perror("Error getting serial port settings");
        exit(EXIT_FAILURE);
    }

    cfsetispeed(&serial_port_settings, B115200);
    cfsetospeed(&serial_port_settings, B115200);

    serial_port_settings.c_cflag &= ~PARENB;
    serial_port_settings.c_cflag &= ~CSTOPB;
    serial_port_settings.c_cflag &= ~CSIZE;
    serial_port_settings.c_cflag |= CS8;

    if (tcsetattr(fd, TCSANOW, &serial_port_settings) == -1) {
        perror("Error setting serial port settings");
        exit(EXIT_FAILURE);
    }
}

int auto_detect_serial_port(int* current_port) {
    std::vector<std::string> possible_ports = {"/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyACM2", "/dev/ttyUSB0"};

    for (int i=*current_port+1; i<possible_ports.size(); i++) {

		std::cout << "trying to open serial port: " << possible_ports[i] << std::endl;
        int fd = try_open_serial_port(possible_ports[i].c_str());
        if (fd != -1) {
			*current_port = i;
            return fd;  // Found a connected port
        }
    }

    return -1;  // No connected port found
}

int main() {
    int serial_fd = -1;
	int current_port = -1;

    while (serial_fd == -1) {
        serial_fd = auto_detect_serial_port(&current_port);
        if (serial_fd == -1) {
            std::cerr << "Error: Unable to detect connected serial port." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));  // Wait for 2 seconds before retrying
        }
    }

    configure_serial_port(serial_fd);

    char buffer[256];  // Adjust the buffer size as needed
    ssize_t bytesRead;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (1) {
        bytesRead = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';  // Null-terminate the string
            std::cout << buffer;
            std::cout << std::flush;
            if (std::string(buffer).find(controller_identifier) != std::string::npos) {
				controller_state = true;
                std::cout << "Appropriate controller detected, STARTING...." << std::endl << std::flush;

                const char *startCommand = "START";
                ssize_t bytesWritten = write(serial_fd, startCommand, strlen(startCommand));
                if (bytesWritten == -1) {
                    perror("Error writing to serial port");
                    break;
                }
            }
        } else if (bytesRead == 0) {
            // No data available, continue reading
        } else {
            perror("Error reading from serial port");
            break;
        }
		if(!controller_state){
	        // Check if the timer exceeds 2 seconds
	        auto current_time = std::chrono::high_resolution_clock::now();
	        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
	        if (elapsed_time >= 2) {
				std::cerr << "Error: Timeout, switching to the next port." << std::endl;
				std::cout << "switching to next...\n";
	            close(serial_fd);
				serial_fd = auto_detect_serial_port(&current_port);
	            start_time = std::chrono::high_resolution_clock::now();
	        }
		}
    }

    close(serial_fd);

    return 0;
}
