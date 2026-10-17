module;
#include <stdio.h>
#include <bits/stdint-uintn.h>

export module aai.utils.threads.job_sys;
import std;

export {
    namespace utils {
        namespace job_sys {
            struct args {
	            uint32_t job_index;
	            uint32_t group_index;
            };

            template <typename T, size_t capacity>
            class safe_ring_buffer {
            public:
            	inline bool push_back(const T& item) {
            		bool result = false;
            		lock.lock();
            		size_t next = (head + 1) % capacity;
            		if (next != tail) {
            			data[head] = item;
            			head = next;
            			result = true;
            		}
            		lock.unlock();
            		return result;
            	}
        
            	inline bool pop_front(T& item) {
            		bool result = false;
            		lock.lock();
            		if (tail != head) {
            			item = data[tail];
            			tail = (tail + 1) % capacity;
            			result = true;
            		}
            		lock.unlock();
            		return result;
            	}
            private:
            	T data[capacity];
        	    size_t head = 0;
        	    size_t tail = 0;
        	    std::mutex lock;
            };

            void execute(const std::function<void()>& job);	
            void dispatch(uint32_t job_count, uint32_t group_size, const std::function<void(args)>& job);
            bool is_busy();
            void wait();

            safe_ring_buffer<std::function<void()>, 256> job_pool;	
            std::condition_variable wake_condition;
            std::mutex wake_mutex;
            uint64_t current_label = 0;
            std::atomic<uint64_t> finished_label;
        };
    }
};
