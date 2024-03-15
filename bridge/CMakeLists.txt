# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

# This example needs a DNS server: let's use the simple DNS server implementation from captive portal example
#set(EXTRA_COMPONENT_DIRS /home/kaspernyhus/esp/idf-versions/esp-idf/examples/protocols/http_server/captive_portal/components/dns_server)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp_tinyusb_test)
