<template>
  <div class="columns is-fullheight">
      Hello, {{ hello }}
  </div>
</template>

<script>
    export default {
        props: {
            hello: {
                type: String,
                default: 'Hello world',
            }

        },
        components: {
        },
        methods: {
        },
        watch: {
        },
        created: function() {
        }
    }
</script>

<style scoped>
</style>
