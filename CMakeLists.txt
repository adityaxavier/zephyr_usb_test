# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(zephyr_usb_test)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE 
    src/main.c
    src/USB/usb_cdc_acm.c
    src/USB/usb_thread.c)
    
zephyr_include_directories(${APPLICATION_SOURCE_DIR}/src)
zephyr_library_include_directories($ENV{ZEPHYR_BASE}/samples/testing/zephyr_usb_test)