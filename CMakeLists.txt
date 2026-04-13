cmake_minimum_required(VERSION 4.2)
project(courseworkpfe C)

set(CMAKE_C_STANDARD 99)

add_executable(courseworkpfe main.c
        waveform.h
        waveform.c
        io.h
        io.c)
