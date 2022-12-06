# Add a library target called "commonlib"
add_library(commonlib)

# Add the cpp files to the "commonlib" target
target_sources(commonlib
    PRIVATE
        byteBuffer.cpp
        task.cpp
        taskMessage.cpp
)