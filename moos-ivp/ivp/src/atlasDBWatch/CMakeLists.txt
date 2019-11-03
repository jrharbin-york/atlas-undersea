#--------------------------------------------------------
# The CMakeLists.txt for:                         pXRelay
# Author(s):                                Mike Benjamin
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
  ATLASLinkApp.cpp
  main.cpp
  )

include_directories(${APR_INCLUDE_DIR} ${ActiveMQ_INCLUDE_DIR})

find_library(ActiveMQ_LIBRARY
  NAMES activemq-cpp)

ADD_EXECUTABLE(ATLASDBWatch ${SRC})

TARGET_LINK_LIBRARIES(ATLASDBWatch
  ${MOOS_LIBRARIES}
  ${ActiveMQ_LIBRARY}
  mbutil
  ${SYSTEM_LIBS})

