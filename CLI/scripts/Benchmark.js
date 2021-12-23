//Benchmark.js 

const Helper = require("./Helper.js");
const Logger = require("./Logger.js");
const Result = require("./Result.js");
const Version = require ("./Version.js");
const spawn = require('child_process').spawn;
const moment = require("moment.min.js");
const homedir = require('os').homedir();
const fs = require('fs');

global.currentChild;
var resultPath = "";
var resultFolderList = new Array();
global.workload;

function runBenchmark(workloadsList, workloadToRun, currentIteration, iterationsToComplete) {

  workload = workloadsList[workloadToRun];

  if (workloadToRun == 0) {
    // Create and Update Results directory for every Iteration
    var timestamp = moment().format("Y-MM-DD-HH-mm-ss");
    resultPath = homedir + "\\Documents\\" + Version.getName() + " " + Version.getVersion() +  "\\result-"+timestamp;

    if (fs.existsSync(resultPath)) {
      for (var nameExtension = 1; nameExtension < 20; ++nameExtension) {
        if (!(fs.existsSync(resultPath + "_" + nameExtension ))) {
          resultPath += "_" + nameExtension;
          break;
        }
      }
    }

    resultPath += "\\";
    fs.mkdirSync(resultPath, {recursive: true});
    resultFolderList.push(resultPath);
  }

  Logger.logStamp("Starting Workload " + (workloadToRun+1) + " of " + (workloadsList.length)
                  + ", Iteration "+ (currentIteration+1) + " of "+ (iterationsToComplete), resultPath, true);

  //Check if workload is a validTest
  if (!global.officialRun) {
    if ((index = validTests.indexOf(workload)) < 0) {
      Logger.logStamp("[workload:"+workload+"] Invalid Workload -- Skipped", resultPath, true);

      // Proceed to next workload/Iteration
      workloadToRun++;
      if (workloadToRun == workloadsList.length) {
        finishIteration(workloadsList, currentIteration, iterationsToComplete);
      }
      else {
        runBenchmark(workloadsList, workloadToRun, currentIteration, iterationsToComplete);
      }
      return;
    }
  }

  if (fs.existsSync(workloadPath + workload)) {

    // Skip if workload belongs to superset of Available workloads
    if (global.officialList.hasOwnProperty(workload) &&
         (global.officialList[workload].hash !== global.availableList[workload].hash)) {
      Logger.logStamp("[workload:"+workload+"] Warning: workload.json Hash Mismatch", resultPath, true);
    }

    //Run the Workload binary
    global.currentChild = spawn(workloadPath + "pezcore.exe -tests " + workloadPath + workload + "\\workload.json",[], { shell: true});
    Logger.logStamp("[workload:"+workload+"] started", resultPath, true);

    // Check for startup error
    global.currentChild.on('error', function(err) {
      Logger.log("Unable to start. Error: "+err);
    });

    process.on('SIGINT', function (code) {
      killWorkload();
      Logger.logStamp("[workload:" + workload + "] Aborted", resultPath, true);
      let status = Result.manageResults(workloadsList, workloadToRun, currentIteration, resultPath);
      Logger.logStamp("\n Received 'SIGINT' signal --  Execution Terminated", resultPath);
      Helper.end(resultFolderList);
    });

    global.currentChild.on('exit', function(code) {
      global.currentChild = "";
      //Non normal exit, Something went wrong
      if (code !== 0) {
        Logger.log("\tfailed! Exit Code Was: "+code);
        return;
      }
      //Normal exit
      else {
        var status = Result.manageResults(workloadsList, workloadToRun, currentIteration, resultPath);
        module.exports.validateResults(status, workload);
      }
      // Proceed to next workload/Iteration
      workloadToRun++;
      if (workloadToRun == workloadsList.length) {
        finishIteration(workloadsList, currentIteration, iterationsToComplete);
      }
      else {
        runBenchmark(workloadsList, workloadToRun, currentIteration, iterationsToComplete);
      }
    });
  }
}

function finishIteration(workloadsList, currentIteration, iterationsToComplete) {

  currentIteration++;

  if (currentIteration < iterationsToComplete) {
    var firstBenchmark = 0;
    runBenchmark(workloadsList, firstBenchmark, currentIteration, iterationsToComplete);
  }
  // We are finished with the benchmark suite
  else {
    Helper.end(resultFolderList);
  }
}

exports.runBenchmarkSuite = function(workloadList, iterationsToComplete) {

  var firstBenchmark = 0;
  var firstIteration = 0;

  runBenchmark(workloadList, firstBenchmark, firstIteration, iterationsToComplete);
}

//Use Tree-Kill module to kill the currently running benchmark if there is one.
function killWorkload() {
  if (global.currentChild) {
    Logger.debug("Trying to kill child proccess: " + global.currentChild.pid);
    var kill = require("tree-kill");
    try {
      kill(global.currentChild.pid,(error) => {
           Logger.error(error); // an AbortError
        });
    } catch (e) {
      Logger.error("Unable to kill child proccess tree. ERROR:" + e);
    }
  }
}

exports.validateResults = function(status, workload) {
  if (status == false)
  {
    Logger.logStamp("[workload:" + workload + "] INVALID RESULT -- see log for errors/warnings", resultPath, true);
    if (global.fastAbort) {
      Logger.log("\nAborting remaining tests (user selected --fast-abort option)");
      Helper.End(resultFolderList);
    }
  }
  else {
    Logger.logStamp("[workload:" + workload + "] completed", resultPath, true);
  }
}
