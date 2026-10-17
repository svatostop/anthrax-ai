export module aai.keeper.entity;
import std;
export {
    namespace keeper {
        class entity {
            public:
                entity() { unique_id = id_counter; id_counter++; }

                virtual void update(float delta_ms) = 0;
            
                int get_id() { return unique_id;}
            private:
                int unique_id = 1;
                inline static std::atomic_int id_counter = 1;
        };
    }
};
      
