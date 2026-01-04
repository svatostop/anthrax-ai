#include "anthraxAI/utils/thread.h"
#include <queue>
#include <thread>
#include <utility>

void Thread::Pool::Process(const Task& task)
{
#ifdef TRACY
    ZoneScopedN("Thread::Pool::Process");
#endif
    switch (task.type) {
        case Thread::Task::Type::EXECUTE: {
            if (task.name == Thread::Task::Name::RENDER) {
                task.func1();
            }
            else if (task.name == Thread::Task::Name::ANIM) {
                task.func2(task.model, task.scene);
            }
            else if (task.name == Thread::Task::Name::ANIM_NODE) {
                task.anim_node(task.node, task.scene, task.i);
            }
            else {
                if (task.info && task.func) {
                    task.func(task.i, task.info);
                }
            }
            break;
        }
        case Thread::Task::Type::STOP:
            return;
    }
}

void Thread::Pool::WaitRender()
{
    for (int i = 0; i < RenderThreads.size(); i++) {
        std::unique_lock<std::mutex> lock(RenderMutex[i]);
        RenderCondition[i].wait(lock, [this, i]{ return RenderQueue[i].empty(); });
    }
}

void Thread::Pool::WaitWork()
{
#ifdef TRACY
    ZoneScopedN("Thread::Pool::WaitWork");
#endif
    for (int i = 0; i < Threads.size(); i++) {
        std::unique_lock<std::mutex> lock(Mutex[i]);
        WorkCondition[i].wait(lock, [this, i]{ return Queue[i].empty(); });
    }
}

void Thread::Pool::WorkRender(int id)
{
#ifdef TRACY
    ZoneScopedN("Thread::Pool::WorkRender");
#endif
    while (!Done) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(RenderMutex[id]);
            std::queue<Task>& queue = RenderQueue[id];
            RenderCondition[id].wait(lock, [this, &id]{ return !RenderQueue[id].empty() || Done; });
            if (Done) {
                break;
            }
            task = RenderQueue[id].front();
           // RenderQueue.pop();
        }
        //if (!OnPause) {
            Process(task);
    {
        std::lock_guard<std::mutex> lock(RenderMutex[id]);
		RenderQueue[id].pop();
		RenderCondition[id].notify_one();
        }
            //}
    }
}

void Thread::Pool::Work(int id)
{
#ifdef TRACY
    ZoneScopedN("Thread::Pool::Work");
#endif
    while (!Done) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(Mutex[id]);
            WorkCondition[id].wait(lock, [this, &id]{ return !Queue[id].empty() || Done; });
            if (Done) {
                break;
            }
            task = Queue[id].front();
        }
        //if (!OnPause) {
            Process(task);
        //}
        {
            std::unique_lock<std::mutex> lock(Mutex[id]);
            Queue[id].pop();
            WorkCondition[id].notify_one();
        }
    }
}

void Thread::Pool::Init(int num)
{
    if (num > std::thread::hardware_concurrency() || num > MAX_WORK_THREAD_NUM) {
        num = std::thread::hardware_concurrency() / 4;// MAX_WORK_THREAD_NUM;
    }
    Threads.reserve(num);
    Queue.resize(num);
    for (int i = 0; i < num; ++i) {
        Threads.emplace_back(std::thread(&Thread::Pool::Work, this, i));
    }

    printf("MAX THREADS = %d \n", std::thread::hardware_concurrency());
    RenderQueue.resize(MAX_RENDER_THREAD_NUM);
    RenderThreads.reserve(MAX_RENDER_THREAD_NUM);
    for (int i = 0; i < MAX_RENDER_THREAD_NUM; ++i) {
        RenderThreads.emplace_back(std::thread(&Thread::Pool::WorkRender, this, i));
    }
}

void Thread::Pool::Reload()
{
    if (Threads.empty() || Done) {
        return;
    }
    Done = true;
    OnPause = false;
    ThreadCounter = 0;
    int i = 0;
    for (std::thread& thread : Threads) {
        WorkCondition[i].notify_all();
        thread.join();
        i++;
    }
    i = 0;
    for (std::thread& thread : RenderThreads) {
        RenderCondition[i].notify_all();
        thread.join();
        i++;
    }

    Done = false;
    Threads.clear();
    RenderThreads.clear();
    RenderQueue.clear();
    Queue.clear();
    Init(MAX_WORK_THREAD_NUM);
}

void Thread::Pool::Stop()
{
    if (Threads.empty() || Done) {
        return;
    }
    Done = true;

    int i = 0;
    for (std::thread& thread : Threads) {
        WorkCondition[i].notify_all();
        if (thread.joinable()) {
            thread.join();
        }
        i++;
    }
    i = 0;
    for (std::thread& thread : RenderThreads) {
        RenderCondition[i].notify_all();
        if (thread.joinable()) {
            thread.join();
        }
        i++;
    }
}

bool Thread::Pool::Push(const Task& task)
{
    if (Threads.empty()) {
        return false;
    }
    if (ThreadCounter ==  Threads.size()) { //std::thread::hardware_concurrency()) { //MAX_WORK_THREAD_NUM) {
        ThreadCounter = 0;
    }
    std::lock_guard<std::mutex> lock(Mutex[ThreadCounter]);

    Queue[ThreadCounter].push(task);
    WorkCondition[ThreadCounter].notify_one();
    ThreadCounter++;
    return true;
}

bool Thread::Pool::PushByID(int id, const Task& task)
{
    if (RenderThreads.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(RenderMutex[id]);

    RenderQueue[id].push(task);
    RenderCondition[id].notify_all();

    return true;
}
