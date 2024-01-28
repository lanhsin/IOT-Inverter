cmake_minimum_required(VERSION 3.5)


# Project declaration
set(INVIOT_MAJOR_VERSION 0)
set(INVIOT_MINOR_VERSION 0)
set(INVIOT_MICRO_VERSION 1)
set(INVIOT_VERSION ${INVIOT_MAJOR_VERSION}.${INVIOT_MINOR_VERSION}.${INVIOT_MICRO_VERSION})
project(invIOT VERSION ${INVIOT_VERSION})

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

add_definitions (-DINVIOT_VERSION=\"${INVIOT_VERSION}\")

set(common_srcs
    common/common.cpp
    common/network.cpp
    common/db_base.cpp
    )

set(inv_srcs
    inv_conf.cpp
    inv_data.cpp
    simdev.cpp
    inverter.cpp
    inv_db.cpp
    main.cpp
    )

set(libmodbus_srcs
    lib/modbus/modbus.c
    lib/modbus/modbus-data.c
    lib/modbus/modbus-rtu.c
    lib/modbus/modbus-tcp.c
    lib/modbus/modbus-goodwe.c
    lib/modbus/modbus-kaco.c
)
add_library(modbus STATIC ${libmodbus_srcs})

add_library(sqlite STATIC lib/sqlite3.c)

add_executable(invIOT ${common_srcs} ${inv_srcs})
target_compile_options(invIOT PUBLIC -Wall)
target_include_directories(invIOT PUBLIC include)
target_link_libraries(invIOT modbus pthread sqlite)

install(TARGETS invIOT DESTINATION /usr/local/bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR ${INVIOT_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${INVIOT_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${INVIOT_MICRO_VERSION})
set(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_CONTACT "lanhsin.lin@gmail.com")
include(CPack)