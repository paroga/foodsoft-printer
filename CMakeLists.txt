cmake_minimum_required(VERSION 3.6)
project(foodsoft-printer C)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
)

find_package(Cups REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Vala REQUIRED)
include(UseVala)

pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)
pkg_check_modules(LIBSOUP REQUIRED IMPORTED_TARGET libsoup-2.4)
pkg_check_modules(JSONGLIB REQUIRED IMPORTED_TARGET json-glib-1.0)
pkg_check_modules(MM REQUIRED IMPORTED_TARGET mm-glib)

if (CUPS_FOUND)
    if (NOT TARGET Cups::Cups)
        add_library(Cups::Cups INTERFACE IMPORTED)
        set_target_properties(Cups::Cups PROPERTIES
            INTERFACE_LINK_LIBRARIES      "${CUPS_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${CUPS_INCLUDE_DIR}")
    endif ()
endif ()

find_program(GENERATOR_EXECUTABLE ffmpeg)
if (NOT GENERATOR_EXECUTABLE)
    message(FATAL_ERROR "ffmpeg was not found")
endif ()

file(GLOB BITMAPS bitmaps/*.png)
foreach (BITMAP_PNG ${BITMAPS})
    get_filename_component(NAME ${BITMAP_PNG} NAME_WE)
    set(BITMAP_INC ${CMAKE_BINARY_DIR}/bitmap_${NAME}.inc)
    set(BITMAP_RAW ${CMAKE_BINARY_DIR}/bitmap_${NAME}.raw)

    add_custom_command(OUTPUT ${BITMAP_RAW}
        COMMAND ${GENERATOR_EXECUTABLE} -vcodec png -i ${BITMAP_PNG} -vcodec rawvideo -f rawvideo -pix_fmt rgb565 ${BITMAP_RAW}
        MAIN_DEPENDENCY ${BITMAP_PNG})

    list(APPEND BITMAP_NAMES ${NAME})
    list(APPEND BITMAP_TARGETS ${BITMAP_RAW})
endforeach ()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bitmaps.c ${CMAKE_BINARY_DIR}/bitmaps.h
    COMMAND ${CMAKE_SOURCE_DIR}/generate_bitmaps.sh ${CMAKE_BINARY_DIR} ${BITMAP_NAMES}
    MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/generate_bitmaps.sh
    DEPENDS ${BITMAP_TARGETS}
)

vala_precompile(FOODSOFT_PRINTER_C
    main.vala
PACKAGES
    libmm-glib
    libsoup-2.4
    libusb-1.0
    json-glib-1.0
CUSTOM_VAPIS
    cups.vapi
    display.vapi
)

list(APPEND FOODSOFT_PRINTER_C
    characters.c
    display.c
    ${CMAKE_BINARY_DIR}/bitmaps.c
)

add_executable(foodsoft-printer ${FOODSOFT_PRINTER_C})
target_include_directories(foodsoft-printer PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(foodsoft-printer Cups::Cups PkgConfig::LIBUSB PkgConfig::LIBSOUP PkgConfig::JSONGLIB PkgConfig::MM)

install(TARGETS foodsoft-printer DESTINATION bin)
install(FILES splash.fb DESTINATION share/foodsoft-printer)
