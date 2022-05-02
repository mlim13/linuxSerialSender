// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <iostream>
#include <string>
#include <vector>

class PortFailedException : public std::exception {
    virtual const char *what() const noexcept {
        return "Port failed to connect.";
    }
};

class SerialSender {
public:
    int port_fd; // file descriptor to the linux serial port
    struct termios tty; // termios struct
    SerialSender(std::string device_path) {
        // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
        int success = port_fd = open(device_path.c_str(), O_RDWR);
        if (success < 0) {
             perror("Could not open port");
             throw PortFailedException();
        }
        // Read in existing settings
        success = tcgetattr(port_fd, &tty);
        if (success != 0) {
            perror("Could not get attributes.");
            throw PortFailedException();
        }
        
        tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
        tty.c_cflag |= CS8; // 8 bits per byte (most common)
        tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
        
        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO; // Disable echo
        tty.c_lflag &= ~ECHOE; // Disable erasure
        tty.c_lflag &= ~ECHONL; // Disable new-line echo
        tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
        tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
        
        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
        // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)
        
        tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;
        
        // Set in/out baud rate to be 9600
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);
        
        // Save tty settings
        success = tcsetattr(port_fd, TCSANOW, &tty);
        if (success != 0) {
            perror("Could not set attributes.");
            throw PortFailedException();
        }

        tcflush(port_fd, TCIOFLUSH);
    }
    
    /* Sends 2 bytes to the serial port
        - [0] hex code associated with a drive message (0x90)
        - [1] signed 8-bit int associated with a drive value 
    */
    auto send_drive(float value) -> void;
    
    /* Sends 2 bytes to the serial port
        - [0] hex code associated with a steer message (0x91)
        - [1] signed 8-bit int associated with a steer value - Checked to be -5 <= x <= 5
    */
    auto send_steer(int32_t value) -> void;
    
    ~SerialSender() {
        close(port_fd);
    }
};
