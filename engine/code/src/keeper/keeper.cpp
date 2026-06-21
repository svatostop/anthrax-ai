
module aai.keeper;

void keeper::base::init(glm::vec3 camera_pos)
{
    cam = std::make_shared<camera>(keeper::camera::type::EDITOR, camera_pos);
}
