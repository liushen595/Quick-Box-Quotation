cmake_minimum_required(VERSION 3.10)
project(keyboard-replace)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加头文件目录
include_directories(${CMAKE_SOURCE_DIR})

# 定义Unicode版本的Windows API
add_definitions(-DUNICODE -D_UNICODE)

# 添加源文件
set(SOURCE_FILES
    BoxQuotation.cpp
    src/SystemTray.cpp
)

# 添加Windows特定设置
if(WIN32)
    # 添加Windows库
    set(WINDOWS_LIBRARIES
        user32      # 用户界面相关功能
        shell32     # 系统托盘相关功能
        advapi32    # 注册表相关功能
    )
endif()

# 创建可执行文件
add_executable(BoxQuotation ${SOURCE_FILES})

# 链接Windows库
if(WIN32)
    target_link_libraries(BoxQuotation ${WINDOWS_LIBRARIES})
endif()

# 如果需要隐藏控制台窗口，可以取消下面这行的注释
# set_target_properties(main PROPERTIES WIN32_EXECUTABLE TRUE)