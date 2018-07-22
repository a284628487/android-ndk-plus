
cmake_minimum_required(VERSION 3.4.1)

add_library(gpuimage SHARED
            src/main/cpp/yuv-decoder.c)

# add lib dependencies
target_link_libraries(gpuimage
                      log)


