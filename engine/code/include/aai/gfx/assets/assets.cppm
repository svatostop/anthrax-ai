export module aai.gfx.assets;
import std;
export {
    namespace assets {
        template<typename T>
        class base {
            public:
                std::future<std::shared_ptr<T>> load_async(const std::string& path, std::function<std::shared_ptr<T>(std::string)> callback) {
                    return std::async(std::launch::async, [this, path, callback]() {
                        return load(path, callback);
                    });
                }                
                std::shared_ptr<T> load(const std::string& path, std::function<std::shared_ptr<T>(std::string)> callback) 
                {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    auto it = cache.find(path);
                    if (it != cache.end()) {
                        return it->second;
                    }
                    std::shared_ptr<T> resource = callback(path);
                    cache[path] = resource;
                    return resource;
                }
                void unload(const std::string& path) 
                {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    cache.erase(path);
                }
                void unload_if_unused() {
                    std::lock_guard<std::mutex> lock(cache_mutex);
                    auto it = cache.begin();
                    while (it != cache.end()) {
                    if (it->second.use_count() == 1) {
                        it = cache.erase(it);
                    } 
                    else {
                        ++it;
                    }
        }
    }
            private:
                std::map<std::string, std::shared_ptr<T>> cache;
                std::mutex cache_mutex;
        };
    }
};
