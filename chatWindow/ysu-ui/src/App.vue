<template>
  <ChatWindow v-if="route==='chat'"/>
  <loginRegister :route="route" v-if="route==='login'"/>
</template>

<script setup>
import ChatWindow from '@/components/ChatWindow.vue'
import loginRegister from './components/loginRegister.vue';

let route='login'
</script>

<style>
html,
body {
  overflow: hidden;
}
</style>
