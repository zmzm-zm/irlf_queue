add_executable(${TEST_NAME})
target_sources(${TEST_NAME} PRIVATE
	test.cpp)
target_link_libraries(${TEST_NAME}
	${LIBRARY_NAME})