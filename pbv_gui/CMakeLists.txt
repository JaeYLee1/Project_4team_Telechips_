cmake_minimum_required(VERSION 3.16)

project(pbv_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Network)

# ============================================================
# Common
# ============================================================
set(COMMON_SOURCES
    common/telemetry_client.cpp
    common/telemetry_client.h
)

# ============================================================
# Weight Monitor
# 기존 실행파일 이름: pbv_gui
# ============================================================
add_executable(pbv_weight_monitor_gui
    weight_monitor/weight_monitor_main.cpp
    weight_monitor/weight_monitor_window.cpp
    weight_monitor/weight_monitor_window.h
    ${COMMON_SOURCES}
)

target_include_directories(pbv_weight_monitor_gui PRIVATE
    common
    weight_monitor
)

target_link_libraries(pbv_weight_monitor_gui PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)

# ============================================================
# Temperature Monitor
# ============================================================
add_executable(pbv_temperature_gui
    temperature_monitor/temperature_main.cpp
    temperature_monitor/temperaturewindow.cpp
    temperature_monitor/temperaturewindow.h
    ${COMMON_SOURCES}
)

target_include_directories(pbv_temperature_gui PRIVATE
    common
    temperature_monitor
)

target_link_libraries(pbv_temperature_gui PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)

# ============================================================
# Motor Monitor
# ============================================================
add_executable(pbv_motor_monitor_gui
    motor_monitor/motor_monitor_main.cpp
    motor_monitor/motor_monitor_window.cpp
    motor_monitor/motor_monitor_window.h
    ${COMMON_SOURCES}
)

target_include_directories(pbv_motor_monitor_gui PRIVATE
    common
    motor_monitor
)

target_link_libraries(pbv_motor_monitor_gui PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)

# ============================================================
# PBV HMI
# 실제 사용자 인터페이스
# ============================================================
add_executable(pbv_hmi
    hmi/hmi_main.cpp
    hmi/hmi_window.cpp
    hmi/hmi_window.h
    hmi_pages/idle_page.cpp
    hmi_pages/idle_page.h
    hmi_pages/module_a_page.cpp
    hmi_pages/module_a_page.h
    ${COMMON_SOURCES}
)

target_include_directories(pbv_hmi PRIVATE
    common
    hmi
    hmi_pages
)

target_link_libraries(pbv_hmi PRIVATE
    Qt6::Widgets
    Qt6::Charts
    Qt6::Network
)