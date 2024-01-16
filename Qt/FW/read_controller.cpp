#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstdlib> // for exit function
#include <vector>

#define MAX_PORT_NAME_LEN 20

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

    cfsetispeed(&serial_port_settings, B57600);  // Adjust to match the Arduino's baud rate
    cfsetospeed(&serial_port_settings, B57600);

    serial_port_settings.c_cflag &= ~PARENB;
    serial_port_settings.c_cflag &= ~CSTOPB;
    serial_port_settings.c_cflag &= ~CSIZE;
    serial_port_settings.c_cflag |= CS8;

    if (tcsetattr(fd, TCSANOW, &serial_port_settings) == -1) {
        perror("Error setting serial port settings");
        exit(EXIT_FAILURE);
    }
}

int auto_detect_serial_port() {
    std::vector<std::string> possible_ports = {"/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2", "/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyACM2"};

    for (const auto &port : possible_ports) {
        int fd = try_open_serial_port(port.c_str());
        if (fd != -1) {
            return fd;  // Found a connected port
        }
    }

    return -1;  // No connected port found
}


int main() {
    int serial_fd = auto_detect_serial_port();
    if (serial_fd == -1) {
        std::cerr << "Error: Unable to detect connected serial port." << std::endl;
        exit(EXIT_FAILURE);
    }

    configure_serial_port(serial_fd);

    char buffer[100];  // Adjust the buffer size as needed
    ssize_t bytesRead;

    while (1) {
        bytesRead = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';  // Null-terminate the string
            std::cout << buffer;
        } else if (bytesRead == 0) {
            // No data available, continue reading
        } else {
            perror("Error reading from serial port");
            break;
        }
    }

    close(serial_fd);

    return 0;
}
