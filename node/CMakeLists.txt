# Add an executable target called "node"
add_executable(node)

# Add the "node.cpp" file to the "node" target

target_sources(node
    PRIVATE
        main.cpp
        state.cpp
        write.cpp
)

target_include_directories(node
    PRIVATE
        ../common
)

# Link the "node" target against the "common" library
target_link_libraries(node commonlib)