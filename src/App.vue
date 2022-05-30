<template>
  <v-app>
    <v-app-bar
      app
      color="primary"
      dark
    >
      <div class="d-flex align-center">
        <h1>Drift X</h1>

      </div>

      <v-spacer></v-spacer>
      <div>
        Please enter api key: <input type="password" v-model="apiKey" data-lpignore="true">
      </div>
    </v-app-bar>

    <v-main>
      <v-container fluid>

      <SerialConnector/>
      <DriverProfile/>
      <SerialView/>
      </v-container>
    </v-main>
  </v-app>
</template>

<script>
import SerialConnector from './components/SerialConnector';
import SerialView from './components/SerialView';
import DriverProfile from './components/DriverProfile'

export default {
  name: 'App',

  components: {
    SerialConnector,
    SerialView,
    DriverProfile,

  },
  mounted: function () {
    if (localStorage.getItem("apiKey")) {
      try {
        this.apiKey = localStorage.getItem("apiKey");
      } catch (e) {
        localStorage.removeItem("apiKey");
      }
    }
  },
  data: () => ({
    apiKey: null,
    //
  }),
  watch: {
    // whenever question changes, this function will run
    apiKey(newKey, oldKey) {
      if (newKey != null && newKey != oldKey) {
        this.apiKey = newKey;
        localStorage.setItem("apiKey", newKey);
      }
    }
  },
};
</script>
<style scoped>
input{
  background-color: rgb(255 255 255 / 19%);
}
</style>
