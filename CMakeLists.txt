cmake_minimum_required(VERSION 3.19)
project(customGraphics LANGUAGES CXX)

# MSVC 使用 UTF-8 编码
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(customGraphics
    WIN32 MACOSX_BUNDLE
    main.cpp
    graphicstestwidget.cpp
    graphicstestwidget.h
    graphicstestwidget.ui
    handlers/IInteractionHandler.h
    handlers/SceneConfig.h
    handlers/SceneConfig.cpp
    handlers/BackgroundHandler.h
    handlers/BackgroundHandler.cpp
    handlers/RulerHandler.h
    handlers/RulerHandler.cpp
    handlers/DrawHandler.h
    handlers/DrawHandler.cpp
    handlers/DragDropHandler.h
    handlers/DragDropHandler.cpp
    commands/ShapeCommands.h
    commands/ShapeCommands.cpp
    view/CustomGraphicsView.h
    view/CustomGraphicsView.cpp
    view/CustomGraphicsScene.h
    view/CustomGraphicsScene.cpp
    handlers/ZoomHandler.h
    handlers/ZoomHandler.cpp
    handlers/PanHandler.h
    handlers/PanHandler.cpp
    handlers/RubberBandHandler.h
    handlers/RubberBandHandler.cpp
    handlers/CollisionHandler.h
    handlers/CollisionHandler.cpp
)

target_link_libraries(customGraphics
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS customGraphics
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET customGraphics
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
