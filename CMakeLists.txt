cmake_minimum_required(VERSION 3.5.0)

project(xmlrpc_client)

set(SRC
    main.c)

add_executable(main ${SRC})

target_link_libraries(main xmlrpc xmlrpc_client xmlrpc_util)
