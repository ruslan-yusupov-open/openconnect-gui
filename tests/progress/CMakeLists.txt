cmake_minimum_required(VERSION 3.16)
project(progress_callback_regression LANGUAGES CXX)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(PROGRESS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/../../src/vpninfo.cpp" CACHE FILEPATH "Исходник callback")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROGRESS_SOURCE}")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/extract_callback.py"
        "${PROGRESS_SOURCE}" "${CMAKE_CURRENT_BINARY_DIR}/progress_callback.inc"
    RESULT_VARIABLE extraction_result
)
if(NOT extraction_result EQUAL 0)
    message(FATAL_ERROR "Не удалось извлечь progress_vfn из production-исходника")
endif()

add_executable(progress_callback_test progress_callback_test.cpp)
target_compile_features(progress_callback_test PRIVATE cxx_std_17)
target_include_directories(progress_callback_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
option(PROGRESS_ASAN "Проверять доступ к памяти через AddressSanitizer" ON)
if(PROGRESS_ASAN)
    if(MSVC)
        target_compile_options(progress_callback_test PRIVATE /fsanitize=address /Zi /Od)
        target_link_options(progress_callback_test PRIVATE /INCREMENTAL:NO)
    else()
        target_compile_options(progress_callback_test PRIVATE -fsanitize=address -fno-omit-frame-pointer -g -O0)
        target_link_options(progress_callback_test PRIVATE -fsanitize=address)
    endif()
endif()

enable_testing()
foreach(scenario empty newline plain trim_one long trace formatted formatted_empty)
    add_test(NAME progress_${scenario} COMMAND progress_callback_test "${scenario}")
endforeach()
