cmake_minimum_required(VERSION 2.8)

project( kcp_image_streaming )

set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
# message(STATUS "OpenCV library status:")
# message(STATUS "  config: ${OpenCV_DIR}")
# message(STATUS "  version: ${OpenCV_VERSION}")
# message(STATUS "  libraries: ${OpenCV_LIBS}")
# message(STATUS "  include path: ${OpenCV_INCLUDE_DIRS}")


add_executable( server kcp_server.cc )
target_link_libraries( server ${OpenCV_LIBS} )

add_executable( client kcp_client.cc )
target_link_libraries( client ${OpenCV_LIBS} )
