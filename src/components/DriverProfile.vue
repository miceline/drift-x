<template>
  <v-row class="text-center">
    <v-col class="mb-5" cols="12">
      <v-row justify="start">
        <v-col cols="6">
          <v-card class="mx-1">
            <v-snackbar v-model="snackbar" absolute top right color="success">
              <span>Registration successful!</span>
              <v-icon dark> mdi-checkbox-marked-circle </v-icon>
            </v-snackbar>
            <v-form ref="form" @submit.prevent="submit">
              <v-card-text class="text-start">
                <p class="text-h5 text--primary">Driver Profile</p>
                <div class="text--primary">
                  <v-simple-table dense>
                    <template v-slot:default dense>
                      <tbody>
                        <tr>
                          <td>Competition number</td>
                          <td>
                          <v-text-field 
                            hide-details
                            v-model="currentDriver.driverCompNumber"
                            flat dense solo
                          ></v-text-field>
                          </td>
                        </tr>
                        <tr>
                          <td>Name</td>
                          <td>
                          <v-text-field 
                            hide-details
                            v-model="currentDriver.driverName"
                            flat dense solo
                          ></v-text-field>
                          </td>
                        </tr>
                        <tr>
                          <td>Car</td>
                          <td>
                          <v-text-field 
                            hide-details
                            v-model="currentDriver.carType"
                            flat dense solo
                          ></v-text-field>
                          </td>
                        </tr>
                        <tr>
                          <td>Team</td>
                          <td>
                          <v-text-field 
                            hide-details
                            v-model="currentDriver.teamName"
                            flat dense solo
                          ></v-text-field>
                          </td>
                        </tr>
                        <tr>
                          <td>score</td>
                          <td>
                          <v-text-field 
                            hide-details
                            v-model="currentDriver.totalScore"
                            flat dense solo
                          ></v-text-field>
                          </td>
                        </tr>
                      </tbody>
                    </template>
                  </v-simple-table>
                </div>
              </v-card-text>
              <v-card-actions>
                <v-btn text color="primary accent-4"> REFRESH DRIVER </v-btn>
                <v-btn text color="primary" type="submit"> Register </v-btn>
              </v-card-actions>
            </v-form>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="mx-1">
            <v-card-text class="text-start">
              <p class="text-h5 text--primary">Run Overview</p>
              <div class="text--primary">
                <v-simple-table>
                  <template v-slot:default dense>
                    <thead>
                      <tr>
                        <th class="text-left">Node Id</th>
                        <th class="text-left">Date</th>
                        <th class="text-left">Time</th>
                        <th class="text-left">Score</th>
                      </tr>
                    </thead>
                    <tbody>
                      <tr v-for="item in currentScores" :key="item.id">
                        <td>{{ item.id }}</td>
                        <td>{{ item.date }}</td>
                        <td>{{ item.time }}</td>
                        <td>{{ item.score }}</td>
                      </tr>
                      <tr>
                        <td></td>
                        <td class="text-xs-right"></td>
                        <td class="text-xs-right"><strong>Total Score</strong></td>
                        <td class="text-xs-right"><strong>{{ totalscore }}</strong></td>
                      </tr>
                    </tbody>
                  </template>
                </v-simple-table>
              </div>
            </v-card-text>
          </v-card>
        </v-col>
      </v-row>
    </v-col>
  </v-row>
</template>

<script>
// const worker =
//worker.postMessage('')

export default {
  name: "DriverProfile",
  props: {
    msg: String,
  },
  mounted: function () {
    if (localStorage.getItem("drivers")) {
      try {
        this.drivers = JSON.parse(localStorage.getItem("drivers"));
      } catch (e) {
        localStorage.removeItem("drivers");
      }
    }
    this.$root.$on("runEvent", (data) => {
      this.runEvent(data);
    });
    this.$root.$on("scoreEvent", (data) => {
      this.currentScores = data;
    });
    //currentRuns
  },
  data: function () {
    return {
      snackbar: false,
      drivers: [],
      currentDriver: {
        driverCompNumber: 2,
        driverName: "John Doe",
        carType: "1995 Nissan - 240SX",
        teamName: "Monster Drift",
        registrationDate: null,
        runs: [],
      },
      currentScores: [
          {
              id: 1,
              date: "",
              time: "",
              score: 0,
          },
          {
              id: 2,
              date: "",
              time: "",
              score: 0,
          }
      ],
    };
  },
  computed: {
    totalscore: function () {
       var total = 0; 
      for(var key in this.currentScores)
      {
          var record = this.currentScores[key];
          total += record.score;
      }  
      return total;
    },
  },
  methods: {
    resetForm() {
      this.form = Object.assign({}, this.defaultForm);
      this.$refs.form.reset();
    },
    submit() {
      this.snackbar = true;
      this.resetForm();
    },
    addDriver() {
      // ensure they actually typed something
      if (!this.currentDriver) {
        return;
      }

      this.drivers.push(this.currentDriver);
      //this.currentDriver = null;
      this.saveDrivers();
    },
    removeDriver(x) {
      this.drivers.splice(x, 1);
      this.saveDrivers();
    },
    saveDrivers() {
      const parsed = JSON.stringify(this.drivers);
      localStorage.setItem("drivers", parsed);
    },
    runEvent: function (data) {
      console.log("runEvent", data);
    },
  },
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

</style>
