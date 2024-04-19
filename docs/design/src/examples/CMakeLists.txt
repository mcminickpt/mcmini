add_executable(hello-world hello-world.c)
add_executable(vms volatile_mem_stream.cpp ../../src/mcmini/real_world/shm.cpp)
target_include_directories(vms PUBLIC ../../include)
target_link_libraries(hello-world PUBLIC -pthread)
