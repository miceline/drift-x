<template>
  <v-app>
    <v-app-bar
      app
      color="primary"
      dark
    >
      <div class="d-flex align-center">
        <h1>Drift Pylon App</h1>
      </div>

      <v-spacer></v-spacer>


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

  data: () => ({
    //
  }),
};
</script>
