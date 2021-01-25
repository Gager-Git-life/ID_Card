cmake_minimum_required(VERSION 3.10)
project(idcard)

set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR /home/gager/opencv_install/opencv-4.3.0/build)
find_package(OpenCV 4 REQUIRED)
#message(${OpenCV_INCLUDE_DIRS})
#message(${OpenCV_LIBS})
message(${CMAKE_SOURCE_DIR})

include_directories(
        ${OpenCV_INCLUDE_DIRS}
        mnn/include
        src
)

link_directories(mnn/lib)

add_executable(idcard src/main.cpp src/Image.cpp src/UltraFace.cpp)
target_link_libraries(idcard MNN ${OpenCV_LIBS})
