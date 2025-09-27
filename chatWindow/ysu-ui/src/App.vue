<template>
  <ChatWindow :setRoute="setRoute" v-if="route==='chat'"/>
  <loginRegister :setRoute="setRoute" v-if="route==='login'"/>
</template>

<script setup>
import {ref} from 'vue'
import ChatWindow from '@/components/ChatWindow.vue'
import loginRegister from './components/loginRegister.vue';

const route=ref('chat')
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
