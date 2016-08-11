# Include the ExternalProject module
include(ExternalProject)

# Ignore all warning from the third-party libraries
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions("-w")
endif()

# Set the list of compiler flags for the external projects
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CXXFLAGS "-fPIC")
endif()


# Create the install target for the third-party libraries
install(DIRECTORY ${THIRDPARTY_DIR}/lib 
    DESTINATION .)
install(DIRECTORY ${THIRDPARTY_DIR}/include 
    DESTINATION .)
# Create the install target for the third-party libraries
#install(DIRECTORY ${THIRDPARTY_DIR}/
#    DESTINATION .)
