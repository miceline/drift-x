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
            <v-overlay :value="overlay">
              <v-progress-circular
              indeterminate
              size="64"
              ></v-progress-circular>
            </v-overlay>

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
                            <div class="d-flex">
                              <v-text-field
                              hide-details
                              v-model="currentDriver.totalScore"
                              flat dense solo
                              :disabled="true"
                              ></v-text-field>
                              <v-btn text color="warning" @click="loadScore" :disabled="currentDriver.db_id == 0" > Load Score </v-btn>
                            </div>
                          </td>
                        </tr>
                      </tbody>
                    </template>
                  </v-simple-table>
                </div>
              </v-card-text>
              <v-card-actions>
                <v-btn text color="primary accent-4" type="submit"> Lookup Driver </v-btn>
                <v-btn text color="primary" :disabled="!registration || currentDriver.db_id != 0" @click="registerDriver"> Register </v-btn>
                <v-btn text color="primary" :disabled="currentDriver.db_id == 0" @click="updateDriver"> Update </v-btn>
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
                        <td><v-btn text color="primary accent-4" :disabled="!(saveState && currentDriver.db_id != 0)" @click="saveRun"> Save Run </v-btn></td>
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
    <Modal
      v-show="modalVisible"
    />
  </v-row>

</template>

<script>
// const worker =
//worker.postMessage('')

import Modal from './Modal.vue';

export default {
  name: "DriverProfile",
  components: {
      Modal,
    },
  props: {
    msg: String,
  },
  mounted: function () {
    this.$root.$on("runEvent", (data) => {
      this.runEvent(data);
    });
    this.$root.$on("scoreEvent", (data) => {
      this.currentScores = data;
    });
    this.$root.$on("armEvent", (data) => {
      console.log('armEvent', data);
      this.armId = data.armId ? data.armId : this.armId;
      this.armState = data.armState;
    });
    this.$root.$on("raceMode", (data) => {
      //console.log('raceMode', data);
      this.raceMode = data;
      if(this.raceMode.mode === 'trail') this.raceMode.round=0;
    });
    //raceMode
    //currentRuns
  },
  data: function () {
    return {
      modalVisible: false,
      raceMode: {
        mode: null,
        round: null,
      },
      armId: 0,
      armState: false,
      saveState: false,
      overlay: false,
      registration: false,
      snackbar: false,
      // drivers: [],
      currentDriver: {
        db_id: 0,
        driverCompNumber: '',
        driverName: '',
        carType: '',
        teamName: '',
        registrationDate: null,
        runs: [],
        totalScore: 0,
      },
      currentScores: [],
    };
  },
  watch: {
    currentScores(newValue, oldValue){
      console.log('newValue',newValue,'oldValue',oldValue);
      let finished = false;
      for(const scoreNode of newValue)
      {
        if(scoreNode.score)finished = true;
      }
      if(finished){
        //stop timer
        this.saveState = true;
      }else{
        this.saveState = false;
      }
    }
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
    loadScore() {
      this.findScores(this.currentDriver.db_id);
    },
    overlayOn() {
      this.overlay = true;
    },
    overlayOff() {
      this.overlay = false;
    },
    submit() {
      this.findDriver(this.currentDriver.driverCompNumber);
    },
    resetForm() {
      this.form = Object.assign({}, this.defaultForm);
      this.$refs.form.reset();
    },
    registerDriver() {
      this.addDriver();
      this.snackbar = true;
      //this.resetForm();
    },
    addDriver(){
      this.overlayOn();
      const requestOptions = {

        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Authorization': `Token ${this.$root.$children[0].$data.apiKey}`,
        },
        body: JSON.stringify(
          {
            'Driver Name': this.currentDriver.driverName,
            'Competition Number': this.currentDriver.driverCompNumber,
            'Car Type': this.currentDriver.carType,
            'Team': this.currentDriver.teamName
          })
        };
        fetch('https://api.baserow.io/api/database/rows/table/67752/?user_field_names=true', requestOptions)
        .then(response => response.json())
        .then(data => {
          console.log(data.id);
          this.registration = false;
          this.findDriver(this.currentDriver.driverCompNumber);
          this.overlayOff();
        });
      },
      findDriver(compNumber){
        const requestOptions = {

          method: 'GET',
          headers: {
            'Content-Type': 'application/json',
            'Authorization': `Token ${this.$root.$children[0].$data.apiKey}`,
          },
        };
        var vm = this;
        fetch('https://api.baserow.io/api/database/rows/table/67752/?user_field_names=true&filter__field_402680__equal='+compNumber, requestOptions)
        .then(response => {
          if(response.status==401)
           {
              throw new Error('Authentication please check api key...')
           }
          response.json()
        })
        .then(data => {
          console.log(data);
          if(data.count == 1)
          {
            this.currentDriver.driverName = data.results[0]['Driver Name'];
            this.currentDriver.driverCompNumber = data.results[0]['Competition Number'];
            this.currentDriver.carType = data.results[0]['Car Type'];
            this.currentDriver.teamName = data.results[0]['Team'];
            this.currentDriver.db_id = data.results[0]['id'];
          }else{
            //enable registration
            this.currentDriver.driverName = '';
            this.currentDriver.carType = '';
            this.currentDriver.teamName = '';
            this.currentDriver.db_id = 0;
            this.registration = true;
          }
        })
        .catch(function(data)
        {
          console.log('error data',data);
          console.log(this);
          vm.modalVisible = true;

          setTimeout(function() {
          // This does not work, you need the outside context view model.
          //this.sendButtonDisable = true;

          // This works, since wm refers to your view model.
          vm.modalVisible = false;
        }, 3000); 

        });
      },
      updateDriver(){
        this.overlayOn();
        const db_id = this.currentDriver.db_id;
        const requestOptions = {

          method: 'PATCH',
          headers: {
            'Content-Type': 'application/json',
            'Authorization': `Token ${this.$root.$children[0].$data.apiKey}`,
          },
          body: JSON.stringify(
            {
              'Driver Name': this.currentDriver.driverName,
              'Car Type': this.currentDriver.carType,
              'Team': this.currentDriver.teamName
            })
          };
          fetch(`https://api.baserow.io/api/database/rows/table/67752/${db_id}/?user_field_names=true`, requestOptions)
            .then(response => response.json())
            .then(data => {
              console.log(data.id);
              this.registration = false;
              this.findDriver(this.currentDriver.driverCompNumber);
              this.overlayOff();
            });
          },
          findScores(db_id){
            const requestOptions = {

              method: 'GET',
              headers: {
                'Content-Type': 'application/json',
                'Authorization': `Token ${this.$root.$children[0].$data.apiKey}`,
              },
            };
            fetch('https://api.baserow.io/api/database/rows/table/67757/?user_field_names=true&filter__field_402705__link_row_has='+db_id, requestOptions)
            .then(response => response.json())
            .then(data => {
              console.log(data);
              if(data.count > 0)
              {
                this.currentDriver.totalScore = 0;
                data.results
                const sumScores = {};

                for (const run of data.results) {
                  sumScores[run['Run Unique Id']] = sumScores[run['Run Unique Id']]? sumScores[run['Run Unique Id']] += (','+run['Score']) : run['Score'];
                }

                console.log(sumScores);
                const scoreKeys = Object.keys(sumScores);
                for(const event of scoreKeys)
                {
                  console.log(event);
                  const eventValues = sumScores[event];
                  const eventScores = eventValues.split(",");
                  var totalSum =0;
                  for(const val of eventScores)
                  {
                    totalSum += parseInt(val);
                    console.log(event, totalSum);
                  }
                  console.log(event, totalSum, eventScores.length);
                  this.currentDriver.totalScore += (totalSum/eventScores.length);
                  console.log('this.currentDriver.totalScore', this.currentDriver.totalScore);
                }
              }

            });
          },
          saveRun() {
            for(const item of this.currentScores)
            {
              this.saveRunItem(item);
            }
          },
          saveRunItem(object){
            const requestOptions = {

              method: 'POST',
              headers: {
                'Content-Type': 'application/json',
                'Authorization': `Token ${this.$root.$children[0].$data.apiKey}`,
              },
              body: JSON.stringify(
              {
                'Run Unique Id': object.systemArmId != null ? object.systemArmId:this.armId,
                'Date': object.date,
                'Time': object.time,
                'Node Id': object.id,
                'Score': object.score?object.score:0,
                'Race Mode': this.raceMode.mode? this.raceMode.mode : '',
                'Race Round': this.raceMode.round? this.raceMode.round: '',
                'Driver Id': [
                    this.currentDriver.db_id
                ]
              }),
            };
            fetch('https://api.baserow.io/api/database/rows/table/67757/?user_field_names=true', requestOptions)
            .then(response => response.json())
            .then(data => {
              console.log(data);
              this.saveState = false;

            });
          },

          runEvent: function (data) {
            console.log("runEvent", data);
          },
        },
      };
      </script>

      <!-- Add "scoped" attribute to limit CSS to this component only -->
      <style scoped>
      .together{
        min-width: 0
        padding = 0
      }
      </style>
