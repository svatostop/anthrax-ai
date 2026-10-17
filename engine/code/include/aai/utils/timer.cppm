export module aai.utils.timer;
import std;
export {
    constexpr float MAX_FRAME_TIME = 1000.0f / 120.0f;
namespace utils{
class timer {
    public:
    timer() {
        frame_start = time.now();
        frame_end = time.now();     
    }
    using ms = std::chrono::duration<float, std::milli>;
    std::chrono::high_resolution_clock time;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration> frame_start;
    std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::high_resolution_clock::duration>  frame_end;
    ms delta;
    float delta_ms = 0;
    float fps = 0;
    
    void update() {
        frame_start = time.now();
    }
    void next_frame() {
        frame_end = time.now();
        delta = (frame_end - frame_start);
        delta_ms = delta.count();
        ms fps_duration(MAX_FRAME_TIME);
        if (delta.count() <  MAX_FRAME_TIME)
            std::this_thread::sleep_for(fps_duration - delta);
        frame_end = time.now();
        delta = (frame_end - frame_start);
        delta_ms = delta.count();
        // printf("fps: %f\n", delta_ms);
    }
};
}
};
