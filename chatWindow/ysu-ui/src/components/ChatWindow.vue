<template>
  <div class="app-layout" id="app-layout">

    <div class="sidebar">
      <div class="logo-section">
        <img src="" id="logo-img" width="160" height="160" class="logo-img" />
        <!-- alt="" -->
        <span class="logo-text" id="logo-text"></span>
      </div>
      <el-button class="new-chat-button" @click="dialogVisible = true">
        <i class="fa-solid fa-user"></i>
        &nbsp;切换角色
      </el-button>
      <!-- 切换角色对话框 -->
      <el-dialog v-model="dialogVisible" title="请选择您想要对话的角色" :before-close="handleClose" max-width="640px">
        <div class="selectUsers">
          <div v-for="(user, index) in users" :key="index" :id="user.name" class="select-user"
            @click="changeUser(user.avatar, user.name, user.background)">
            <img :src="user.avatar" class="select-img" />
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
      <!-- 顶部导航栏 -->
      <div class="topbar">
        <div class="title">欢迎来到 AI 虚拟角色聊天平台</div>
        <div class="current-role">角色：{{ currentRole || '未选择角色' }}</div>
        <div class="actions">
          <el-button type="text" @click="handleLogout">退出</el-button>
          <el-button type="text" @click="handleSettings">设置</el-button>
        </div>
      </div>
      <div class="chat-container" id="chat-container">
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
          <el-button @click="startVoiceInput" circle class="mic-button">
            <i class="fa-solid fa-microphone"></i>
          </el-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { onMounted, ref, watch } from 'vue'
import axios from 'axios'
import { v4 as uuidv4 } from 'uuid'
import { ElMessage, ElMessageBox } from 'element-plus'


const messaggListRef = ref()
const isSending = ref(false)
const uuid = ref()
const inputMessage = ref('')
const messages = ref([])
const dialogVisible = ref(true);
const users = ref()
const currentRole = ref('') // 这里可以根据用户选择更新
const props = defineProps(['setRoute']);
let recognition;//语音识别对象

const handleLogout = () => {
  ElMessageBox.confirm(
    '是否退出登录？',
    '提示',
    {
      confirmButtonText: '是',
      cancelButtonText: '否',
      type: 'warning',
    }
  )
    .then(() => {
      props.setRoute('login')
      console.log("123")
    })
    .catch(() => {

    })
}

const handleSettings = () => {
  console.log("打开设置")
  // TODO: 打开设置对话框
}

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
  ).then((res) => {
    users.value = res.data;
    console.log(users.value);

  });
  hello()
})

const startVoiceInput = () => {
  // 兼容性处理
  const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
  if (!SpeechRecognition) {
    ElMessage.error("当前浏览器不支持语音输入，请使用 Chrome 或 Edge");
    return;
  }

  if (!recognition) {
    recognition = new SpeechRecognition();
    recognition.lang = "zh-CN"; // 识别中文
    recognition.interimResults = false; // 是否返回临时结果
    recognition.maxAlternatives = 1; // 候选结果数量

    recognition.onresult = (event) => {
      const transcript = event.results[0][0].transcript;
      inputMessage.value = transcript; // 把识别的内容放入输入框
      console.log("识别结果：", transcript);
    };

    recognition.onerror = (event) => {
      console.error("语音识别错误：", event.error);
      ElMessage.error("语音识别出错，请重试");
    };

    recognition.onend = () => {
      console.log("语音识别结束");
    };
  }

  recognition.start();
  ElMessage.info("正在听，请开始说话…");
};

const changeUser = (img, name, background) => {
  const mainImg = document.getElementById("logo-img");
  const mainName = document.getElementById("logo-text");
  const backgroundimg = document.getElementById("chat-container");
  mainImg.src = img;
  mainName.innerHTML = name;
  currentRole.value = name;
  backgroundimg.style.backgroundImage = "url(" + background + ")";
  console.log(img);
  dialogVisible.value = false;
  messages.value = [];
  hello()
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
  background: radial-gradient(circle at top left, #0f172a, #1e293b, #0f172a);
  color: #e2e8f0;
  font-family: 'Orbitron', 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background-size: cover;
  overflow: hidden;
}


/* 顶部导航栏 */
.topbar {
  height: 60px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 24px;
  background: linear-gradient(90deg, rgba(30, 41, 59, 0.7), rgb(11, 11, 81));
  box-shadow: 0 4px 20px rgba(0, 255, 255, 0.3);
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.topbar .title {
  font-size: 20px;
  font-weight: 700;
  color: #fff;
  letter-spacing: 2px;
  text-shadow: 0 0 8px #38bdf8;
}

.topbar .current-role {
  font-size: 16px;
  font-weight: 600;
  color: #ffffff;
  text-shadow: 0 0 6px rgba(255, 255, 255, 0.405);
}

.topbar .actions .el-button {
  background: linear-gradient(135deg, #0ea5e9, #6366f1);
  color: #e0e0e0;
  border-radius: 8px;
  padding: 6px 12px;
}

.topbar .actions .el-button:hover {
  background: rgba(99, 102, 241, 0.3);
  color: #ffffff;
  box-shadow: 0 0 8px rgba(99, 102, 241, 0.8);
}

/* 左侧边栏 */
.sidebar {
  width: 220px;
  background: rgba(30, 41, 59, 0.7);
  backdrop-filter: blur(8px);
  border-right: 1px solid rgba(255, 255, 255, 0.1);
  padding: 20px 15px;
  display: flex;
  flex-direction: column;
  align-items: center;
  box-shadow: inset -4px 0 12px rgba(0, 255, 255, 0.15);
}

.logo-section {
  display: flex;
  flex-direction: column;
  align-items: center;
  margin-bottom: 30px;
}

.logo-img {
  border-radius: 50%;
  object-fit: cover;
  box-shadow: 0 0 12px rgba(0, 255, 255, 0.7);
  border: 3px solid rgba(56, 189, 248, 0.6);
}

.logo-text {
  font-size: 18px;
  font-weight: 600;
  margin-top: 10px;
  color: #38bdf8;
  text-shadow: 0 0 10px #06b6d4;
}


/* 按钮 */
.new-chat-button {
  width: 100%;
  margin-top: 12px;
  border-radius: 8px;
  font-size: 14px;
  font-weight: 600;
  /* 字体加粗 */
  background: linear-gradient(135deg, #0ea5e9, #6366f1);
  color: #fff;
  border: none;
  box-shadow: 0 0 8px rgba(56, 189, 248, 0.5);
  text-shadow: 0 0 6px rgba(0, 0, 0, 0.6);
  /* 给文字加深色阴影，提高可读性 */
  transition: all 0.3s;
}

.new-chat-button:hover {
  background: linear-gradient(135deg, #06b6d4, #3b82f6);
  box-shadow: 0 0 15px rgba(59, 130, 246, 0.8);
  color: #fff;
}

/* 聊天主界面 */
.main-content {
  flex: 1;
  display: flex;
  flex-direction: column;
  background: transparent;
  padding: 0;
}

.chat-container {
  display: flex;
  flex-direction: column;
  flex: 1;
  padding: 20px;
  background: rgba(15, 23, 42, 0.6);
  background-image: url('src/assets/苏格拉底学院.jpg');
  background-size:cover;
  backdrop-filter: blur(10px);
  border-radius: 12px;
  margin: 12px;
  box-shadow: 0 0 20px rgba(0, 255, 255, 0.2);
}



/* 消息列表 */
.message-list {
  flex: 1;
  overflow-y: auto;
  padding: 10px 0;
  display: flex;
  flex-direction: column;
}

.message {
  max-width: 75%;
  padding: 12px 16px;
  margin-bottom: 14px;
  border-radius: 14px;
  line-height: 1.5;
  font-size: 15px;
}

/* 消息气泡 */
.user-message {
  align-self: flex-end;
  background: linear-gradient(135deg, #0ea5e9, #2563eb);
  color: #fff;
  border-bottom-right-radius: 4px;
  box-shadow: 0 0 12px rgba(14, 165, 233, 0.7);
}

.bot-message {
  align-self: flex-start;
  background: rgba(56, 191, 248, 0.8);
  border: 1px solid rgba(56, 189, 248, 0.4);
  color: #e0f2fe;
  border-bottom-left-radius: 4px;
  box-shadow: 0 0 12px rgba(56, 189, 248, 0.3);
}


@keyframes fadeIn {
  from {
    opacity: 0;
    transform: translateY(10px);
  }

  to {
    opacity: 1;
    transform: translateY(0);
  }
}

/* 输入框 */
.input-container {
  display: flex;
  align-items: center;
  padding: 12px 16px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
  background: rgba(30, 41, 59, 0.8);
  backdrop-filter: blur(6px);
  border-radius: 12px;
}

.input-container .el-input__inner {
  border-radius: 8px;
  border: 1px solid rgba(56, 189, 248, 0.4);
  background: rgba(15, 23, 42, 0.6);
  color: #fff;
}

.input-container .el-input__inner::placeholder {
  color: #9ca3af;
}

.input-container .el-button {
  margin-left: 10px;
  border-radius: 8px;
  background: linear-gradient(135deg, #06b6d4, #3b82f6);
  color: #fff;
  box-shadow: 0 0 10px rgba(59, 130, 246, 0.7);
}

.input-container .el-button:hover {
  background: linear-gradient(135deg, #38bdf8, #6366f1);
}

.mic-button {
  margin-left: 8px;
  border-radius: 50%;
  background: linear-gradient(135deg, #22d3ee, #3b82f6);
  color: #fff;
  box-shadow: 0 0 10px rgba(59, 130, 246, 0.6);
  transition: all 0.3s;
}

.mic-button:hover {
  background: linear-gradient(135deg, #38bdf8, #6366f1);
  box-shadow: 0 0 15px rgba(56, 189, 248, 0.8);
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
  transition: all 0.3s;
  border-radius: 12px;
  padding: 10px;
  background: #f9fafb;
}

.select-user:hover {
  background: #e0f2fe;
  box-shadow: 0 0 8px rgba(37, 99, 235, 0.4);
}

.select-img {
  width: 100px;
  height: 100px;
  border-radius: 12px;
  object-fit: cover;
  margin-bottom: 8px;
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
}

/* 加载点动画 */
.loading-dots .dot {
  background-color: #38bdf8;
  box-shadow: 0 0 8px #38bdf8;
  animation: pulse 1.5s infinite;
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
</style>
