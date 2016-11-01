# Include the ExternalProject module
include(ExternalProject)

# Ignore all warning from the third-party libraries
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions("-w")
endif()

# Download and install the bsonio library
ExternalProject_Add(BSONIO
    PREFIX thirdparty
    GIT_REPOSITORY https://dmiron@bitbucket.org/gems4/bsonio.git 
#    GIT_TAG TCorrPT
    UPDATE_COMMAND ""
    CMAKE_ARGS -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
               -DCMAKE_INSTALL_PREFIX=${THIRDPARTY_DIR}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DBUILD_SHARED_LIBS=ON
)

# Download and install the Reaktoro library
#ExternalProject_Add(REAKTORO
#    PREFIX thirdparty
#    GIT_REPOSITORY https://allanleal@bitbucket.org/reaktoro/reaktoro.git
#    GIT_TAG ${REAKTORO_TAG}
#    UPDATE_COMMAND ""
#    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${THIRDPARTY_DIR}
#               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#               -DBUILD_SHARED_LIBS=ON
#               -DLINK_GEMS=OFF
#)

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
