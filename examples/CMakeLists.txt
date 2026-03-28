add_executable(test)
add_executable(compare)
target_sources(test PRIVATE
	test.cpp
)
target_link_libraries(test
	${LIBRARY_NAME}
)
target_sources(compare PRIVATE
	compare.cpp
)
target_link_libraries(compare
	${LIBRARY_NAME}
)