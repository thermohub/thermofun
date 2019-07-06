# The path where cmake config files are installed
set(THERMOFUN_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/ThermoFun)

install(EXPORT ThermoFunTargets
    FILE ThermoFunTargets.cmake
    NAMESPACE ThermoFun::
    DESTINATION ${THERMOFUN_INSTALL_CONFIGDIR}
    COMPONENT cmake)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ThermoFunConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ThermoFunConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ThermoFunConfig.cmake
    INSTALL_DESTINATION ${THERMOFUN_INSTALL_CONFIGDIR}
    PATH_VARS THERMOFUN_INSTALL_CONFIGDIR)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ThermoFunConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ThermoFunConfigVersion.cmake
    DESTINATION ${THERMOFUN_INSTALL_CONFIGDIR})
