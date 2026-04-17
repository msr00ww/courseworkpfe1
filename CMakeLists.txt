cmake_minimum_required(VERSION 3.20)
project(courseworkpfe C)

set(CMAKE_C_STANDARD 99)

add_executable(courseworkpfe
        main.c
        Waveform.h
        waveform.c
        io.h
        io.c)

file(COPY power_quality_log.csv DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(courseworkpfe m)