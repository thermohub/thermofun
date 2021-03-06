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

# Download and install the bsonio library
ExternalProject_Add(JSONIO
    PREFIX thirdparty
    GIT_REPOSITORY https://bitbucket.org/gems4/jsonio.git 
#    GIT_TAG impex-move
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
               -DCMAKE_INSTALL_PREFIX=${THIRDPARTY_DIR}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_INSTALL_INCLUDEDIR=include
               -DCMAKE_INSTALL_LIBDIR=lib
               -DCMAKE_INSTALL_BINDIR=bin
               -DBUILD_SHARED_LIBS=ON
)


# Create the install target for the third-party libraries
install(DIRECTORY ${THIRDPARTY_DIR}/lib 
    DESTINATION .)
install(DIRECTORY ${THIRDPARTY_DIR}/include 
    DESTINATION .)
# Create the install target for the third-party libraries
#install(DIRECTORY ${THIRDPARTY_DIR}/
#    DESTINATION .)
