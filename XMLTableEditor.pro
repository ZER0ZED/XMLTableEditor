QT += core widgets xml

CONFIG += c++17

TARGET = XMLTableEditor
TEMPLATE = app

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    xmlworker.cpp

# Header files
HEADERS += \
    mainwindow.h \
    xmlworker.h



# Additional clean files
QMAKE_CLEAN += $(TARGET)

# Compiler flags for professional development
QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
