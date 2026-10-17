module;
#include <stdio.h>
#include <bits/stdint-uintn.h>

module aai.utils.threads.job_sys;
import aai.utils.threads;
import std;
void utils::job_sys::execute(const std::function<void()>& job)
{
    current_label++;
	while (!job_pool.push_back(job)) { 
        wake_condition.notify_one();
        std::this_thread::yield();
    }

	wake_condition.notify_one(); 
}
void utils::job_sys::dispatch(uint32_t job_count, uint32_t group_size, const std::function<void(args)>& job)
{
    if (job_count == 0 || group_size == 0) {
		return;
	}
	const uint32_t group_count = (job_count + group_size - 1) / group_size;
	current_label += group_count;
	for (uint32_t group_index = 0; group_index < group_count; ++group_index) {
		const auto& job_group = [job_count, group_size, job, group_index]() {
			const uint32_t group_job_offset = group_index * group_size;
			const uint32_t group_job_end = std::min(group_job_offset + group_size, job_count);
            job_sys::args args;
			args.group_index = group_index;
			for (uint32_t i = group_job_offset; i < group_job_end; ++i)	{
				args.job_index = i;
				job(args);
			}
		};

		while (!job_pool.push_back(job_group)) {
            wake_condition.notify_one();
            std::this_thread::yield();
        }
		wake_condition.notify_one();
    }
}
bool utils::job_sys::is_busy()
{
    return finished_label.load() < current_label;
}
void utils::job_sys::wait()
{
    while (is_busy()) { 
        wake_condition.notify_one();
        std::this_thread::yield();
    }
}
