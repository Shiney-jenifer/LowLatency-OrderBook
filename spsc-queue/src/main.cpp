#include "OrderBook.h"
#include "SPSCQueue.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include<chrono>
#include<thread>

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
    long orderCount = 0;
    SPSCQueue<1024> queue;
    OrderBook book(std::cout);
    std::atomic<bool> done(false);
    std::string line;
    auto start = std::chrono::high_resolution_clock::now();
    std::thread producer([&] {
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
                    if (type == 0)
                    {
                        Order o;
                        o.orderId = std::stoull(parts[1]);
                        o.side = (std::stoi(parts[2]) == 0) ? Side::Buy : Side::Sell;
                        o.quantity = std::stoul(parts[3]);
                        o.price = std::stoll(parts[4]);
                        o.msg = Message::Add;
                        while (!queue.push(o)) {}
                    }
                    else if (type == 1)
                    {
                        Order o;
                        o.orderId = std::stoull(parts[1]);
                        o.msg = Message::Cancel;
                        while (!queue.push(o)) {}
                    }
                }

            }
            catch (...)
            {
                std::cerr << "bad message: " << line << "\n";
            }
        }
        done.store(true);
    });
    std::thread consumer([&] {
        Order o;
        while (1)
        {
            if (queue.pop(o))
            {
                if (o.msg == Message::Add)
                {
                    book.add(o);
                }
                else
                {
                    book.cancel(o.orderId);
                }
                orderCount++;
            }
            else
            {
                if (done == true) break;
            }
        }

    });
    producer.join();
    consumer.join();
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "OrderCount:" << orderCount << " " << "Time:" << elapsed << "\n";
    return 0;
}
