cmake_minimum_required(VERSION 3.20.0)


################################
set(EXECUTABLE_TARGET "${PROJECT_NAME}_receive")
add_executable(${EXECUTABLE_TARGET})

target_compile_options(${EXECUTABLE_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_sources(
    ${EXECUTABLE_TARGET}
    PRIVATE # All private source and header files listed here
        main_receive.c
)

target_link_libraries(
    ${EXECUTABLE_TARGET}
    PRIVATE # All external libraries needed for this project.
        ${PROJECT_NAME}
)

add_test(
    NAME
        ${EXECUTABLE_TARGET}
    COMMAND
        ${EXECUTABLE_TARGET}
)

################################
set(EXECUTABLE_TARGET "${PROJECT_NAME}_transmit")
add_executable(${EXECUTABLE_TARGET})

target_compile_options(${EXECUTABLE_TARGET} PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_sources(
    ${EXECUTABLE_TARGET}
    PRIVATE # All private source and header files listed here
        main_transmit.c
)

target_link_libraries(
    ${EXECUTABLE_TARGET}
    PRIVATE # All external libraries needed for this project.
        ${PROJECT_NAME}
)

add_test(
    NAME
        ${EXECUTABLE_TARGET}
    COMMAND
        ${EXECUTABLE_TARGET}
)
