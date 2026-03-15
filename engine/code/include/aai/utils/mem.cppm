module;
#include <cstdint>

export module aai.utils.mem;
export import aai.utils;
import std;
export {
    namespace utils {
        typedef std::pair<int, std::function<void()>> event_pair;
        event_pair pair_event(int n, std::function<void()> function) { return std::make_pair(n, function); }
        class mem : public utils::singleton<mem> {
            public:
                enum class event {
                    DELETE = 0,
                    SIZE
                };
                enum class type {
                    NONE = 0,
                    VK,
                };
                void push(event e, type t, std::function<void()>&& function); 
                void flush(event e, type t); 
            private:
                std::deque<event_pair> events[static_cast<uint32_t>(event::SIZE)];
        };
    }
};

