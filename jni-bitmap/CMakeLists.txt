
cmake_minimum_required(VERSION 3.4.1)

add_library(bmputil SHARED
            src/main/cpp/bmputil.c)

# add lib dependencies
target_link_libraries(bmputil
                      jnigraphics
                      log)

