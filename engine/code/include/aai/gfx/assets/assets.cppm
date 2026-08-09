module;
#include <cstdint>

import aai.gfx.vk.device;
export module aai.gfx.assets;
import std;
export {
    namespace assets {
        template<typename T>
        class base {
            public:
                std::future<uint32_t> load_async(const std::string& path, std::function<void (std::shared_ptr<T>, const char*)> callback) {
                // uint32_t load_async(const char* path, std::function<void (std::shared_ptr<T>, const char*)> callback) {
                    return std::async(std::launch::async, [this, path, callback]() {
                        return load(path, callback);
                    });
                }                
                uint32_t load(const std::string& path, std::function<void (std::shared_ptr<T>, const char*)> callback) 
                {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    auto it = paths_id_map.find(path);
                    if (it != paths_id_map.end()) {
                        return it->second;
                    }
                    std::shared_ptr<T> resource(new T);
                    callback(resource, path.c_str());
                    counter++;
                    cache[counter] = resource;
                    paths_id_map[path] = counter;
                    return counter;
                }
                void unload(uint32_t id) 
                {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    delete cache[id];
                    cache.erase(id);
                    auto& it = std::find_if(paths_id_map.begin(), paths_id_map.end(), [id](const auto& a) { return a.second == id; });
                    paths_id_map.erase(it.first);
                }
                void unload_all(const vk::device::handlers& dev) {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    auto it = cache.begin();
                    while (it != cache.end()) {
                        if (it->second.use_count() == 1) {
                            it->second.get()->clean(dev);
                            it = cache.erase(it);
                        } 
                        else {
                            ++it;
                        }
                    }
                }
                std::shared_ptr<T> get(uint32_t n) { if (cache.find(n) != cache.end()) return cache[n]; return nullptr; }
            private:
                uint32_t counter = 0;
                std::map<uint32_t, std::shared_ptr<T>> cache;
                std::map<std::string, uint32_t> paths_id_map;
                std::mutex cache_mutex;
        };
    }
};
