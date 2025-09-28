<template>
  <div class="auth-container">
    <div class="title-section">
      <div class="title-logo">
        <!-- 如果有 logo.png 就用图片，没有就换成 🤖 -->
        <img src="@/assets/logo.png" alt="logo" class="logo-img" />
        <!-- <span class="logo-emoji">🤖</span> -->
        <h1 class="system-title">欢迎来到 AI 虚拟角色聊天平台</h1>
      </div>
      <p class="system-subtitle">开启你的智能聊天之旅</p>
    </div>

    <div class="card">
      <h2>{{ mode === 'login' ? '登录' : '注册' }}</h2>

      <form @submit.prevent="handleSubmit">
        <label class="field">
          <span>用户名</span>
          <input
            v-model.trim="username"
            placeholder="字母、数字和下划线"
            autocomplete="username"
          />
        </label>

        <label class="field">
          <span>密码</span>
          <input
            v-model="password"
            type="password"
            autocomplete="current-password"
            placeholder="至少 6 位"
          />
        </label>

        <label v-if="mode === 'register'" class="field">
          <span>确认密码</span>
          <input
            v-model="confirmPassword"
            type="password"
            autocomplete="new-password"
            placeholder="再次输入密码"
          />
        </label>

        <div class="actions">
          <button type="submit" class="btn-primary">{{ mode === 'login' ? '登录' : '注册' }}</button>
          <button type="button" class="btn-link" @click="toggleMode">
            {{ mode === 'login' ? '去注册' : '去登录' }}
          </button>
        </div>

        <p v-if="message" :class="['message', message.type]">{{ message.text }}</p>
      </form>
    </div>
  </div>
</template>



<script setup>
import { ref } from 'vue'
import md5 from 'crypto-js/md5'
const props=defineProps(['setRoute']);

// 要求说明：
// 1. 用户名只能包含字母、数字和“_”。
// 2. 密码需在客户端用 MD5 加密后再发送（注意：MD5 不安全，仅按需求实现；生产环境请在服务器端使用 bcrypt/argon2 并加盐）。

const mode = ref('login') // 'login' | 'register'
const username = ref('')
const password = ref('')
const confirmPassword = ref('')
const message = ref(null)
const loading = ref(false)

const usernameRegex = /^[A-Za-z0-9_]+$/

function validateUsername(name) {
  return name && usernameRegex.test(name)
}

function validatePassword(pwd) {
  return pwd && pwd.length >= 6
}

function toggleMode() {
  mode.value = mode.value === 'login' ? 'register' : 'login'
  message.value = null
}

// 密码预处理：将每个字符的 charCode + 1，再拼接回来
function shiftPassword(pwd) {
  return Array.from(pwd)
    .map(ch => String.fromCharCode(ch.charCodeAt(0) + 1))
    .join('')
}

async function handleSubmit() {
  message.value = null

  if (!validateUsername(username.value)) {
    message.value = { type: 'error', text: '用户名只能包含字母、数字和下划线（_）。' }
    return
  }

  if (!validatePassword(password.value)) {
    message.value = { type: 'error', text: '密码长度至少 6 个字符。' }
    return
  }

  if (mode.value === 'register' && password.value !== confirmPassword.value) {
    message.value = { type: 'error', text: '两次输入的密码不一致。' }
    return
  }

  loading.value = true
  try {
    // 先对密码做字符 +1 处理
    //const shiftedPwd = shiftPassword(password.value)
    // 再做 MD5 加密
    const hashed = md5(password.value).toString()

    const payload = {
      user: username.value,
      pwd: hashed,
    }

    const url = mode.value === 'login' ? 'api/user/login' : 'api/user/register'

    const res = await fetch(url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload),
    })

    if (!res.ok) {
      const text = await res.text()
      throw new Error(text || `请求失败：${res.status}`)
    }

    const data = await res.text();
    message.value = { type: 'success', text: data.message || (mode.value === 'login' ? '登录成功' : '注册成功') }
    if(mode.value==='login'){
      localStorage.setItem('user',payload.user)
      console.log(localStorage.getItem('user'));
      
      props.setRoute('chat');
    }

    // TODO: 登录成功后可执行跳转或保存 token
  } catch (err) {
    message.value = { type: 'error', text: err.message || '网络或服务器错误' }
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.auth-container {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  background-image: url('https://ugc-img.ifengimg.com/img/2021/12/21/8C9GjBjOKOT/bb280dc4-ede7-40be-b06b-6575324738de_w2194_h919.jpeg');
  background-size:cover;
  opacity: 1;
  padding: 24px;
  margin: 0px;
  font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
}

.title-section {
  text-align: center;
  margin-bottom: 24px;
  color: #fff;
  animation: fadeInDown 0.8s ease-in-out;
}

.title-logo {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 12px;
}

.logo-img {
  width: 48px;
  height: 48px;
  border-radius: 50%;
  object-fit: cover;
  background: #fff;
  padding: 4px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.2);
}

.logo-emoji {
  font-size: 40px;
}

.system-title {
  font-size: 26px;
  font-weight: 700;
  margin: 0;
}

.system-subtitle {
  font-size: 16px;
  font-weight: 400;
  opacity: 0.9;
  margin-top: 8px;
}
@keyframes fadeInDown {
  from { opacity: 0; transform: translateY(-20px); }
  to { opacity: 1; transform: translateY(0); }
}

@keyframes zoomIn {
  from { opacity: 0; transform: scale(0.95); }
  to { opacity: 1; transform: scale(1); }
}

.card {
  width: 400px;
  padding: 36px 32px;
  border-radius: 20px;
  background: rgba(255, 255, 255, 0.15);
  backdrop-filter: blur(18px) saturate(160%);
  -webkit-backdrop-filter: blur(18px) saturate(160%);
  border: 1px solid rgba(255, 255, 255, 0.25);
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.25),
              0 0 20px rgba(99, 102, 241, 0.3);
  animation: zoomIn 0.7s ease-in-out;
  color: #fff;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(20px); }
  to { opacity: 1; transform: translateY(0); }
}


h2 {
  margin: 0 0 24px 0;
  font-size: 24px;
  font-weight: 700;
  text-align: center;
  background: linear-gradient(90deg, #6366f1, #3906d4);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
}

.field span {
  font-size: 14px;
  margin-bottom: 6px;
  color: #0857f4;
  font-weight: 500;
}
@keyframes fadeIn {
  from { opacity: 0; transform: translateY(20px); }
  to { opacity: 1; transform: translateY(0); }
}

.field {
  display: flex;
  flex-direction: column;
  margin-bottom: 18px;
}

input {
  padding: 12px 14px;
  border: 1px solid rgba(255, 255, 255, 0.3);
  border-radius: 12px;
  outline: none;
  font-size: 15px;
  background: rgba(255, 255, 255, 0.1);
  color: #fff;
  transition: all 0.3s ease-in-out;
  box-shadow: inset 0 2px 6px rgba(0,0,0,0.2);
}

input:focus {
  border-color: #6366f1;
  box-shadow: 0 0 12px rgba(99, 102, 241, 0.6);
  background: rgba(255, 255, 255, 0.2);
}

.actions {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  margin-top: 10px;
}


.btn-primary {
  flex: 1;
  padding: 12px 14px;
  border-radius: 12px;
  border: none;
  cursor: pointer;
  background: linear-gradient(90deg, #6366f1, #06b6d4);
  color: white;
  font-size: 15px;
  font-weight: 600;
  box-shadow: 0 4px 12px rgba(99, 102, 241, 0.4);
  transition: all 0.3s ease;
}

.btn-primary:hover {
  transform: translateY(-2px) scale(1.03);
  box-shadow: 0 6px 18px rgba(99, 102, 241, 0.7);
}

.btn-link {
  background: none;
  border: none;
  padding: 8px 12px;
  cursor: pointer;
  font-size: 14px;
  font-weight: 500;
  color: #a5b4fc;
  transition: all 0.2s;
}

.btn-link:hover {
  color: #fff;
  text-decoration: underline;
}

.message {
  margin-top: 16px;
  font-size: 14px;
  text-align: center;
}

.message.error {
  color: #f87171;
}

.message.success {
  color: #4ade80;
}
</style>
