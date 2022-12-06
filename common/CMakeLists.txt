# Add a library target called "commonlib"
add_library(commonlib)

# Add the cpp files to the "commonlib" target
target_sources(commonlib
    PRIVATE
        src/byteBuffer.cpp
        src/task.cpp
        src/taskMessage.cpp
)