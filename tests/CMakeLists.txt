cmake_minimum_required(VERSION 3.11.0)

project(Tests)   


    include_directories(
						${CMAKE_CURRENT_SOURCE_DIR}/src
						${CMAKE_SOURCE_DIR}/core/src
	)

	#making sure to add the common cpp flags, that are defined in the main cpp file
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS}")

    #adding the executable
    add_executable(${PROJECT_NAME} src/main.cpp)
	SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(${PROJECT_NAME} TheBinderCore)

	#setting working directory
	set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>)

