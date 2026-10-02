export module aai.utils.timer;
import std;
export {
namespace utils{
namespace timer {
    using ms = std::chrono::duration<float, std::milli>;
    constexpr float MAX_FRAME_TIME = 1000.0f / 120.0f;
    std::chrono::high_resolution_clock timer;
    std::chrono::time_point frame_start = timer.now();
    std::chrono::time_point frame_end = timer.now();
    ms delta;
    float delta_ms = 0;
    float fps = 0;
    
    void update() {
        frame_start = timer.now();
    }
    void next_frame() {
        frame_end = timer.now();
        delta = (frame_end - frame_start);
        delta_ms = delta.count();
        ms fps_duration(MAX_FRAME_TIME);
        if (delta.count() <  MAX_FRAME_TIME)
            std::this_thread::sleep_for(fps_duration - delta);
        frame_end = timer.now();
        delta = (frame_end - frame_start);
        delta_ms = delta.count();
        // printf("fps: %f\n", delta_ms);
    }
}
}
};
