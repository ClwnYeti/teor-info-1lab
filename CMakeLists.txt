cmake_minimum_required(VERSION 3.29)
project(teor_info-lab1)

set(CMAKE_CXX_STANDARD 20)


include_directories(evgeny-belyaev-lib)

add_library(
        evgeny-belyaev-lib
        "evgeny-belyaev-lib/ac_dec.cpp"
        "evgeny-belyaev-lib/ac_enc.cpp"
        "evgeny-belyaev-lib/context.cpp"
)

add_executable(teor_info-lab1 main.cpp)
target_link_libraries(teor_info-lab1 evgeny-belyaev-lib)
