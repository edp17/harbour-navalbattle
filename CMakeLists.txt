cmake_minimum_required(VERSION 3.10)
project(harbour-navalbattle LANGUAGES CXX)

set(NAVALBATTLE_VERSION "development" CACHE STRING
    "Version shown by the application")
option(NAVALBATTLE_BUILD_TESTS "Build deterministic engine tests" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core LinguistTools Multimedia Qml Quick REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SAILFISHAPP REQUIRED sailfishapp)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/GameEngine.cpp
    src/SettingsManager.cpp
    src/GameEngine.h
)

set(NAVALBATTLE_TRANSLATIONS
    translations/harbour-navalbattle-hu.ts
)
qt5_add_translation(NAVALBATTLE_QM_FILES ${NAVALBATTLE_TRANSLATIONS})
add_custom_target(${PROJECT_NAME}-translations ALL DEPENDS ${NAVALBATTLE_QM_FILES})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-translations)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${SAILFISHAPP_INCLUDE_DIRS}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    ${SAILFISHAPP_CFLAGS_OTHER}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    NAVALBATTLE_VERSION="${NAVALBATTLE_VERSION}"
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${SAILFISHAPP_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Multimedia
    Qt5::Qml
    Qt5::Quick
    ${SAILFISHAPP_LIBRARIES}
)

if(NAVALBATTLE_BUILD_TESTS)
    enable_testing()

    add_executable(test-gameengine
        tests/test_gameengine.cpp
        tests/GameEngineTestAccess.h
        src/GameEngine.cpp
        src/GameEngine.h
    )
    target_include_directories(test-gameengine PRIVATE src tests)
    target_link_libraries(test-gameengine Qt5::Core)
    add_test(NAME gameengine-deterministic COMMAND test-gameengine)

    add_executable(test-settingsmanager
        tests/test_settingsmanager.cpp
        src/SettingsManager.cpp
        src/SettingsManager.h
    )
    target_include_directories(test-settingsmanager PRIVATE src)
    target_link_libraries(test-settingsmanager Qt5::Core)
    add_test(NAME settingsmanager-persistence COMMAND test-settingsmanager)

    add_executable(simulate-ai
        tests/simulate_ai.cpp
        tests/GameEngineTestAccess.h
        src/GameEngine.cpp
        src/GameEngine.h
    )
    target_include_directories(simulate-ai PRIVATE src tests)
    target_link_libraries(simulate-ai Qt5::Core)
    add_test(NAME gameengine-difficulty-simulation COMMAND simulate-ai)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY qml/
    DESTINATION share/${PROJECT_NAME}/qml
    FILES_MATCHING
        PATTERN "*.qml"
        PATTERN "*.png"
        PATTERN "*.wav"
)

install(FILES ${NAVALBATTLE_QM_FILES}
    DESTINATION share/${PROJECT_NAME}/translations
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
