module;
#include <stdio.h>
#include <bits/stdint-uintn.h>

export module aai.utils.threads;
export import aai.utils;
export import aai.utils.timer;

import aai.keeper.types;
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

        class back_data_swapchain {
            public:
                void stop() { data_ready = true; backdata_changed.notify_one(); }

                void submit_data(std::deque<keeper::back_datas>&& d) {
                    std::lock_guard<std::mutex> lock(backdata_mutex);
                    for (keeper::back_datas& val : d) {
                        data.push_back(val);
                    }
                    data_ready = true;
                    d.clear();
                    backdata_changed.notify_one();
                }
                std::deque<keeper::back_datas>& get_data() {
                    std::unique_lock<std::mutex> lock(backdata_mutex);
                    backdata_changed.wait(lock, [&]{ return data_ready; });
                    data_ready = false;
                    return data;
                }

            private:
                std::deque<keeper::back_datas> data;
                bool data_ready = false;
                std::mutex backdata_mutex;
                std::condition_variable backdata_changed;
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
        };

        class threads {
            public:
                threads() { init(); }
                ~threads() { stop(); }
                
                void start(std::function<void(float delta_ms)>&& render_loop_callback, std::function<void(float delta_ms)>&& logic_loop_callback);
                void stop();
                
                void submit_data(std::deque<keeper::back_datas>& d) { back_data->submit_data(std::move(d)); }
                std::deque<keeper::back_datas>& get_back_data() { return back_data->get_data(); }
            private:
                uint32_t num_threads = 0;
                logic_threads logic_thread;
                render_threads render_thread;
                
                std::unique_ptr<back_data_swapchain> back_data; 
                void init();
        };

    }
};
