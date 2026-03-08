import aai.gfx.vk;

void vk::base::init(bool validate)
{
    inst.init(validate);
    dev.init(inst.get_instance(), validate, inst.get_layers());
}
