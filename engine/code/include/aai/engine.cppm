export module aai;
export import aai.gfx;

export {
    namespace  aai {
        class core {
            public:
                void init();

                void run();

            private:
                gfx::base renderer;
        };
    }
};
