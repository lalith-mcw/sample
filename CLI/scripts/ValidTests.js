//ValidTests.js

var exports = module.exports = {};
const Logger = require("./Logger.js");
const spawnSync = require('child_process').spawnSync;
const fs = require('fs');

exports.generateValidTestsJson = function(workloadsList) {

  for (let workloadIndex = 0; workloadIndex < workloadsList.length; workloadIndex++ ) {
    workload = workloadsList[workloadIndex];
    try {
      if (!fs.existsSync(workloadPath + "pezcore.exe")) {
          throw Error('pezcore.exe doesnot exist');
      }
      const childProcess = spawnSync(workloadPath + "pezcore.exe -tests " + workloadPath + workload + "\\workload.json --query",[], { shell: true});
    }
    catch(err) {
      Logger.error(workload + ":Unable to generate validTests.json -- "+ err);
    }
  }

  Logger.log("validTests.json generation completed\n");
}

exports.checkTests = function(workloadList) {

  for (workload in workloadList) {
    try {
      var workloadPath = global.workloadPath + workloadList[workload];
      var jsonFile = workloadPath + "/validTests.json";

      const jsonString = fs.readFileSync(jsonFile);
      const workloadObj = JSON.parse(jsonString);

      var totalSubtest = Object.keys(workloadObj["tests"]).length;
      var invalidSubtest = 0;

      var workloadArray = []; // To store invalid subtests
      for (var subtest in workloadObj["tests"]) {
        var errorInSubTest = false;
        if (workloadObj["tests"][subtest]["errors"]) {
          var subtests = {};
          subtests = {name:workloadObj["tests"][subtest]["name"],
                      error:workloadObj["tests"][subtest]["errors"]};
          errorInSubTest = true;
        }
        if (workloadObj["tests"][subtest]["parameters"]) {
          var numberOfParameters = workloadObj["tests"][subtest]["parameters"].length;
          for (var param in workloadObj["tests"][subtest]["parameters"]) {
              if (workloadObj["tests"][subtest]["parameters"][param]["errors"]) {
              var subtests = {};
              subtests = {name:workloadObj["tests"][subtest]["name"],
                          error:workloadObj["tests"][subtest]["parameters"][param]["errors"]};
              errorInSubTest = true;
              }
          }
        }
        if (errorInSubTest) {
          workloadArray.push(subtests);
          invalidSubtest +=1;
        }
      }

      if (invalidSubtest < totalSubtest) {  //Workload is valid if atleast one subtest is valid
         validTests.push(workloadObj["workloadName"]);
      }
      if (invalidSubtest > 0) {
         invalidTests[workloadObj["workloadName"]] = {name:[workloadObj["workloadName"]], error:workloadArray};
      }
    }
    catch (err) {
      Logger.error(" Parsing validTests.json failed: " + err);
    }
  }

  if (validTests.length > 0) {
    Logger.log("Valid Test list: ");
    for(var subtest in validTests) {
      Logger.log("  " + validTests[subtest]);
    }
    Logger.log("\n");
  }

  if (Object.keys(invalidTests).length) {
    Logger.log("Invalid Test list: ");
    for(var workload in invalidTests) {
      var invalidList = "";

      //Print Invalid list in specific format
      for (var subtest in invalidTests[workload].error) {
        invalidList += invalidTests[workload].error[subtest].name + ": " +
                       invalidTests[workload].error[subtest].error + "\n        ";
      }
      Logger.log("  " + invalidTests[workload].name + "\n        " + invalidList);
    }
    Logger.log("\n");
  }

  return;
}
