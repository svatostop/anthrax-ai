cmake_minimum_required(VERSION 4.2.3)


project(anthraxAI-demo-models)

set(_target anthraxAI-demo-models)
set(_code_dir ${CMAKE_CURRENT_SOURCE_DIR}/code)
set(_platform_dir ${CMAKE_CURRENT_SOURCE_DIR}/platform/${aai_lower_platform})

file(GLOB_RECURSE _sources CONFIGURE_DEPENDS
	${_code_dir}/*.h**
	${_code_dir}/*.c**
)
file(GLOB_RECURSE _platform_sources CONFIGURE_DEPENDS
	${_platform_dir}/*.h**
	${_platform_dir}/*.c**
)

add_executable(${_target} ${_sources} ${_platform_sources})
target_link_libraries(${_target} PUBLIC engine)

unset(_target)
unset(_code_dir)
unset(_platform_dir)
unset(_sources)
unset(_platform_sources)
