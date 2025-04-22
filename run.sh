# !/bin/bash

cd assets

# Ubuntu 24.04.02
# attempts to launch `../build/demos/demo-models/anthraxAI-demo-models`
# would return an `Core::Audio::Init(): Device was not created` error continually
# a temporary fix:
# forcing loading the system `openal` instead of the one built by anthrax
# `build/engine/libs/openal-soft/openal-soft/libopenal.so.1`
# `apt install libopenal1` if not installed

LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libopenal.so.1 ../build/demos/demo-models/anthraxAI-demo-models
