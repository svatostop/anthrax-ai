module aai.keeper.camera;

void keeper::camera::set_directions()
{
    direction = glm::normalize(position);
    world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    front = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::normalize(glm::cross(world_up, direction));
    up = glm::vec3(0.0f, 1.0f,  0.0f);
}

void keeper::camera::update_movement()
{
    float delta = 0.001;// Utils::Debug::GetInstance()->DeltaMs;
    // bool half_speed = Utils::Debug::GetInstance()->HalfSpeed;
// #ifdef AAI_LINUX
    const float camspeed = delta * 0.005;// (half_speed ? 0.005 : 0.05);
// #else
    // const float camspeed = 0.05f * delta;
// #endif
    // if (Core::WindowManager::GetInstance()->GetPressedKey() == W_KEY) {
    //     Position += camspeed * Front;
    // }
    // if (Core::WindowManager::GetInstance()->GetPressedKey() == S_KEY) {
    //     Position -= camspeed * Front;
    // }
    // if (Core::WindowManager::GetInstance()->GetPressedKey() == A_KEY) {
    //     Position -= glm::normalize(glm::cross(Front, Up)) * camspeed;
    // }
    // if (Core::WindowManager::GetInstance()->GetPressedKey() == D_KEY) {
    //     Position += glm::normalize(glm::cross(Front, Up)) * camspeed;
    // }
}

void keeper::camera::update_directions()
{
    float delta = 0.001;//Utils::Debug::GetInstance()->DeltaMs;
    // Vector2<int> mousemove = Core::WindowManager::GetInstance()->GetMouseDelta();
    // if (mousemove.x == 0 && mousemove.y == 0) return;
    // float rotspeed = delta * 0.00005;
    //
    // if (mousemove.x || mousemove.y) {
    //     float yaw = rotspeed * mousemove.x;
    //     float pitch = rotspeed * mousemove.y;
    //
    //     Rotation = glm::rotate(glm::mat4(1.0), -yaw, glm::vec3(0.f, 1.f, 0.f));
    //     Rotation = glm::rotate(Rotation, -pitch, Right);
    //     Yaw = yaw;
    //     Pitch = pitch;
    //
    //     Direction = glm::normalize(glm::mat3(Rotation) * Direction);
    // Up = glm::normalize(glm::mat3(Rotation) * Up);
    // Right = glm::normalize(glm::cross(Direction, Up));
    // Front = -Direction;
    // }

}
