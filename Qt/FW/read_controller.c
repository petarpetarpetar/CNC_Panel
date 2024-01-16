#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define SERIAL_PORT "/dev/ttyUSB0"  // Replace with the correct serial port

int open_serial_port(const char *port) {
    int fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Error opening serial port");
        exit(EXIT_FAILURE);
    }
    return fd;
}

void configure_serial_port(int fd) {
    struct termios serial_port_settings;

    if (tcgetattr(fd, &serial_port_settings) == -1) {
        perror("Error getting serial port settings");
        exit(EXIT_FAILURE);
    }

    cfsetispeed(&serial_port_settings, B115200);  // Adjust to match the Arduino's baud rate
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

int main() {
    int serial_fd = open_serial_port(SERIAL_PORT);
    configure_serial_port(serial_fd);

    char buffer[100];  // Adjust the buffer size as needed
    ssize_t bytesRead;

    while (1) {
        bytesRead = read(serial_fd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';  // Null-terminate the string
            printf("Received from Arduino: %s\n", buffer);
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
