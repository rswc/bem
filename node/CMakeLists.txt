# Add an executable target called "node"
add_executable(node)

# Add the "node.cpp" file to the "node" target

target_sources(node
    PRIVATE
        src/main.cpp
        src/config.cpp
        src/communication.cpp
        src/protocol.cpp
        src/graceful.cpp
        src/execute.cpp
        src/state.cpp
)

target_include_directories(node
    PRIVATE
        ../common/src
)

# Link the "node" target against the "common" library
target_link_libraries(node commonlib)