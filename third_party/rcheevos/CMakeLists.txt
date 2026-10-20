# SPDX-License-Identifier: GPL-3.0-only
# Local build wrapper. Upstream C sources are unmodified and MIT licensed.
cmake_minimum_required(VERSION 3.28)
project(sfz2al_rcheevos LANGUAGES C)
file(GLOB RC_SOURCES CONFIGURE_DEPENDS src/api/*.c src/runtime/*.c src/util/*.c)
add_library(sfz2al_rcheevos STATIC ${RC_SOURCES} src/client/rc_client.c)
target_include_directories(sfz2al_rcheevos PUBLIC include src/util)
if(MSVC)
  target_compile_definitions(sfz2al_rcheevos PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(sfz2al_rcheevos PRIVATE /w)
endif()
