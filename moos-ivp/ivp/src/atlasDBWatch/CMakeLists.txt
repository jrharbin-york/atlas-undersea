#--------------------------------------------------------
# The CMakeLists.txt for:                    ATLASDBWatch
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  SET(SYSTEM_LIBS
    wsock32)
else (${WIN32})
  SET(SYSTEM_LIBS
    m
    pthread)
endif (${WIN32})

SET(SRC
  ATLASDBWatch_App.cpp
  ATLASDB_Info.cpp
  ATLASLinkApp.cpp
  ATLASLog.cpp
  main.cpp)

set(CMAKE_CXX_STANDARD 11)

set(APR_INCLUDE_DIR "/home/userfs/j/jrh512/source/apr-install/include/apr-1")
set(ActiveMQ_INCLUDE_DIR "/home/userfs/j/jrh512/source/activemq-cpp-install/include/activemq-cpp-3.9.5")
include_directories(${APR_INCLUDE_DIR} ${ActiveMQ_INCLUDE_DIR})


#find_library(ActiveMQ_LIBRARY
#  NAMES activemq-cpp)
set(ActiveMQ_LIBRARY "/home/userfs/j/jrh512/source/activemq-cpp-install/lib/libactivemq-cpp.so")


ADD_EXECUTABLE(ATLASDBWatch ${SRC})

TARGET_LINK_LIBRARIES(ATLASDBWatch
  ${MOOS_LIBRARIES}
  ${ActiveMQ_LIBRARY}
  mbutil
  ${SYSTEM_LIBS})

