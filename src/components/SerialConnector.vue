<template>
  <v-row>
    <v-col class="mb-5" cols="12">
      <v-row class="justify-end">
        <div class="text-center">
          {{this.boardType}} - {{this.boardVersion}}
          <v-btn
            class="ma-1 white--text"
            :loading="loadingConnect"
            :disabled="loadingConnect"
            :color="connected? 'red' : 'secondary'"
            @click="connect"
          >
            {{ isConnected }}
            <template v-slot:loader>
              <span>Connecting...</span>
            </template>
          </v-btn>

          <v-btn
            class="ma-1"
            :loading="loadingConnect"
            :disabled="!connected"
            color="secondary"
            @click="arm"
          >
            Arm
          </v-btn>

          <v-btn
            class="ma-1"
            :loading="loadingConnect"
            :disabled="!connected"
            color="secondary"
            @click="disarm"
          >
            Disarm
          </v-btn>

          <v-btn
            class="ma-1"
            :loading="loadingConnect"
            :disabled="loadingConnect"
            color="secondary"
            @click="clearRun"
          >
            Clear Run
          </v-btn>

          <v-btn
            :loading="loadingConnect"
            :disabled="!connected"
            color="blue-grey"
            class="ma-1 white--text"
            @click="ping"
          >
            Ping
            <v-icon right dark> mdi-cloud-upload </v-icon>
          </v-btn>
        </div>
      </v-row>
    </v-col>
  </v-row>
</template>

<script>
// const worker =
//worker.postMessage('')

import moment from 'moment';

export default {
  name: "SerialConnector",
  props: {
    msg: String,
  },
  mounted: function () {},
  data: function () {
    return {
      boardType: 'aaa',
      boardVersion: '',
      readThread: null,
      connected: false,
      serial: {
        obj: null,
        port: null,
        reader: null,
        writer: null,
        inputDone: null,
        outputDone: null,
        inputStream: null,
        outputStream: null,
        decoder: new TextDecoder(),
        byteArray: [],
        worker: null,
      },
      keepReading: true,
      loadingConnect: false,
    };
  },
  computed: {
    isDisabled: function () {
      return !this.connected;
    },
    isConnected: function() {
      return this.connected? 'Disconnect' : 'connect';
    }
  },
  methods: {
    read: function (reader) {
      reader.read()
      .then((data) => {
        // console.log('data', data);
        let done = data.done;
        let value = data.value;
        if (done) {
          // Allow the serial port to be closed later.
          reader.releaseLock();
          return;
        }
        //console.log(value);
        let decoder = new TextDecoder();
        let result = decoder.decode(value);
        for (var i = 0; i < result.length; ++i) {
          this.serial.byteArray.push(result.charCodeAt(i) & 0xff);
        }
        let crIndex = 0;
        let instructionCount = 0;
        do {
          instructionCount++;
          crIndex = this.serial.byteArray.indexOf(13);
          crIndex++;
          if (crIndex > 0) {
            let byteArray = [];
            for (var y = 0; y < crIndex; y++) {
              //console.log('idx', this.serial.byteArray[y]);
              byteArray.push(this.serial.byteArray[y]);
            }
            const bytesString = String.fromCharCode(...byteArray);
            let jsonCandidate = bytesString.trim();
            //console.log('jsonCandidate', jsonCandidate);
            try {
              let json = JSON.parse(jsonCandidate);
              console.log(
                Date.now(),
                "instructionCount",
                instructionCount,
                json
              );
              this.$root.$emit("serialEvent", json);
            } catch (e) {
              console.log("unparseble data", jsonCandidate);
            }

            let byteArrayCopy = [];
            for (var x = crIndex; x < this.serial.byteArray.length; x++) {
              byteArrayCopy.push(this.serial.byteArray[x]);
            }
            this.serial.byteArray = byteArrayCopy;
            //console.log(this.serial.byteArray);
          }
        } while (crIndex > 0);

        // this.lineBuffer += result;
        // const splitLines = this.lineBuffer.split(/\r?\n/);
        // console.log('result:', splitLines);
        // for(let i =0; i < splitLines.length; i++)
        // {
        //   this.log.push(splitLines[i]);
        // }
      })
      .catch((error) => {
        console.log('serial_connector', error);
        this.disconnect();
      });
    },
    clearRun: function() {
      this.$root.$emit("runEvent", "clear");
    },
    connect: function () {
      if(this.connected)
      {
        this.disconnect();
      }else{
        this.loadingConnect = true;
        navigator.serial.requestPort().then((port) => {
          this.port = port;
          console.log(port);
          port.open({ baudRate: 115200 }).then(() => {
            this.reader = port.readable.getReader();
            port
              .getSignals()
              .then((signals) => {
                console.log(signals);
                this.connected = true;
              })
              .then(() => {
                this.readThread = setInterval(() => this.read(this.reader), 100);
              })
              .then(() => {
                let line = '\n{"action":"version"}\n';
                this.sendCommand(line);
              });
          });
        });
        this.loadingConnect = false;
      }
    },
    async disconnect () {
      this.boardType = '';
      this.boardVersion = '';
      clearInterval(this.readThread);
      const writer = this.port.writable.getWriter();
      writer.releaseLock();
      //const reader = this.port.reader.getReader();
      this.reader.cancel();
      this.reader.releaseLock();


      await this.port.close();
      this.connected = false;
      this.reader =null;
      this.write = null;
    },
    //{"action":"arm"}
    async arm() {
      this.$root.$emit("runEvent", "clear");
      var line = '{"action":"arm"}\n';
      await this.sendCommand(line);
    },
    async disarm() {
      var line = '{"action":"disarm"}\n';
      await this.sendCommand(line);
    },
    async ping() {
      this.$root.$emit("runEvent", "offlineDevices");
      //        setTime(
      // input["hr"],
      // input["min"],
      // input["sec"],
      // input["day"],
      // input["month"],
      // input["yr"]
      // );
      var line = {
        action: "ping",
        hr: moment().get('hour'),
        min: moment().get('minute'),
        sec: moment().get('second') ,
        day: moment().get('date'),
        month: moment().get('month'), //need to be from 0 to 11
        yr: moment().get('year')

        };
      let lineStr = JSON.stringify(line) + '\n';
      await this.sendCommand(lineStr);
    },
    async sendCommand(line) {
      console.log("[SEND]", line);
      const writer = this.port.writable.getWriter();
      var uint8array = new TextEncoder("utf-8").encode(line);
      await writer.write(uint8array);
      // Allow the serial port to be closed later.
      writer.releaseLock();
    },
    writeToStream: function (...lines) {
      const writer = this.outputStream.getWriter();
      lines.forEach((line) => {
        console.log("[SEND]", line);
        writer.write(line + "\n");
      });
      writer.releaseLock();
    },
  },
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
h3 {
  margin: 40px 0 0;
}
ul {
  list-style-type: none;
  padding: 0;
}
li {
  display: inline-block;
  margin: 0 10px;
}
a {
  color: #42b983;
}

/**
* Main body
*/

.main {
  flex: 1;
  overflow-x: hidden;
  overflow-y: auto;
  padding-top: 70px;
  padding-left: 1em;
  padding-right: 1em;
}
</style>
