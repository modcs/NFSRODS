TEMPLATE = app
CONFIG += console c++
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    Config/lex.yy.c \
    Config/y.tab.c \
    Extras/cluster.c \
    attr.c \
    daemon.c \
    error.c \
    fd_cache.c \
    fh.c \
    fh_cache.c \
    locate.c \
    md5.c \
    mount.c \
    nfs.c \
    password.c \
    readdir.c \
    user.c \
    winsupport.c \
    xdr.c \
    irodsapi/rodsconnection.cpp \
    irodsapi/rodsobjentry.cpp \
    irodsapi/rodscapi.c \
    utils/rodscapi_utils.c \
    utils/utils.c \
    utils/json.c

HEADERS += \
    Config/exports.h \
    Config/exports.l \
    Config/exports.y \
    Config/y.tab.h \
    Extras/cluster.h \
    attr.h \
    backend.h \
    backend_unix.h \
    backend_win32.h \
    daemon.h \
    error.h \
    fd_cache.h \
    fh.h \
    fh_cache.h \
    locate.h \
    md5.h \
    mount.h \
    nfs.h \
    password.h \
    readdir.h \
    user.h \
    winsupport.h \
    xdr.h \
    irodsapi/rodscapi.h \
    irodsapi/rodsconnection.h \
    irodsapi/rodsobjentry.h \
    utils/rodscapi_utils.h \
    utils/utils.h \
    utils/json.h

DISTFILES += \
    Config/Makefile.in \
    contrib/nfsotpclient/nfsotpclient.py \
    contrib/nfsotpclient/rpc.py \
    contrib/nfsotpclient/README \
    contrib/nfsotpclient/mountclient/__init__.py \
    contrib/nfsotpclient/mountclient/mountconstants.py \
    contrib/nfsotpclient/mountclient/mountpacker.py \
    contrib/nfsotpclient/mountclient/mounttypes.py \
    contrib/rpcproxy/rpcproxy \
    doc/kirch1.txt \
    doc/passwords.txt \
    doc/TODO \
    doc/README.win \
    Extras/tags.7 \
    Extras/Makefile.in \
    LICENSE \
    README

LIBS += -lfl

QMAKE_CXXFLAGS += -std=c++0x
QMAKE_CXXFLAGS += -Wno-write-strings -fPIC -Wno-deprecated -D_FILE_OFFSET_BITS=64 -DPARA_OPR=1 -D_REENTRANT
QMAKE_CXXFLAGS += -DTAR_STRUCT_FILE -DGNU_TAR -DTAR_EXEC_PATH="/bin/tar" -DZIP_EXEC_PATH="/usr/bin/zip" -DUNZIP_EXEC_PATH="/usr/bin/unzip"
QMAKE_CXXFLAGS += -DPAM_AUTH -DUSE_BOOST -DBUILD_HOST=\\\"`hostname`\\\" -DBUILD_TAG=\\\"$(BUILD_TAG)\\\"

INCLUDEPATH += /usr/include/openssl
INCLUDEPATH += /usr/include/irods
INCLUDEPATH += /usr/include/irods/boost

LIBS += -L/usr/lib/irods/externals -lirods_client_api -lirods_client -lboost_filesystem -lboost_regex -lboost_system -lboost_thread
LIBS += -lboost_chrono -lboost_date_time -lboost_filesystem -lboost_iostreams -lboost_program_options
LIBS += /usr/lib/irods/externals/libjansson.a
LIBS += -lcrypto -lssl
LIBS += -ldl -lm -lpthread -lcurl
LIBS += -lldap
