
export module aai.gfx.vk;
export import aai.gfx.vk.instance;
export import aai.gfx.vk.device;

export {
   namespace vk {
       class base {
           public:
               void init(bool validate);

           private:
               instance inst;
               device dev;
       };
   }
};

