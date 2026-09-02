#include "Engine.h"
#include <iostream>

bool Engine::processLine(const std::string& line) {
    InputMessage msg = parseMessage(line);

    if (msg.type == MessageType::Invalid)
        return false;

    if (msg.type == MessageType::Add) {
        const AddOrderRequest& req = msg.add;
        if (knownIds_.count(req.orderId)) {
            std::cerr << "ERROR: duplicate orderId " << req.orderId << "\n";
            return false;
        }
        knownIds_.insert(req.orderId);
        book_.add(req, ++seqNum_);

    } else if (msg.type == MessageType::Cancel) {
        book_.cancel(msg.cancel);
    }

    return true;
}
