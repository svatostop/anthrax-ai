cmake_minimum_required(VERSION 3.30)

project(anthraxAI-demos
	VERSION 0.1.0
	LANGUAGES CXX
)

include(AAIScanDemos)
aai_scan_demos(RESULT aai_demos DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

foreach(demo IN LISTS aai_demos)
	add_subdirectory(${demo})
endforeach()
