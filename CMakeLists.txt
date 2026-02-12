cmake_minimum_required(VERSION 3.10)
project(harbour-navalbattle LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Qml Quick REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SAILFISHAPP REQUIRED sailfishapp)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/GameEngine.cpp
    src/SettingsManager.cpp
    src/GameEngine.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${SAILFISHAPP_INCLUDE_DIRS}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${SAILFISHAPP_CFLAGS_OTHER}
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${SAILFISHAPP_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    ${SAILFISHAPP_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY qml/
    DESTINATION share/${PROJECT_NAME}/qml
    FILES_MATCHING
        PATTERN "*.qml"
        PATTERN "*.png"
)

install(FILES icons/86x86/harbour-navalbattle.png
    DESTINATION share/icons/hicolor/86x86/apps
)
install(FILES icons/108x108/harbour-navalbattle.png
    DESTINATION share/icons/hicolor/108x108/apps
)

install(FILES rpm/harbour-navalbattle.desktop
    DESTINATION share/applications
)
install(FILES rpm/harbour-navalbattle.appdata.xml
    DESTINATION share/metainfo
)
