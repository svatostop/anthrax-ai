import aai.utils.mem;
import std;

void utils::mem::push(utils::mem::event e, utils::mem::type t, std::function<void()>&& function)
{
    events[static_cast<int>(e)].push_back(pair_event(static_cast<int>(t), function));
}
void utils::mem::flush(utils::mem::event e, utils::mem::type t)
{
    auto it = std::remove_if(
        events[static_cast<int>(e)].begin(), events[static_cast<int>(e)].end(),
        [&](const event_pair &pair) {
            if (pair.first ==static_cast<int>(t)) {
                pair.second();
                return true;
            }
            else {
                return false;
            }});
    events[static_cast<int>(e)].resize(std::distance(events[static_cast<int>(e)].begin(), it));
}
