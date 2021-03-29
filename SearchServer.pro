TEMPLATE = app
CONFIG += console c++17 thread
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Tests/find_top_docs_par.cpp \
        Tests/match_doc_par.cpp \
        Tests/proc_queries.cpp \
        Tests/removed_doc_par.cpp \
        document.cpp \
        main.cpp \
        process_queries.cpp \
        read_input_functions.cpp \
        remove_duplicates.cpp \
        request_queue.cpp \
        search_server.cpp \
        string_processing.cpp

HEADERS += \
    Lib/concurrent_map.h \
    Tests/log_duration.h \
    Tests/finde_top_docs_par.h \
    Tests/match_doc_par.h \
    Tests/proc_queries.h \
    Tests/removed_doc_par.h \
    document.h \
    paginator.h \
    process_queries.h \
    read_input_functions.h \
    remove_duplicates.h \
    request_queue.h \
    search_server.h \
    string_processing.h
