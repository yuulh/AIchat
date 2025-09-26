<template>
  <div class="app-layout" id="app-layout">
    <div class="sidebar">
      <div class="logo-section">
        <img src="http://47.109.39.124/public/4c377441f4c2012135c76614fea448a0.jpg" id="logo-img" width="160" height="160" class="logo-img" />
        <!-- alt="" -->
        <span class="logo-text" id="logo-text">哈利波特</span>
      </div>
      <el-button class="new-chat-button" @click="dialogVisible = true">
        <i class="fa-solid fa-user"></i>
        &nbsp;切换角色
      </el-button>
      <!-- 切换角色对话框 -->
      <el-dialog v-model="dialogVisible" title="请选择您想要对话的角色" :before-close="handleClose" max-width="640px">
        <div class="selectUsers">
          <div v-for="(user, index) in users" :key="index" :id="user.name" class="select-user"
            @click="changeUser(user.img, user.name, user.background)">
            <img :src="user.avatar+'.jpg'" class="select-img" />
            <a style="font-size:20px">{{ user.name }}</a>
          </div>
        </div>

      </el-dialog>

      <div></div>

      <el-button class="new-chat-button" @click="newChat">
        <i class="fa-solid fa-plus"></i>
        &nbsp;新会话
      </el-button>
    </div>
    <div class="main-content">
      <div class="chat-container">
        <div class="message-list" ref="messaggListRef">
          <div v-for="(message, index) in messages" :key="index" :class="message.isUser ? 'message user-message' : 'message bot-message'
            ">
            <!-- 会话图标 -->
            <i :class="message.isUser
              ? 'fa-solid fa-user message-icon'
              : 'fa-solid fa-robot message-icon'
              "></i>
            <!-- 会话内容 -->
            <span>
              <span v-html="message.content"></span>
              <!-- loading -->
              <span class="loading-dots" v-if="message.isThinking || message.isTyping">
                <span class="dot"></span>
                <span class="dot"></span>
              </span>
            </span>
          </div>
        </div>
        <div class="input-container">
          <el-input v-model="inputMessage" placeholder="请输入消息" @keyup.enter="sendMessage"></el-input>
          <el-button @click="sendMessage" :disabled="isSending" type="primary">发送</el-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { onMounted, ref, watch } from 'vue'
import axios from 'axios'
import { v4 as uuidv4 } from 'uuid'

const messaggListRef = ref()
const isSending = ref(false)
const uuid = ref()
const inputMessage = ref('')
const messages = ref([])
const dialogVisible = ref(false);
const users = ref()

onMounted(() => {
  initUUID()
  // 移除 setInterval，改用手动滚动
  watch(messages, () => scrollToBottom(), { deep: true })
  axios.get('/api/assistant/list', {
    params: {
      tag: "all",
      page: "1",
      page_size: "10"
    }
  }
  ).then((res)=>{
    users.value=res.data;
    console.log(users.value);
    
  });
  hello()
})

const changeUser = (img, name, background) => {
  const mainImg = document.getElementById("logo-img");
  const mainName = document.getElementById("logo-text");
  const backgroundimg = document.getElementById("app-layout");
  mainImg.src = img;
  mainName.innerHTML = name;
  backgroundimg.style.backgroundImage = "url(" + background + ")";
  dialogVisible.value = false;
}


const scrollToBottom = () => {
  if (messaggListRef.value) {
    messaggListRef.value.scrollTop = messaggListRef.value.scrollHeight
  }
}

const hello = () => {
  sendRequest('你好')
}

const sendMessage = () => {
  if (inputMessage.value.trim()) {
    sendRequest(inputMessage.value.trim())
    inputMessage.value = ''
  }
}

const sendRequest = (message) => {
  isSending.value = true
  const userMsg = {
    isUser: true,
    content: message,
    isTyping: false,
    isThinking: false,
  }
  //第一条默认发送的用户消息”你好“不放入会话列表
  if (messages.value.length > 0) {
    messages.value.push(userMsg)
  }


  // 添加机器人加载消息
  const botMsg = {
    isUser: false,
    content: '', // 增量填充
    isTyping: true, // 显示加载动画
    isThinking: false,
  }
  messages.value.push(botMsg)
  const lastMsg = messages.value[messages.value.length - 1]
  scrollToBottom()

  axios
    .post(
      '/api/ysu/chat',
      { assistant_id: "1", conversation_id: uuid.value, messages: message },
      {
        responseType: 'stream', // 必须为合法值 "text"
        onDownloadProgress: (e) => {
          const fullText = e.event.target.responseText // 累积的完整文本
          let newText = fullText.substring(lastMsg.content.length)
          lastMsg.content += newText //增量更新
          console.log(lastMsg)
          scrollToBottom() // 实时滚动
        },
      }
    )
    .then(() => {
      // 流结束后隐藏加载动画
      messages.value.at(-1).isTyping = false
      isSending.value = false
    })
    .catch((error) => {
      console.error('流式错误:', error)
      messages.value.at(-1).content = '请求失败，请重试'
      messages.value.at(-1).isTyping = false
      isSending.value = false
    })
}

// 初始化 UUID
const initUUID = () => {
  let storedUUID = localStorage.getItem('user_uuid')
  if (!storedUUID) {
    storedUUID = uuidToNumber(uuidv4())
    localStorage.setItem('user_uuid', storedUUID)
  }
  uuid.value = storedUUID
}

const uuidToNumber = (uuid) => {
  let number = 0
  for (let i = 0; i < uuid.length && i < 6; i++) {
    const hexValue = uuid[i]
    number = number * 16 + (parseInt(hexValue, 16) || 0)
  }
  return number % 1000000
}

// 转换特殊字符
const convertStreamOutput = (output) => {
  return output
    .replace(/\n/g, '<br>')
    .replace(/\t/g, '&nbsp;&nbsp;&nbsp;&nbsp;')
    .replace(/&/g, '&amp;') // 新增转义，避免 HTML 注入
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
}

const newChat = () => {
  // 这里添加新会话的逻辑
  console.log('开始新会话')
  localStorage.removeItem('user_uuid')
  window.location.reload()
}

</script>
<style scoped>
.app-layout {
  display: flex;
  height: 100vh;
  background: #f9f9f9;
  color: #333;
  font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
}

.sidebar {
  width: 220px;
  background: #ffffff;
  border-right: 1px solid #eaeaea;
  padding: 20px 15px;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.logo-section {
  display: flex;
  flex-direction: column;
  align-items: center;
  margin-bottom: 30px;
}

.logo-img {
  border-radius: 12px;
  object-fit: cover;
  box-shadow: 0 2px 6px rgba(0,0,0,0.15);
}

.logo-text {
  font-size: 18px;
  font-weight: 600;
  margin-top: 10px;
  color: #444;
}

.new-chat-button {
  width: 100%;
  margin-top: 12px;
  border-radius: 8px;
  font-size: 14px;
  background: #f5f5f5;
  color: #333;
  border: none;
  transition: all 0.2s;
}
.new-chat-button:hover {
  background: #eaeaea;
}

/* 聊天主界面 */
.main-content {
  flex: 1;
  display: flex;
  flex-direction: column;
  background: #f9f9f9;
  padding: 0;
}

.chat-container {
  display: flex;
  flex-direction: column;
  flex: 1;
  padding: 20px;
}

.message-list {
  flex: 1;
  overflow-y: auto;
  padding: 10px 0;
  display: flex;
  flex-direction: column;
}

/* 消息气泡 */
.message {
  max-width: 75%;
  padding: 12px 16px;
  margin-bottom: 14px;
  border-radius: 12px;
  line-height: 1.5;
  box-shadow: 0 1px 3px rgba(0,0,0,0.08);
  font-size: 15px;
}

.user-message {
  align-self: flex-end;
  background: #d1f1d6;
  border-bottom-right-radius: 4px;
}

.bot-message {
  align-self: flex-start;
  background: #fff;
  border-bottom-left-radius: 4px;
}

.message-icon {
  display: none; /* 去掉图标，保持简洁 */
}

/* 输入框区域 */
.input-container {
  display: flex;
  align-items: center;
  padding: 12px 16px;
  border-top: 1px solid #eaeaea;
  background: #fff;
}

.input-container .el-input {
  flex: 1;
}

.input-container .el-input__inner {
  border-radius: 8px;
  border: 1px solid #ddd;
  padding: 10px 14px;
}

.input-container .el-button {
  margin-left: 10px;
  border-radius: 8px;
  padding: 10px 18px;
}

/* 角色选择对话框 */
.selectUsers {
  display: flex;
  flex-wrap: wrap;
  gap: 20px;
  justify-content: center;
}

.select-user {
  display: flex;
  flex-direction: column;
  align-items: center;
  cursor: pointer;
  transition: all 0.2s;
  border-radius: 12px;
  padding: 10px;
}
.select-user:hover {
  background: #f5f5f5;
}

.select-img {
  width: 100px;
  height: 100px;
  border-radius: 12px;
  object-fit: cover;
  margin-bottom: 8px;
}


.loading-dots {
  padding-left: 5px;
}

.dot {
  display: inline-block;
  margin-left: 5px;
  width: 8px;
  height: 8px;
  background-color: #000000;
  border-radius: 50%;
  animation: pulse 1.2s infinite ease-in-out both;
}

.dot:nth-child(2) {
  animation-delay: -0.6s;
}

@keyframes pulse {

  0%,
  100% {
    transform: scale(0.6);
    opacity: 0.4;
  }

  50% {
    transform: scale(1);
    opacity: 1;
  }
}

.input-container {
  display: flex;
}

.input-container .el-input {
  flex: 1;
  margin-right: 10px;
}

/* 媒体查询，当设备宽度小于等于 768px 时应用以下样式 */

</style>
