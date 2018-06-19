# Include the ExternalProject module
include(ExternalProject)

# Ignore all warning from the third-party libraries
if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions("-w")
endif()

option(BUILD_THIRDPARTY "Build thirdparty libaries." OFF)
set(BUILD_THIRDPARTY OFF)

if (REFRESH_THIRDPARTY) 
	set(BUILD_THIRDPARTY ON)
else ()
	find_library(JSONUI_LIB jsonui PATHS /usr/local/lib NO_DEFAULT_PATH)
	if(NOT JSONUI_LIB)
        	set(BUILD_THIRDPARTY ON)
	endif()
endif()

if (BUILD_THIRDPARTY)

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

else ()
	message(STATUS "JSONIO already present at /usr/local/lib. CMake will Stop. \n   Use -DREFRESH_THIRDPARTY=ON to reinstall ThirdParty.")
return(0)

endif()


