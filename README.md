# 部署方式

假设项目根目录为~/

## 前端

`cd ~/chatWindow && npm run dev`

## 后端

### nginx

nginx提供静态资源和反向代理服务，需要优先启动

`nginx -c ~/server/config/nginx.conf`

### 服务器

- 若您的设备为GNU/Linux X86_64架构，可以尝试直接启动预编译的服务进程

`~/server/AIchat`

- 尝试重新编译

  ```shell
  $ cd ~/server/src
  $ mkdir build && cd build
  $ cmake ..
  $ make
  $ ./AIchat
  ```

  通过make，您可以在build文件夹下找到AIchat可执行文件，启动改进程即可



# 配置介绍

在~/server/config中，存在三个配置文件

1. nginx.conf  提供nginx配置，无需修改

2. log4cpp.conf  提供日志打印配置，基于log4cpp规则按需修改

3. config.conf  提供服务器各项配置，以空格分隔的键值对形式

   ```
   logConfigPath /usr/AIchat/server/config/log4cpp.conf  log4cpp配置文件路径
   SERVER_HOST 47.109.39.124  服务器主机地址
   PORT 8888  服务启动端口，需注意nginx反向代理端口
   
   MySQL_HOST 47.109.39.124  MySQL服务端地址
   MySQL_USER xxx  数据库用户名
   MySQL_PASSWORD xxxxxx  数据库密码
   MySQL_DATABASE AIchat  数据库库名
   
   Redis_HOST 47.109.39.124  Redis服务端地址
   Redis_PORT 6379  端口
   Redis_DATABASE 1  数据库库名
   
   Retry_MAX 3  http请求最大重试次数
   
   LLM_URL https://openai.qiniu.com/v1/chat/completions   LLM地址
   TTS_URL https://openai.qiniu.com/v1/voice/tts   TTS地址
   MODEL_USER qwen-turbo   LLM模型名
   API_KEY xxxxxx    模型API密钥
   ```

   







