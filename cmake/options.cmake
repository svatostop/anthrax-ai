#======================================= Information ========================================#

cmake_host_system_information(RESULT aai_platform QUERY OS_NAME)
string(TOUPPER ${aai_platform} aai_upper_platform)
string(TOLOWER ${aai_platform} aai_lower_platform)

#========================================== Options =========================================#

option(AAI_BUILD_DEMOS                 "Build the demos"                    ON)
option(AAI_COMPILE_SHADERS             "Compile the shaders"                OFF)

#======================================== Directories ========================================#

set(aai_assets_dir ${aai_root}/assets CACHE PATH "Path to the assets directory")
set(aai_engine_dir ${aai_root}/engine CACHE PATH "Path to the engine directory")
set(aai_demos_dir  ${aai_root}/demos  CACHE PATH "Path to the demo directory")

#====================================== Configurations ======================================#

list(APPEND CMAKE_MODULE_PATH
	${aai_assets_dir}/cmake
	${aai_engine_dir}/cmake
	${aai_demos_dir}/cmake
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)




add_compile_definitions(
	AAI_${aai_upper_platform}
	AAI_$<UPPER_CASE:$<CONFIG>>
)
