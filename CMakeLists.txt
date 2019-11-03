cmake_minimum_required(VERSION 3.5)
project(Train_Project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    asm.s
    bwio.c
    bwio.h
    clockserver.c
    clockserver.h
    initialization.c
    initialization.h
    kernel_hwi.c
    kernel_hwi.h
    kernel_swi.c
    kernel_swi.h
    main.c
    main.h
    nameserver.c
    nameserver.h
    orex.ld
    print.c
    print.h
    priority_queue.c
    priority_queue.h
    rps.c
    rps.h
    string.c
    string.h
    syscall.c
    syscall.h
    task.c
    task.h
    track.c
    track.h
    trainserver.c
    trainserver.h
    ts7200.h
    uartserver.c
    uartserver.h global.h traininfo.h )

add_executable(Train_Project ${SOURCE_FILES})