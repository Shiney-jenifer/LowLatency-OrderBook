#include "Engine.h"
#include <iostream>
#include <string>

int main() {
    Engine engine(std::cout);
    std::string line;
    while (std::getline(std::cin, line)) {
        // Strip Windows-style carriage return if present
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty()) continue;
        engine.processLine(line);
    }
    return 0;
}
