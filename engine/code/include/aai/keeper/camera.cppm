export module aai.keeper.camera;
import glm;
export {
    namespace keeper {
        class camera
        {
            public:
                enum class type {
                    EDITOR = 0,
                };

                camera() {};
                camera(type info, glm::vec3 pos)
                    : t(info), position(pos.x, pos.y, pos.z) { set_directions(); }


                void set_position(glm::vec3 pos) { position = pos; set_directions();};
                void set_directions();

                glm::vec3 get_dir() const { return direction; }
                glm::vec3 get_pos() const { return position; }
                glm::vec3 get_front() const { return front; }
                glm::vec3 get_right() const { return right; }
                float get_yaw() const { return yaw; }
                float get_pitch() const { return pitch; }
                glm::vec3 get_up() const { return up; }

                // TODO make private - figure out something
                void update() { update_movement(); update_directions(); };

                // void SetSelected(bool id) override { }
                // void SetGizmo(Keeper::Objects* gizmo) override { GizmoHandle = reinterpret_cast<Keeper::Gizmo*>(gizmo); }

                // Keeper::Type GetType() const override { return ObjectType; }
                // uint32_t CameraType() const override { return static_cast<uint32_t>(Type); }

            private:
                void update_movement();
                void update_directions();
                type t;

                glm::vec3 position;
                glm::vec3 target = glm::vec3(0.0f, 0.0f, 1.0f);
                glm::vec3 direction;
                glm::vec3 right;
                glm::vec3 front;
                glm::vec3 up;

                glm::vec3 world_up;

                glm::mat4 view;
                glm::mat4 rotation;

                float yaw = -90.0f;
                float pitch = 0.0f;
            
                // Keeper::Gizmo* GizmoHandle = nullptr;
        };
    }
};
