#include "Messages.h"
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iostream>

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string token;
    while (std::getline(ss, token, delim))
        parts.push_back(token);
    return parts;
}

InputMessage parseMessage(const std::string& line) {
    InputMessage result;   // type = Invalid by default

    if (line.empty()) return result;

    std::vector<std::string> parts = split(line, ',');
    if (parts.empty()) {
        std::cerr << "ERROR: empty message\n";
        return result;
    }

    int msgType = -1;
    try { msgType = std::stoi(parts[0]); }
    catch (...) {
        std::cerr << "ERROR: unknown message type: " << line << "\n";
        return result;
    }

    try {
        if (msgType == 0) {
            if (parts.size() != 5)
                throw std::runtime_error("AddOrderRequest requires 5 fields");

            AddOrderRequest req;
            req.orderId  = static_cast<uint64_t>(std::stoull(parts[1]));
            int side     = std::stoi(parts[2]);
            if (side != 0 && side != 1)
                throw std::runtime_error("side must be 0 or 1");
            req.side     = static_cast<Side>(side);
            req.quantity = static_cast<uint32_t>(std::stoul(parts[3]));
            req.price    = std::stod(parts[4]);
            if (req.orderId == 0)  throw std::runtime_error("orderId must be positive");
            if (req.quantity == 0) throw std::runtime_error("quantity must be positive");
            if (req.price <= 0)    throw std::runtime_error("price must be positive");

            result.type = MessageType::Add;
            result.add  = req;

        } else if (msgType == 1) {
            if (parts.size() != 2)
                throw std::runtime_error("CancelOrderRequest requires 2 fields");

            CancelOrderRequest req;
            req.orderId = static_cast<uint64_t>(std::stoull(parts[1]));

            result.type   = MessageType::Cancel;
            result.cancel = req;

        } else {
            std::cerr << "ERROR: unknown message type: " << line << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: malformed message (" << e.what() << "): " << line << "\n";
        result.type = MessageType::Invalid;
    }

    return result;
}
