
export module aai;
export import aai.gfx;
export import aai.window;

export {
    namespace  aai {
        class core {
            public:
                core() {}
                ~core() { gfx.wait_timeline(); clean(); }

                void init();

                void run();
            private:
                void clean();

                aai::window win;
                gfx::base gfx;
        };
    }
};
