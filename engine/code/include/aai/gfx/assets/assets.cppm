module;
#include <cstdint>

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
                    auto it = cache.find(path);
                    if (it != cache.end()) {
                        return std::distance(cache.begin(), it);// it->second;
                    }
                    std::shared_ptr<T> resource(new T);
                    callback(resource, path.c_str());
                    cache[path] = resource;
                    counter++;
                    return counter;
                }
                void unload(const std::string& path) 
                {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    delete cache[path];
                    cache.erase(path);
                }
                void unload_if_unused() {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    auto it = cache.begin();
                    while (it != cache.end()) {
                    if (it->second.use_count() == 1) {
                        delete it->second;
                        it = cache.erase(it);
                    } 
                    else {
                        ++it;
                    }
        }
    }
            private:
                uint32_t counter = 0;
                std::map<std::string, std::shared_ptr<T>> cache;
                std::mutex cache_mutex;
        };
    }
};
