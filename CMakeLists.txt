cmake_minimum_required(VERSION 3.20)

project(paper)


execute_process(
    COMMAND pkg-config --variable=pkgdatadir wayland-protocols
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Found wayland-protocols at ${WAYLAND_PROTOCOLS_DIR}")

set(WL_CODEGEN_DIR "${CMAKE_BINARY_DIR}/wl")
function(protocol proj protoPath protoName external)
    file(MAKE_DIRECTORY ${WL_CODEGEN_DIR})
    if (external)
        execute_process(
            COMMAND wayland-scanner client-header ${protoPath} ${WL_CODEGEN_DIR}/${protoName}.h
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        execute_process(
            COMMAND wayland-scanner private-code ${protoPath} ${WL_CODEGEN_DIR}/${protoName}.c
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        target_sources(${proj} PRIVATE ${WL_CODEGEN_DIR}/${protoName}.h ${WL_CODEGEN_DIR}/${protoName}.c)
    else()
        execute_process(
            COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOLS_DIR}/${protoPath} ${WL_CODEGEN_DIR}/${protoName}.h
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        execute_process(
            COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOLS_DIR}/${protoPath} ${WL_CODEGEN_DIR}/${protoName}.c
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        target_sources(${proj} PRIVATE ${WL_CODEGEN_DIR}/${protoName}.h ${WL_CODEGEN_DIR}/${protoName}.c)
    endif()
endfunction()

add_executable(paper src/main.cc src/wayland.cc src/panic.cc)

set_property(TARGET paper PROPERTY CXX_STANDARD 23)

protocol(paper "protocols/wlr-layer-shell-unstable-v1.xml" "wlr-layer-shell-unstable-v1" true)
protocol(paper "stable/xdg-shell/xdg-shell.xml" "xdg-shell" false)

target_link_libraries(paper wayland-client)
target_include_directories(paper PRIVATE ${WL_CODEGEN_DIR})