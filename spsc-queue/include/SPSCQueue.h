#pragma once
#include "Order.h"
#include <atomic>
#include <cstddef>
#include <array>

template<size_t Capacity>
class SPSCQueue
{
	private:
		std::atomic<size_t> head_;
		std::atomic<size_t> tail_;
		std::array<Order,Capacity> buffer_;
	public:
		SPSCQueue():head_(0),tail_(0){}

		bool push(const Order& o)
		{
			size_t temp = tail_.load(std::memory_order_relaxed);
			size_t next = (temp + 1) % Capacity;
			if (next == head_.load(std::memory_order_acquire)) 
				return false;
			else
			{
				buffer_[temp] = o;
				tail_.store(next, std::memory_order_release);
				return true;
			}
		}

		bool pop(Order& o)
		{
			size_t temp = head_.load(std::memory_order_relaxed);
			if (temp == tail_.load(std::memory_order_acquire))
				return false;
			else
			{
				o = buffer_[temp];
				size_t next = (temp + 1) % Capacity;
				head_.store(next, std::memory_order_release);
				return true;
			}
		}
};

