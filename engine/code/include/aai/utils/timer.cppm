module;
#include <stdio.h>
export module aai.utils.timer;
import std;
export {
namespace utils{
namespace timer {
#define MAX_FPS 120
    std::chrono::high_resolution_clock timer;
    std::chrono::time_point end = timer.now();
    std::chrono::time_point start = timer.now();
    float delta = 0;
    float delta_ms = 0;
    float fps = 0;
    
    using ms = std::chrono::duration<float, std::milli>;
    void next_frame() {
        start = timer.now();
        delta = std::chrono::duration_cast<ms>(start - end).count();
        end = timer.now();
        while (delta <= 1000.0f / MAX_FPS) {
            start = timer.now();
            delta = std::chrono::duration_cast<ms>(start - end).count();
            delta_ms = delta;
        }
        fps = 1000.0f / delta;
        // printf("fps: %f\n", fps);
    }
}
}
};
