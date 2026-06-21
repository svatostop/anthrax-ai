
export module aai;
export import aai.gfx;
export import aai.keeper;
export import aai.window;

export {
    namespace  aai {
        class core {
            public:
                core() {}
                ~core() { gfx.clean(); clean(); }

                void init();

                void run();
            private:
                void clean();

                aai::window win;
                gfx::base gfx;
                keeper::base keeper;
        };
    }
};
