<template>
  <v-row class="text-center">
    <v-col cols="12">
      <div id="notSupported" class="hidden">
        Sorry, <b>Web Serial</b> is not supported on this device, make sure
        you're running Chrome 78 or later and have enabled the
        <code>#enable-experimental-web-platform-features</code> flag in
        <code>chrome://flags</code>
      </div>
    </v-col>
    <v-col class="mb-5" cols="12">



      <v-row justify="start">
        <h2 class="headline font-weight-bold mb-3 pa-3">Connected Nodes ({{nodes.length}})</h2>
        <v-btn
          class="ma-1"
          color="secondary"
          @click="topologyRefresh"
        >
        refresh
      </v-btn>
      </v-row>

      <v-row justify="start">
        <v-col cols="6" md="3" v-for="item in nodes" :key="item.nodeId">
          <v-card
            class="mx-1"
            :color="item.online ? '' : 'blue-grey lighten-5'"
          >
            <v-card-text class="text-start">
              <div>
                State -
                <v-chip
                  class="ma-2"
                  small
                  outlined
                  :color="item.nodeArmed ? 'red' : 'green'"
                  :text-color="item.nodeArmed ? 'red' : 'green'"
                >
                  {{ item.nodeArmed ? "Armed" : "Disarmed" }}
                </v-chip>
              </div>
              <p class="text-h5 text--primary">{{ item.name }}</p>
              <div class="text--primary">
                <v-simple-table>
                  <template v-slot:default dense>
                    <thead>
                      <tr>
                        <th class="text-left">Key</th>
                        <th class="text-left">Value</th>
                      </tr>
                    </thead>
                    <tbody>
                      <!-- <tr class="background-color: primary">
                        <td class="white--text text--lighten-1">Run info</td>
                        <td></td>
                      </tr>
                      <tr>
                        <td>date</td>
                        <td>{{ item.runDate }}</td>
                      </tr>
                      <tr>
                        <td>time</td>
                        <td>{{ item.runTime }}</td>
                      </tr>
                      <tr>
                        <td>score</td>
                        <td>{{ item.score }}</td>
                      </tr>
                      <tr>
                        <td>average</td>
                        <td>{{ item.avg }}</td>
                      </tr>
                      <tr>
                        <td>sample size</td>
                        <td>{{ item.sampleSize }}</td>
                      </tr> -->
                      <tr class="background-color: primary">
                        <td class="white--text text--lighten-1">Node Info</td>
                        <td></td>
                      </tr>
                      <tr>
                        <td>node id</td>
                        <td>{{ item.nodeId }}</td>
                      </tr>
                      <tr>
                        <td>node date</td>
                        <td>{{ item.nodeDate }}</td>
                      </tr>
                      <tr>
                        <td>node time</td>
                        <td>{{ item.nodeTime }}</td>
                      </tr>
                      <tr>
                        <td>acceptanceZone</td>
                        <td>{{ item.acceptanceZone }}</td>
                      </tr>
                      <tr>
                        <td>triggerZone</td>
                        <td>{{ item.triggerZone }}</td>
                      </tr>
                      <tr>
                        <td>freeMem</td>
                        <td>{{ item.freeMem }}</td>
                      </tr>
                    </tbody>
                  </template>
                </v-simple-table>
              </div>
            </v-card-text>
            <v-card-actions>
              <v-btn text color="primary accent-4"> Config Node </v-btn>
            </v-card-actions>
          </v-card>
        </v-col>
      </v-row>

      <v-row justify="start">
        <v-col class="mb-5" cols="12">
          <v-row justify="start">
            <h2 class="headline font-weight-bold mb-3 pa-3">Scoring Logs</h2>
          </v-row>
          <v-row justify="start">
            <v-data-table
              :headers="headers"
              :items="log"
              :items-per-page="5"
              class="elevation-1"
            ></v-data-table>
          </v-row>
        </v-col>
      </v-row>

    </v-col>
  </v-row>
</template>

<script>
// const worker =
//worker.postMessage('')

import moment from 'moment';

export default {
  name: "SerialView",
  props: {
    msg: String,
  },
  // watch: {
  //   nodes: {
  //     handler(val, oldVal) {
  //       console.log("nodes data changed");
  //       this.emitNewScrore();
  //     },
  //     deep: true,
  //   },
  // },
  created() {
    window.addEventListener('keydown', (e) => {
      if (e.key == '/') {
        this.clearRun();
      }
    });
  },
  mounted: function () {
    this.checkForSerialAccess();
    this.$root.$on("serialEvent", (data) => {
      // here you need to use the arrow function
      this.serialData(data);
    });

    this.$root.$on("runEvent", (data) => {
      // here you need to use the arrow function
      if (data == "clear") {
        this.clearRun();
      }else if(data == "offlineDevices")
      {
        for (let key in this.nodes) {
          let node = this.nodes[key];
          node.online = false;
        }
      }
    });
  },
  data: function () {
    return {
      headers: [
          {
            text: 'nodeId',
            align: 'start',
            sortable: false,
            value: 'nodeId',
          },
          { text: 'Time', value: 'time' },
          { text: 'Score', value: 'score' },
          { text: 'Sample Size', value: 'sampleSize' },
          { text: 'LowestNum', value: 'lowestNum' },
          { text: 'Average', value: 'avg' },
        ],
      nodes: [],
      connected: false,
      log: [],
      nodeConfig: [{
        nodeId: 886187420,
        nodeName: 'Enrick Device1',
      },{
        nodeId: 886192323,
        nodeName: 'Enrick Device2',
      }],
    };
  },
  methods: {
    clearRun: function() {
      this.log = [];
        //clear event
        for (let key in this.nodes) {
          let node = this.nodes[key];
          node.runDate = "";
          node.runTime = "";
          node.nodeArmed = false;
          node.systemArmId = null;
          node.recording = null;
          node.lowestNum = null;
          node.avg = null;
          node.score = null;
          node.sampleSize = null;
          node.acceptanceZone = null;
          node.triggerZone = null;
        }
        this.emitNewScore();
    },
    checkForSerialAccess: function () {
      const notSupported = document.getElementById("notSupported");
      notSupported.classList.toggle("hidden", "serial" in navigator);
    },
    serialData: function (data) {
      console.log('SerialView','serialData: ', data)
      if (data.msgType == "disarm_system_ack") {
        this.nodes.find((node) => node.nodeId === data.nodeId).nodeArmed = false;
        //this.log.push("data recevied");
      }
      if (data.msgType == "arm_system_ack") {
        this.nodes.find((node) => node.nodeId === data.nodeId).nodeArmed = true;
        //this.log.push("data recevied");
      }
      if (data.msgType == "ping") {
        let node = this.nodes.find((node) => node.nodeId == data.nodeId);
        if(node != null)
        {
          let time = data.time;
          time = time.trim();
          let momentDate = moment(time, 'ddd MMM DD HH:mm:ss YYYY');
          node.nodeDate = momentDate.format('YYYY-MM-DD');
          node.nodeTime = momentDate.format('HH:mm:ss');
          node.online = true;
          node.freeMem = data.freeMem;
        }
      }
      if (data.msgType == "version") {
        console.log(this.$parent);
        this.$parent.$data.boardType = data.boardType;
        this.$parent.$data.boardVersion = data.version;
      }
      if (data.msgType == "scoring") {
        let node = this.nodes.find((node) => node.nodeId === data.nodeId);
        console.log('scoring-find-node', node);
        let time = data.time;
        time = time.trim();
        node.online = true;
        //console.log('node.sampleSize',node.sampleSize);
        if(data.sampleSize > 6){
          //Thu Jan 1 00:26:30 1970
          if(node.score == null)
          {
            let momentDate = moment(time, 'ddd MMM DD HH:mm:ss YYYY');
            node.runDate = momentDate.format('YYYY-MM-DD');
            node.runTime = momentDate.format('HH:mm:ss');
            //node.nodeId = data.nodeId;
            node.acceptanceZone = data.acceptanceZone;
            node.avg = data.avg;
            node.freeMem = data.freeMem;
            node.lowestNum = data.lowestNum;
            node.sampleSize = data.sampleSize;
            node.score = data.score;
            node.systemArmId = data.systemArmId;
            node.triggerZone = data.triggerZone;
          }
          this.log.unshift(data);
          this.emitNewScore();
        }
      }
      //{"msgType":"node-update","parentNodeId":2559706015,"timeZone":"GMT","nodeId":2559706015,"topology":{"nodeId":2559706015,"root":true,"subs":[{"nodeId":2559771790}]}}
      if (data.msgType == "node-update") {
        //let topology = data.topology;
        //let topologyNodes = topology.subs; //array
        let topologyNodes = data.topology;

        //remove what is not there
        //this.nodes = [];
        console.log('[nodes-cleaup]', '[map]', this.nodes)
        for(let key in this.nodes)
        {
          console.log('[nodes-cleaup]', '[key]', key)
          let nodeKey = this.nodes[key];
          console.log('[nodes-cleaup]','[nodeKey]', nodeKey);
          let node = topologyNodes.find((id) => id === nodeKey.nodeId);
          if(node == null)
          {
            console.log('[nodes-cleaup]', '[key-delete]',node, key)
            this.$delete(this.nodes, key);
          }
        }

        for (let key in topologyNodes) {
          let subNodeId = topologyNodes[key];
          //let subNodeId = sub.nodeId;
          let node = this.nodes.find((node) => node.nodeId === subNodeId);
          if(node == null)
          {
            let nextNode = "node"  + (this.nodes.length +1);
            let nodeInfo = this.nodeConfig.find((node) => node.nodeId == subNodeId);
            if(nodeInfo) nextNode = nodeInfo.nodeName;
            this.nodes.push({
              online: false,
              nodeDate: "",
              nodeTime: "",
              runDate: "",
              runTime: "",
              nodeArmed: false,
              systemArmId: null,
              recording: null,
              lowestNum: null,
              avg: null,
              score: null,
              sampleSize: null,
              name: nextNode,
              nodeId: subNodeId,
              freeMem: null,
              acceptanceZone: null,
              triggerZone: null,
            });
          }
        }

        //node.online = true;
        //node.nodeId = data.nodeId;
      }
    },
    emitNewScore: function () {
      let scoreList = [];
      for (const node of this.nodes) {
        //let node = this.nodes[key];
        scoreList.push({
          id: node.nodeId,
          nodeDescription: node.name,
          date: node.runDate,
          time: node.runTime,
          score: node.score,
          systemArmId: node.systemArmId,
        });
      }

      this.$root.$emit("scoreEvent", scoreList);
    },
    async topologyRefresh() {
      var line = {
        action: "topology_refresh"

        };
      let lineStr = JSON.stringify(line) + '\n';
      console.log('[debug]', this.$parent);
      await this.$parent.$children[0].sendCommand(lineStr);
    }
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

.hidden {
  display: none;
}

#notSupported {
  padding: 1em;
  background-color: red;
  color: white;
  margin-top: 1em;
  margin-bottom: 1em;
}
</style>
