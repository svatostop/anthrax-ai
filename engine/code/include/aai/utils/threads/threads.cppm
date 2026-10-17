module;
#include <stdio.h>
#include <bits/stdint-uintn.h>

export module aai.utils.threads;
export import aai.utils;
export import aai.utils.timer;
import aai.utils.threads.job_sys;
import std;

export {
    namespace utils {
        class threads_interface {
            public:
                void start(std::function<void(float delta_ms)>&& loop_callback) {
                    running = true;
                    callback = loop_callback;
                    local_thread = std::thread([this]() {
                        run();
                    });
                }
                virtual void run() = 0;
                virtual void stop() {
                    running = false;
                    if (local_thread.joinable())
                        local_thread.join();
                }
            private:
                uint32_t num_threads = 0;
            protected:
                std::function<void(float delta_ms)> callback;
                std::thread local_thread;
                std::atomic<bool> running{false};
                
                timer time;
        };

        class logic_threads : public threads_interface {
            public:
                void run() override;
            private:
        };
        class render_threads : public threads_interface {
            public:
                void run() override;
                void stop() override;
            private:
                std::condition_variable backdata_changed;
        };
        class threads {
            public:
                threads() { init(); }
                ~threads() { stop(); }
                
                void start(std::function<void(float delta_ms)>&& render_loop_callback, std::function<void(float delta_ms)>&& logic_loop_callback);
                void stop();
            private:
                uint32_t num_threads = 0;
                logic_threads logic_thread;
                render_threads render_thread;
                void init();
        };

    }
};
