export module aai.keeper.camera;
import aai.keeper.entity; 
import glm;
export {
    namespace keeper {
        class camera : public entity
        {
            public:
                enum class type {
                    EDITOR = 0,
                };

                camera()  { t = type::EDITOR; };
                void set_position(const glm::vec3& pos) { position = pos; set_directions();};
                void set_window_size(const glm::ivec2& w) { window_size = w; }
                
                const glm::mat4& get_view() const { return view; }
                const glm::mat4& get_proj() const { return projection; }
                const glm::mat4& get_reverse_proj() const { return reverse_projection; }
                const glm::vec3& get_dir() const { return direction; }
                const glm::vec3& get_pos() const { return position; }
                const glm::vec3& get_front() const { return front; }
                const glm::vec3& get_right() const { return right; }
                float get_yaw() const { return yaw; }
                float get_pitch() const { return pitch; }
                const glm::vec3& get_up() const { return up; }

                void update() override { update_movement(); update_directions(); update_matrices(); };
            private:
                void set_directions();
                void update_movement();
                void update_directions();
                void update_matrices();
                type t;

                glm::vec3 position;
                glm::vec3 target = glm::vec3(0.0f, 0.0f, 1.0f);
                glm::vec3 direction;
                glm::vec3 right;
                glm::vec3 front;
                glm::vec3 up;

                glm::vec3 world_up;

                glm::mat4 view;
                glm::mat4 projection;
                glm::mat4 reverse_projection;
                glm::mat4 rotation;
                
                glm::ivec2 window_size;

                float yaw = -90.0f;
                float pitch = 0.0f;
            
                // Keeper::Gizmo* GizmoHandle = nullptr;
        };
    }
};
