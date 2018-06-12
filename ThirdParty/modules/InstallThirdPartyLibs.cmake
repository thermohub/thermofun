# Include the ExternalProject module
include(ExternalProject)

# Ignore all warning from the third-party libraries
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions("-w")
endif()

# Download and install the jsonio library
ExternalProject_Add(JSONIO
    PREFIX thirdparty
    GIT_REPOSITORY https://bitbucket.org/gems4/jsonio.git 
#    GIT_TAG impex-move
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
               -DCMAKE_INSTALL_PREFIX=${THIRDPARTY_DIR}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DBUILD_SHARED_LIBS=ON
)

# Download and install the bsonio library
#ExternalProject_Add(BSONUI
#    PREFIX thirdparty
#    GIT_REPOSITORY https://svetadmitrieva@bitbucket.org/gems4/bsonui.git
#    UPDATE_COMMAND ""
#    CMAKE_ARGS -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
#    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${THIRDPARTY_DIR}
#               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#               -DBUILD_SHARED_LIBS=ON
#)


# Create the install target for the third-party libraries
install(DIRECTORY ${THIRDPARTY_DIR}/
    DESTINATION .)
