export module aai.gfx;

export import aai.gfx.vk;
export {
    namespace gfx {
        class base {
            public:
                void init();

            private:
                vk::base vk;
        };
    }
};
