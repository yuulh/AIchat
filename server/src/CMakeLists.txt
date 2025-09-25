cmake_minimum_required(VERSION 3.20)
project(AIchat)

# 当前目录以及子目录下的所有.cc文件
file(GLOB_RECURSE SRC_LIST *.cc)

# 指定生成可执行文件
add_executable(AIchat ${SRC_LIST})

#链接库  -lpthread -llog4cpp -lrocketmq -lwfrest -lworkflow -lz -ldl -lrt
target_link_libraries(AIchat
    log4cpp pthread rocketmq wfrest workflow z dl rt
)

# 编译选项
add_compile_options(-std=c++11 -Wall -O2)