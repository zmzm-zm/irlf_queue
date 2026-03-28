cmake_minimum_required(VERSION 3.16)
project(irlf_queue
	VERSION 0.0.1
	LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_NAME irlf_queue)
add_subdirectory(src)
add_subdirectory(examples)