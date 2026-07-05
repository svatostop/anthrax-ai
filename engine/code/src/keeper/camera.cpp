module;
#include <stdio.h>
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"

module aai.keeper.camera;
import aai.io;
import aai.utils.timer;
void keeper::camera::set_directions()
{
    direction = glm::normalize(position);
    world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    front = glm::vec3(0.0f, 0.0f, -1.0f);
    right = glm::normalize(glm::cross(world_up, direction));
    up = glm::vec3(0.0f, 1.0f,  0.0f);
    right = glm::normalize(glm::cross(direction, up));
    front = -direction;
}

void keeper::camera::update_matrices()
{
    view = glm::lookAt(get_pos(), get_pos() + get_front(), get_up());
	projection = glm::perspective(glm::radians(45.f), (window_size.x / (float)window_size.y), 1.0f, 1000.0f);
	projection[1][1] *= -1;
}

void keeper::camera::update_movement()
{
    float delta = 0.001 * utils::timer::delta_ms;
    if (aai::io::key::is_state(aai::io::key::val::W)) {
        position += delta * front;
    }
    if (aai::io::key::is_state(aai::io::key::val::S)) {
        position -= delta * front;
    }
    if (aai::io::key::is_state(aai::io::key::val::A)) {
        position -= glm::normalize(glm::cross(front, up)) * delta;
    }
    if (aai::io::key::is_state(aai::io::key::val::D)) {
        position += glm::normalize(glm::cross(front, up)) * delta;
    }
}

void keeper::camera::update_directions()
{
    float delta = 0.0000005 * utils::timer::delta_ms;
    if (!aai::io::mouse::is_state(aai::io::mouse::val::LEFT_PRESSED))
        return;
    glm::vec2 pos_delta = aai::io::mouse::get_delta();

    float rot_speed = delta;
    
    float yaw = rot_speed * pos_delta.x;
    float pitch = rot_speed * pos_delta.y;

    rotation = glm::rotate(glm::mat4(1.0), -yaw, glm::vec3(0.f, 1.f, 0.f));
    rotation = glm::rotate(rotation, -pitch, right);
    yaw = yaw;
    pitch = pitch;

    direction = glm::normalize(glm::mat3(rotation) * direction);
    up = glm::normalize(glm::mat3(rotation) * up);
    right = glm::normalize(glm::cross(direction, up));
    front = -direction;
}
