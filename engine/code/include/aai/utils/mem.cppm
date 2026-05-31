module;
#include <cstdint>
#include <cstddef>

export module aai.utils.mem;
export import aai.utils;
import std;
export {
    namespace utils {
        typedef std::pair<int, std::function<void()>> event_pair;
        event_pair pair_event(int n, std::function<void()> function) { return std::make_pair(n, function); }
        class mem : public utils::singleton<mem> {
            public:
                struct stats {
                    size_t bytes;
                    uint32_t allocated = 0;
                    uint32_t deallocated = 0;
                };
                enum class resource {
                    TEXTURE = 0,
                    RENDER_TARGET,
                    MESH
                };
                typedef std::unordered_map<resource, std::unordered_map<std::string, stats>> stats_map;
                enum class event {
                    DELETE = 0,
                    SIZE
                };
                enum class type {
                    NONE = 0,
                    VK,
                    VK_DEVICE,
                    VK_SWAPCHAIN,
                    VK_INSTANCE
                };
                void push(event e, type t, std::function<void()>&& function); 
                void flush(event e, type t);
                void flush_all(event e);

                void track_allocation(resource t, const std::string& tag, size_t bytes) {
                    utils::ASSERT(tag.empty(), "tag of resource can't be empty");                    
                    stats_mem[t][tag].bytes = bytes;
                    stats_mem[t][tag].allocated++;
                }
                void track_deallocation(resource t, const std::string& tag) {
                    utils::ASSERT(tag.empty(), "tag of resource can't be empty");                    
                    stats_mem[t][tag].deallocated++;
                }

            private:
                std::deque<event_pair> events[static_cast<uint32_t>(event::SIZE)];
                
                stats_map stats_mem;
                size_t totally_allocated_bytes = 0;
        };
    }
};

