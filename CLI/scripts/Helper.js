// Helper.js 

var exports = module.exports = {};
const Logger = require("./Logger.js");
const fs = require('fs');
const Hash = require('object-hash');

exports.showWorkloadList = function(workloadList) {
  var displayList = "";   //Display one workload per line

  for (var workload in workloadList) {
    displayList += workloadList[workload].name + " [" + workloadList[workload].version + "]";
    if ( workloadList[workload].hasOwnProperty('hash') && workloadList[workload].hash !== officialList[workload].hash)
        displayList += "  (Warning! workload.json Hash mismatch)";
    displayList +="\n";
  }

  return displayList;
}

exports.getAvailableWorkloads = function() {
  var detectedWorkloads = {};

  fs.readdirSync(workloadPath).forEach(file => {
    if (fs.existsSync(workloadPath+file+"/workload.json")) {
      //Retrieve workload info from workload.json
      const jsonFile = fs.readFileSync(workloadPath + file + "/workload.json");
      const workloadJson = JSON.parse(jsonFile);
      var hashvalue = Hash.MD5(jsonFile);
      detectedWorkloads[workloadJson["workloadName"]] = { name:workloadJson["workloadName"],
                                                          version:workloadJson["workloadVersion"],
                                                          hash:hashvalue};
    }
  });

  return detectedWorkloads;
}

exports.getOfficialWorkloads = function() {
  var list = {};
  if (fs.existsSync("./officialRun.json")) {
    //Retrieve official workloads list
    const jsonFile = fs.readFileSync("./officialRun.json");
    const workloadJson = JSON.parse(jsonFile);

    for(var workload in workloadJson['workloads']) {
      list[workloadJson['workloads'][workload]["name"]] = workloadJson['workloads'][workload];
    }
  }
  else {
    Logger.error("./officialRun.json" + " not found");
  }
  return list;
}

exports.compare = function() {
  var missingWorkloads = {};

  for (var workload in officialList) {
    let officialName = officialList[workload].name;
    let officialHash = officialList[workload].hash;

    if (!availableList.hasOwnProperty(officialName) || (availableList[officialName].hash != officialHash)) {
      //TODO: Specify whether workload unavailable or hash mismatch
      missingWorkloads[officialName] = { name:officialName, version:officialList[workload].version};
    }
  }

  if (Object.keys(missingWorkloads).length > 0) {
    Logger.error("Cannot initiate an official run, required workloads are missing\\hash mismatch:\n" +
                 module.exports.showWorkloadList(missingWorkloads));
    return false;
  }

  return true;
}

exports.printWorkloadList = function() {
  Logger.log("Available workloads:\n" + module.exports.showWorkloadList(module.exports.getAvailableWorkloads()));
}

exports.end = function(resultFolders) {
  Logger.log("\nRun complete -- results and logs available in: ");
  for (var i=0; i < resultFolders.length; i++) {
    Logger.log("[Iteration " + (i+1) + "] "+ resultFolders[i]);
  }
  process.exit();
}
