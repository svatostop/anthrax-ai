import aai.utils.mem;
import std;
void utils::mem::push(utils::mem::event e, utils::mem::type t, std::function<void()>&& function)
{
    events[static_cast<int>(e)].push_back(pair_event(static_cast<int>(t), function));
}
