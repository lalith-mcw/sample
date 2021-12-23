//OfficialJson.js

const Logger = require("./Logger.js");
const Helper = require("./Helper.js");
const fs = require("fs");
const Hash = require('object-hash');

exports.createOfficialJSON = function() {

  var officialWorkloads = Object.keys(Helper.getAvailableWorkloads());

  var jsonData = [];
  for (var i=0; i < officialWorkloads.length; i++) 
  {
    var workload = workloadPath+officialWorkloads[i];

    const jsonFile = fs.readFileSync(workload + "/workload.json");
    const workloadJson = JSON.parse(jsonFile);
    var workloadName = workloadJson["workloadName"];
    var workloadVersion = workloadJson["workloadVersion"];
    var hashvalue = Hash.MD5(jsonFile);

    jsonData.push({name: workloadName, version: workloadVersion, hash: hashvalue});
  }

  fs.writeFileSync('officialRun.json', JSON.stringify({workloads : jsonData}, null, '\t'),function (err) {
    if (err) {
      Logger.error("Error occured while writing JSON Object to File: "+ err);
      return;
    }
  });

  Logger.log("OfficialRun.json generated");
}
