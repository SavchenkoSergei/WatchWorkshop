cmake_minimum_required(VERSION 3.21)
project(WatchWorkshop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W4)  
else()
    add_compile_options(-Wall -Wextra)
endif()

add_library(workshop_core STATIC
    src/core/src/Client.cpp
    src/core/src/Watch.cpp
    src/core/src/Master.cpp
    src/core/src/Part.cpp
    src/core/src/Account.cpp
    src/core/src/LogEntry.cpp
    src/core/src/Order.cpp
    src/core/src/Service.cpp
    src/core/src/QuartzRepairService.cpp
    src/core/src/MechanicalRepairService.cpp
    src/core/src/ChronographRepairService.cpp
    src/core/src/ServiceCatalog.cpp
    src/core/src/FileStorage.cpp
    src/core/src/Workshop.cpp)
target_include_directories(workshop_core PUBLIC src/core/include)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets)
if(QT_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)           
    add_executable(workshop_gui WIN32
        src/gui/src/main.cpp
        src/gui/src/MainWindow.cpp
        src/gui/src/LoginDialog.cpp
        src/gui/src/StatusDelegate.cpp
        src/gui/src/SortProxy.cpp
        src/gui/src/DetailsDialog.cpp
        src/gui/src/ClientDialog.cpp
        src/gui/src/WatchDialog.cpp
        src/gui/src/MasterDialog.cpp
        src/gui/src/OrderDialog.cpp
        src/gui/src/PartDialog.cpp
        src/gui/src/ClientTableModel.cpp
        src/gui/src/WatchTableModel.cpp
        src/gui/src/MasterTableModel.cpp
        src/gui/src/OrderTableModel.cpp
        src/gui/src/PartTableModel.cpp
        src/gui/src/LogTableModel.cpp
        src/gui/resources.qrc)
    target_include_directories(workshop_gui PRIVATE src/gui/include)
    target_link_libraries(workshop_gui PRIVATE workshop_core Qt${QT_VERSION_MAJOR}::Widgets)
else()
    message(STATUS "Qt не найден: собираются только ядро")
endif()
