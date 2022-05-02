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
#include <fstream>
#include <unistd.h>

#include "sendSerialCommands.h"

int main(int argc, char* argv[]) {
    if (argc != 3 or (std::string(argv[2]) != "1" and std::string(argv[2]) != "2")) {
        std::cout << "Incorrect usage." << std::endl;
        std::cout << "Correct usage: <executable> <path_to_port> <mode>" << std::endl;
        std::cout << "    <executable> = the name/path of executable. eg. ./sendSerialCommands" << std::endl;
        std::cout << "    <path_to_port> = the path to the port. Typically '/dev/ttyACM0' on RPi." << std::endl;
        std::cout << "    <mode> = Either 1 or 2" << std::endl;
        std::cout << "        1 - reads commands dynamically from console" << std::endl;
        std::cout << "        2 - reads commands from hardcoded file" << std::endl;
        return 1;
    }
    auto serial_sender = SerialSender(argv[1]);
  
    auto input = std::string{};
    auto arg2 = std::string(argv[2]);
    auto file_stream = std::ifstream("/mnt/c/Users/matth/Developer/Uni/thesis/linuxSerialC/commands.txt"); // Absolute path so we can find it on boot
    while (input != "close" ) {
        if (arg2 == "1") {
            std::cout << "Enter command: ";
            getline(std::cin, input);
        } else if (arg2 == "2") {
            usleep(300000); // 0.3 seconds. Additional sleep time included in file
            getline(file_stream, input);
            if (file_stream.eof()) break;
        }

        auto delimiter = std::string{" "};
    
        // Parsing input
        auto input_tokens = std::vector<std::string>{};
        size_t pos = 0;
        auto token = std::string{};
        while ((pos = input.find(delimiter)) != std::string::npos) {
            token = input.substr(0, pos);
            input_tokens.push_back(token);
            input.erase(0, pos + delimiter.length());
        }
        input_tokens.push_back(input);
        
        if (input_tokens.size() != 2) {
            std::cout << "Incorrect usage" << std::endl;
            std::cout << "Correct usage: <command> <value>" << std::endl;
            continue;
        }
        
        if (input_tokens[0] == "drive") {
            float value = (float)atof(input_tokens[1].c_str());
            serial_sender.send_drive(value);
        }
        else if (input_tokens[0] == "steer") {
            int32_t value = (int32_t)atoi(input_tokens[1].c_str());
            serial_sender.send_steer(value);
        }
        // Sleep only works in Mode 2 (reading from file)
        else if (input_tokens[0] == "sleep" and arg2 == "2") {
            float value = (float)atof(input_tokens[1].c_str());
            usleep(1000000 * value);
        }
        else {
            std::cout << "Invalid input: Command must be 'drive', 'steer' or 'sleep' (only in Mode 2)" << std::endl;
        }
    }
    file_stream.close();
    return 0; // success
};
