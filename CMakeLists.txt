cmake_minimum_required(VERSION 2.8)

project(net_calc)
SET(CMAKE_CXX_FLAGS " -std=c++14 -ggdb -Wall -Werror -Wextra -O2 ")

include_directories(".")
add_library(net_calc_lib STATIC
		calculator.cpp
		)

ADD_EXECUTABLE (net-calc
	main.cpp
)

TARGET_LINK_LIBRARIES (net-calc
        net_calc_lib
		boost_system
		pthread
)

ADD_EXECUTABLE (generate-expression
		generate_expression.cpp
		)

TARGET_LINK_LIBRARIES (generate-expression pthread)

enable_testing()
set(unittests calculator)
foreach(t ${unittests})
	set(test_name ${t}_gtest)
	add_executable(${test_name} gtests/${t}.cpp )
	target_link_libraries(${test_name} net_calc_lib gtest gtest_main pthread)
	add_test(${test_name} ${test_name})
endforeach()