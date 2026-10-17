
export module aai;
export import aai.gfx;
export import aai.keeper;
export import aai.window;
export import aai.editor;
export import aai.utils.timer;
export import aai.utils.threads;

export {
    namespace  aai { 
        class core {
            public:
                core() {}
                ~core() { thread.stop(); gfx.clean(); clean(); }

                void init();

                void run();
            private:
                void clean();
                
                void render_update_callback(float delta_ms);
                void logic_update_callback(float delta_ms);

                aai::window win;
                aai::editor editor;
                gfx::base gfx;
                keeper::base keeper;

                utils::timer time;
                utils::threads thread;
        };
    }
};
