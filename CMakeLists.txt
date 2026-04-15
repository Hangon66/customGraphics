cmake_minimum_required(VERSION 3.19)
project(customGraphics LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(customGraphics
    WIN32 MACOSX_BUNDLE
    main.cpp
    graphicstestwidget.cpp
    graphicstestwidget.h
    graphicstestwidget.ui
    IInteractionHandler.h
    CustomGraphicsView.h
    CustomGraphicsView.cpp
    CustomGraphicsScene.h
    CustomGraphicsScene.cpp
    ZoomHandler.h
    ZoomHandler.cpp
    PanHandler.h
    PanHandler.cpp
    RubberBandHandler.h
    RubberBandHandler.cpp
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
