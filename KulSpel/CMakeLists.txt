cmake_minimum_required(VERSION 3.19)
project(KulSpel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Core)


qt_standard_project_setup()

qt_add_executable(KulSpel
    WIN32 MACOSX_BUNDLE
    main.cpp



    gameview.h gameview.cpp
    player.h player.cpp
    bullet.h bullet.cpp
    enemy.h enemy.cpp
    healthpickup.h healthpickup.cpp
    soundmanager.h soundmanager.cpp
    powerup.h powerup.cpp
)


target_link_libraries(KulSpel
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Multimedia
)
target_link_libraries(KulSpel PRIVATE Qt6::Core)

include(GNUInstallDirs)

install(TARGETS KulSpel
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET KulSpel
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
