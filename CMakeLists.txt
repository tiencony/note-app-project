cmake_minimum_required(VERSION 3.16)
project(NotesApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------
# Thu vien loi: moi thu tru main.cpp. Ca chuong trinh va bo test
# cung dung chung, nen khong phai bien dich cac file nay hai lan.
# ---------------------------------------------------------------
add_library(notes_core
    srcs/Note.cpp
    srcs/NoteManager/NoteManager.cpp
    srcs/NoteManager/NoteManager_Links.cpp
    srcs/NoteManager/NoteManager_Search.cpp
    srcs/NoteManager/NoteManager_Tags.cpp
    srcs/Utils/FileUtils.cpp
    srcs/Utils/Parser.cpp
    srcs/Utils/StringUtils.cpp
    srcs/Utils/TimeUtils.cpp
)

# PUBLIC: ai link voi notes_core cung duoc them duong dan "headers/"
# (de viet #include "NoteManager/NoteManager.h", "Utils/...").
target_include_directories(notes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/headers)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(notes_core PUBLIC -Wall -Wextra)
endif()

# ---------------------------------------------------------------
# Chuong trinh chinh
# ---------------------------------------------------------------
add_executable(notes srcs/main.cpp)
target_link_libraries(notes PRIVATE notes_core)

# ---------------------------------------------------------------
# Bo test tu dong (chay bang ctest)
# ---------------------------------------------------------------
enable_testing()

add_executable(notes_tests
    tests/test_main.cpp
    tests/test_notes.cpp
    tests/test_parser.cpp
)
target_link_libraries(notes_tests PRIVATE notes_core)

# Test dung duong dan tuong doi ("vault_test"), nen phai chay tu thu muc goc cua project.
add_test(NAME notes_tests
         COMMAND notes_tests
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------
# Khong dua vao build (giu nhu Makefile cu):
#   srcs/testing.cpp        - file nhap
#   debug/debug_rename.cpp  - chuong trinh debug rieng; neu muon build, bo comment 2 dong duoi
# ---------------------------------------------------------------
# add_executable(debug_rename debug/debug_rename.cpp)
# target_link_libraries(debug_rename PRIVATE notes_core)