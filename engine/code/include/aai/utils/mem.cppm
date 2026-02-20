module;
#include <cstdint>

export module aai.utils.mem;
export import aai.utils;
import std;
export {
    namespace utils {
        typedef std::pair<int, std::function<void()>> event_pair;
        class mem : public utils::singleton<mem> {
            public:
                enum class event {
                    DELETE = 0,
                    SIZE
                };
                enum class type {
                    GENERAL = 0,
                };
                void push(event e, type t, std::function<void()>&& function) {} 
            private:
                 event_pair pair_event(int n, std::function<void()> function) { return std::make_pair(n, function); }
                std::deque<event_pair> events[static_cast<uint32_t>(event::SIZE)];
        };
    }
};

