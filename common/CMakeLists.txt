# Add a library target called "commonlib"
add_library(commonlib)

# Add the cpp files to the "commonlib" target
target_sources(commonlib
    PRIVATE
        src/baseMessage.cpp
        src/invalidMessage.cpp
        src/messageFactory.cpp
        src/task.cpp
        src/taskMessage.cpp
        src/helloMessage.cpp 
        src/resultMessage.cpp 
        src/result.cpp
        src/gamelist.cpp
        src/taskNotifyMessage.cpp
)