# Add an executable target called "coordinator"
add_executable(coordinator)

# Add the "coordinator.cpp" file to the "coordinator" target
target_sources(coordinator
    PRIVATE
        accept.cpp
        main.cpp
        node.cpp
        readNode.cpp
        writeNode.cpp
)

# Link the "coordinator" target against the "common" library
target_link_libraries(coordinator commonlib)

target_include_directories(coordinator
    PRIVATE
        ../common
)