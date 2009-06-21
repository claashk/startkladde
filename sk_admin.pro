# 0wxxxxifind . -name A�kb�kb!!sh
CONFIG += qt debug
QT += qt3support
TEMPLATE = app
TARGET = 
DEPENDPATH += . version
INCLUDEPATH += . version /usr/include/mysql
LIBS += -L/usr/lib64/mysql -lmysqlclient -lz -lacpi
MAKEFILE = Makefile_sk_admin
OBJECTS_DIR = build/
MOC_DIR= build/

# Input
HEADERS += \
           src/condition_t.h \
           src/config/options.h \
           src/data_types.h \
           src/db/admin_functions.h \
           src/db/db_column.h \
           src/db/db_proxy.h \
           src/db/db_table.h \
           src/db/db_types.h \
           src/db/sk_db.h \
           src/gui/dialogs.h \
           src/gui/settings.h \
           src/io/io.h \
           src/logging/messages.h \
           src/model/sk_flug.h \
           src/model/sk_flugzeug.h \
           src/model/sk_person.h \
           src/model/sk_user.h \
           src/model/startart_t.h \
           src/model/stuff.h \
           src/plugins/plugin_data_format.h \
           src/plugins/sk_plugin.h \
           src/text.h \
           src/time/sk_date.h \
           src/time/sk_time_t.h \
           src/time/time_functions.h \
           src/version.h \
           src/version/version.h \
           src/web/argument.h \

SOURCES += \
           src/condition_t.cpp \
           src/config/options.cpp \
           src/data_types.cpp \
           src/db/admin_functions.cpp \
           src/db/db_column.cpp \
           src/db/db_proxy.cpp \
           src/db/db_table.cpp \
           src/db/db_types.cpp \
           src/db/sk_db.cpp \
           src/gui/dialogs.cpp \
           src/gui/settings.cpp \
           src/io/io.cpp \
           src/logging/messages.cpp \
           src/model/sk_flug.cpp \
           src/model/sk_flugzeug.cpp \
           src/model/sk_person.cpp \
           src/model/sk_user.cpp \
           src/model/startart_t.cpp \
           src/model/stuff.cpp \
           src/plugins/plugin_data_format.cpp \
           src/plugins/sk_plugin.cpp \
           src/sk_admin.cpp \
           src/text.cpp \
           src/time/sk_date.cpp \
           src/time/sk_time_t.cpp \
           src/time/time_functions.cpp \
           src/version.cpp \
           src/web/argument.cpp \

