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
#include <assert.h>

#include "sendSerialCommands.h"

auto SerialSender::send_drive(float value) -> void {
    if (not (value >= 0.23 and value < 4.73)) {
        std::cout << "Drive value must be between 0.23 and 4.73. Command not sent." << std::endl;
        return;
    }
    assert(sizeof(float) == 4);
    uint8_t bytes[5];
    // We need to represent the float value as bits to perform bit shifts
    // We do bitshifts rather than memcpy to avoid endianness issues
    union {
        float f;
        uint32_t u;
    } fu = { .f = value };
    bytes[0] = (char)0x90;
    bytes[1] = (fu.u >> 24) & 0xFF;
    bytes[2] = (fu.u >> 16) & 0xFF;
    bytes[3] = (fu.u >> 8) & 0xFF;
    bytes[4] = fu.u & 0xFF;
    // tcflush(port_fd, TCIOFLUSH); // Discard any residual data to get a clean buffer

    int num_bytes = write(port_fd, bytes, 5);
    if (num_bytes < 0) {
        std::cout << "Could not write to port." << std::endl;
        return;
    }
    //tcdrain(port_fd);
    std::cout << num_bytes << " bytes sent. Sent drive message: " << (float)value << std::endl;
}

auto SerialSender::send_steer(int32_t value) -> void {
    if (not (value >= -5 and value <= 5)) {
        std::cout << "Value must be between -5 and 5 inclusive. Command not sent." << std::endl;
        return;
    }
    uint8_t bytes[5];
    bytes[0] = (char)0x91;
    bytes[1] = (value >> 24) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 8) & 0xFF;
    bytes[4] = value & 0xFF; 
    // tcflush(port_fd, TCIOFLUSH); // Discard any residual data to get a clean buffer
    int num_bytes = write(port_fd, bytes, 5);
    if (num_bytes < 0) {
        std::cout << "Could not write to port." << std::endl;
        return;
    }
    //tcdrain(port_fd);
    std::cout << num_bytes  << " btyes sent. Sent steer message: " << (int)value << std::endl;
}
