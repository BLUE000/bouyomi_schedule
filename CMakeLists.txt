cmake_minimum_required(VERSION 3.16)

project(BouyomiSchedule VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qtの自動処理設定
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# パス設定
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# Qtパッケージの検索
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Test)

# ... (既存の add_executable 部はそのまま) ...

# ユニットテストの設定
enable_testing()
add_executable(TestBouyomiSchedule
    tests/test_main.cpp
    core/timer_manager.cpp
    api/bouyomi_client.cpp
)
target_link_libraries(TestBouyomiSchedule PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)
target_include_directories(TestBouyomiSchedule PRIVATE
    ${CMAKE_SOURCE_DIR}
)
add_test(NAME TimerTest COMMAND TestBouyomiSchedule)

add_executable(LiveTest
    tests/live_test.cpp
    api/bouyomi_client.cpp
)
target_link_libraries(LiveTest PRIVATE
    Qt6::Core
    Qt6::Network
)
target_include_directories(LiveTest PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# ソースファイルの収集
file(GLOB_RECURSE SOURCES 
    "core/*.cpp"
    "api/*.cpp"
    "ui/*.cpp"
    "main.cpp"
)

file(GLOB_RECURSE HEADERS 
    "core/*.h"
    "api/*.h"
    "ui/*.h"
)

# 実行ファイルの生成
add_executable(BouyomiSchedule
    ${SOURCES}
    ${HEADERS}
)

# ライブラリのリンク
target_link_libraries(BouyomiSchedule PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# インクルードディレクトリの設定
target_include_directories(BouyomiSchedule PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/api
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/ui
)

# Windows固有の設定 (GUIアプリとしてビルド)
if(WIN32)
    set_target_properties(BouyomiSchedule PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
