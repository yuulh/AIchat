<template>
  <ChatWindow :setRoute="setRoute" v-if="route==='chat'"/>
  <loginRegister :setRoute="setRoute" v-if="route==='login'"/>
</template>

<script setup>
import {ref} from 'vue'
import ChatWindow from '@/components/ChatWindow.vue'
import loginRegister from './components/loginRegister.vue';

const route=ref(localStorage.getItem('user')?'chat':'login')
//route.value='chat'
//localStorage.getItem('user')?'chat':'login'
function setRoute(e){
  route.value=e;
}
</script>

<style>
html,
body {
  overflow: hidden;
  margin:0px;
}
</style>
