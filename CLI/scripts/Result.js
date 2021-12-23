//Result.js

var exports = module.exports = {};
const Logger = require("./Logger.js");
const fs = require('fs');
const spawnSync = require('child_process').spawnSync;

function moveFile(sourcePath, destPath)
{
    spawnSync('cp', [sourcePath, destPath]);
    fs.unlinkSync(sourcePath);
}

exports.manageResults = function(benchmarkList, benchmarkIndex, iteration, resultPath)
{
  var result = (benchmarkIndex == 0) ? 'Test Name, Score, Units\n' : '';
  var resultStatus = true;

  try {
    var workloadPath = global.workloadPath + benchmarkList[benchmarkIndex];  //TODO : Rename local workloadPath variable
    var jsonFile = workloadPath + "/result.json";
    resultWorkloadPath = resultPath + benchmarkList[benchmarkIndex];

    if (!fs.existsSync(resultWorkloadPath)) {
      fs.mkdirSync(resultWorkloadPath+"/");
    }

    if (fs.existsSync(jsonFile)) {
      const jsonString = fs.readFileSync(jsonFile);
      const workload = JSON.parse(jsonString);

      //Process Overall workload score
      if (workload["errors"]) {
        result += workload["workloadName"] + "," + "Error" + "\n";
      }
      else {
        result += workload["workloadName"] + "," + workload["resultScore"] + "," + "Points\n";
      }

      //Capture Subtest score
      for (var benchmark in workload["tests"]) {

        result += workload["workloadName"] + ":";
        if (workload["tests"][benchmark]["errors"]) {
          result += workload["tests"][benchmark]["name"] + "," +
                    "Error," +
                    workload["tests"][benchmark]["scoreDescription"] + "\n";
          resultStatus = false;
        }
        else if (workload["tests"][benchmark]["warnings"]) {
          result += workload["tests"][benchmark]["name"] + "," +
                    "Warning," +
                    workload["tests"][benchmark]["scoreDescription"] + "\n";
          resultStatus = false;
        }
        else {
          result += workload["tests"][benchmark]["name"] + "," +
                    workload["tests"][benchmark]["resultScore"] + "," +
                    workload["tests"][benchmark]["scoreDescription"] +"\n";
        }
      }

    //Copy result.json to results folder
    moveFile(jsonFile,resultWorkloadPath + "/result.json");
    }
    else {
      result += benchmarkList[benchmarkIndex] + "," +
                "Aborted" + "," + "\n";
    }

    moveFile(workloadPath+"/stdout.log", resultWorkloadPath+"/stdout.log");
    moveFile(workloadPath+"/stderr.log",resultWorkloadPath+"/stderr.log");
  }

  catch (err) {
    Logger.error("Result generation failed: " + err);
  }

  fs.writeFileSync(resultPath + "Results.csv", result, {flag: "a"});

  return resultStatus;
}
