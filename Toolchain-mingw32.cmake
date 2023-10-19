# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
#SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
#SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
#SET(CMAKE_RC_COMPILER x86_64-mingw32-windres)
#SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
#SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
#SET(CMAKE_RC_COMPILER i686-mingw32-windres)
SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER x86_64-mingw32-windres)

# here is the target environment located
#SET(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32/)
SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32/)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -fvisibility=default -ggdb -mthreads -lpsapi -lpthread -DMINGW -lws2_32")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64 -lpsapi -lws2_32 -static -mwindows")
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_CXX_STANDARD_LIBRARIES} -lpsapi -lws2_32 -ldbgeng -lole32")

set(LP_IS_MINGW true)

