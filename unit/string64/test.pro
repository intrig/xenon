TEMPLATE = app

include ( $$(IMSDIRW)/bin/xenonapp.pri )

DESTDIR = ../o

SOURCES = \
    ../main.cpp

INCLUDES = \
    ../main.h

