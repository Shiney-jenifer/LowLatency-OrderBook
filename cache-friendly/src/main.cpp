#include "OrderBook.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// helper: split "0,1,1,5,100" → ["0","1","1","5","100"]
std::vector<std::string> split(const std::string& line, char delim) {
    std::string piece;
    std::vector<std::string> parts;
    for (size_t i = 0;i < line.size();i++)
    {
        if (line[i] == delim)
        {
            parts.push_back(piece);
            piece.clear();
        }
        else
        {
            piece += line[i];
        }
    }
    parts.push_back(piece);
    return parts;
}

int main() 
{
    OrderBook book(std::cout);
    std::string line;
    while (std::getline(std::cin, line)) 
    {
        auto parts = split(line, ',');
        try 
        {
            if (parts.empty()) 
                throw std::runtime_error("bad");
            else
            {
                int type = std::stoi(parts[0]);
                if(type == 0)
                {
                    Order o;
                    o.orderId = std::stoll(parts[1]);
                    o.side = (std::stoi(parts[2]) == 0) ? Side::Buy : Side::Sell;
                    o.quantity = std::stoul(parts[3]);
                    o.price = std::stoll(parts[4]);
                    book.add(o);
                }
                else if (type == 1)
                {
                    book.cancel(std::stoull(parts[1]));
                }
            }

        }
        catch(...)
        {
            std::cerr << "bad message: " << line << "\n";
        }
    }
    return 0;
}
