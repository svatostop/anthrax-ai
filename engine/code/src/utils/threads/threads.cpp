module;
#include <stdio.h>
#include <bits/stdint-uintn.h>
#include <string>

module aai.utils.threads;
import std;

void utils::threads::stop() 
{
    back_data->stop();
    logic_thread.stop();
    render_thread.stop();
    job_sys::wait();
}
void utils::threads::init()
{
	job_sys::finished_label.store(0);
	auto num_cores = std::thread::hardware_concurrency();
	num_threads = std::max(1u, num_cores - 2);
	for (uint32_t thread_id = 0; thread_id < num_threads; ++thread_id) {
		std::thread worker([thread_id] {
			std::function<void()> job;
            using clock = std::chrono::steady_clock;
            while (true) {
				if (job_sys::job_pool.pop_front(job)) {
                    auto frame_start = clock::now();
					job();
                    auto frame_end = clock::now();
                    job_sys::finished_label.fetch_add(1);
                    thread_dbg::get()->set_time("JOB_" + std::to_string(thread_id), std::chrono::duration<float, std::milli>(frame_end - frame_start).count());
                    thread_dbg::get()->set_state("JOB_" + std::to_string(thread_id), thread_state::val::RUNNING);
                }
				else {
					std::unique_lock<std::mutex> lock(job_sys::wake_mutex);
					job_sys::wake_condition.wait(lock);
                    thread_dbg::get()->set_state("JOB_" + std::to_string(thread_id), thread_state::val::IDLE);
                    thread_dbg::get()->set_time("JOB_" + std::to_string(thread_id),0.0f);
				}
			}
		});
		worker.detach(); 
    }
}

void utils::threads::start(std::function<void(float delta_ms)>&& render_loop_callback, std::function<void(float delta_ms)>&& logic_loop_callback)
{
    back_data = std::make_unique<back_data_swapchain>();
    render_thread.start(std::move(render_loop_callback));
    logic_thread.start(std::move(logic_loop_callback));
}

void utils::render_threads::run()
{
    while (running) {
        time.update();
        callback(time.delta_ms);
        time.next_frame();
        thread_dbg::get()->set_state("RENDER", thread_state::RUNNING);
        thread_dbg::get()->set_time("RENDER", time.delta_ms);
    }
}
void utils::render_threads::stop()
{
    running = false;

    if (local_thread.joinable())
        local_thread.join();
}

void utils::logic_threads::run()
{
    while (running) {
        time.update();
        callback(time.delta_ms);
        time.next_frame();
        thread_dbg::get()->set_state("LOGIC", thread_state::RUNNING);
        thread_dbg::get()->set_time("LOGIC", time.delta_ms);
    }
}
