cmake_minimum_required(VERSION 4.2.3)


project(anthraxAI
	VERSION 0.1.0
	DESCRIPTION "Anthrax AI | The Vulkan Based Game Engine"
	LANGUAGES CXX
)
set(aai_engine_code_dir ${aai_engine_dir}/code)
set(aai_engine_libs_dir ${aai_engine_dir}/libs)

add_subdirectory(${aai_engine_libs_dir})

file(GLOB_RECURSE anthrax_sources CONFIGURE_DEPENDS ${aai_engine_code_dir}/src/*.cpp)
file(GLOB_RECURSE anthrax_headers CONFIGURE_DEPENDS ${aai_engine_code_dir}/include/*.cppm)

# add_library(${PROJECT_NAME} ${anthrax_sources} ${anthrax_headers})
# add_library(anthraxAI::engine ALIAS ${PROJECT_NAME})

# add_library(${PROJECT_NAME})
add_library(engine SHARED)

target_sources(engine 
    PRIVATE
        ${anthrax_sources} 
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
           ${anthrax_headers}
)
set_target_properties(engine PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    CXX_MODULE_STD 1 )
target_compile_options(engine PRIVATE -fmodules-ts)
target_compile_features(engine
  PRIVATE   cxx_std_23
  INTERFACE cxx_std_20)

# target_compile_features(engine PUBLIC cxx_std_20)
target_link_libraries(engine PUBLIC anthrax::libraries)
target_include_directories(engine PUBLIC ${aai_engine_code_dir}/include)
