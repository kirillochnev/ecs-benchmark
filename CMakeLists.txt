cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)


project("EcsBenchmark")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_PARALLEL -fopenmp")
#set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fopenmp")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")


add_subdirectory(third_party/entt)
add_subdirectory(third_party/mustache)
add_subdirectory(third_party/glm)
add_subdirectory(third_party/entityx)

add_executable(${PROJECT_NAME}
        src/main.cpp src/benchmark.hpp src/open_ecs.hpp src/mustache.hpp
)

target_link_libraries(${PROJECT_NAME} EnTT)
target_link_libraries(${PROJECT_NAME} mustache)
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} entityx)
target_include_directories(${PROJECT_NAME} PRIVATE ./third_party/OpenEcs/include)
#target_link_libraries(${PROJECT_NAME} tbb)