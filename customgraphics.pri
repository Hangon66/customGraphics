########## CustomGraphics Begin ##########

CUSTOMGRAPHICS_DIR = $$PWD

SOURCES += \
    $$CUSTOMGRAPHICS_DIR/commands/ShapeCommands.cpp \
    $$CUSTOMGRAPHICS_DIR/customgraphicswidget.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/BackgroundHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/BoardLoadHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/CollisionHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/DragDropHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/DrawHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/PanHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/RubberBandHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/RulerHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/SceneConfig.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/SimulationHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/ZoomHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/handlers/GuideLineHandler.cpp \
    $$CUSTOMGRAPHICS_DIR/view/CustomGraphicsScene.cpp \
    $$CUSTOMGRAPHICS_DIR/view/CustomGraphicsView.cpp \
    $$CUSTOMGRAPHICS_DIR/view/PropertyPanel.cpp \
    $$CUSTOMGRAPHICS_DIR/view/MinimapWidget.cpp

HEADERS += \
    $$CUSTOMGRAPHICS_DIR/commands/ShapeCommands.h \
    $$CUSTOMGRAPHICS_DIR/customgraphicswidget.h \
    $$CUSTOMGRAPHICS_DIR/handlers/BackgroundHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/BoardLoadHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/CollisionHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/DragDropHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/DrawHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/IInteractionHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/PanHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/RubberBandHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/RulerHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/SceneConfig.h \
    $$CUSTOMGRAPHICS_DIR/handlers/SimulationHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/ZoomHandler.h \
    $$CUSTOMGRAPHICS_DIR/handlers/GuideLineHandler.h \
    $$CUSTOMGRAPHICS_DIR/view/CustomGraphicsScene.h \
    $$CUSTOMGRAPHICS_DIR/view/CustomGraphicsView.h \
    $$CUSTOMGRAPHICS_DIR/view/PropertyPanel.h \
    $$CUSTOMGRAPHICS_DIR/view/ShapeMetadata.h \
    $$CUSTOMGRAPHICS_DIR/view/MinimapWidget.h

FORMS += $$CUSTOMGRAPHICS_DIR/customgraphicswidget.ui

INCLUDEPATH += \
    $$CUSTOMGRAPHICS_DIR \
    $$CUSTOMGRAPHICS_DIR/commands \
    $$CUSTOMGRAPHICS_DIR/handlers \
    $$CUSTOMGRAPHICS_DIR/items \
    $$CUSTOMGRAPHICS_DIR/view

########## CustomGraphics End   ##########
