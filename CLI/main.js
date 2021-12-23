//main.js

const Helper = require("./scripts/Helper.js");
const Benchmark = require("./scripts/Benchmark.js");
const Version = require("./scripts/Version.js");
const Logger = require("./scripts/Logger.js");
const JSONHandler = require("./scripts/OfficialJson.js");
const Validate = require("./scripts/ValidTests.js");
const spawn = require('child_process').spawn;
const moment = require("moment.min.js");
const fs = require('fs'); 

const { Command, Option } = require('commander');
const program  = new Command();

global.officialWorkloads = [];
global.availableList = {};
global.workloadPath;
global.debugEnabled = false;
global.officialRun = false;
global.fastAbort = false;
global.validTests = [];
global.invalidTests = {};

program
  .version(Version.getName() + " " + Version.getVersion())
  .name('SPECworkstation.exe')
  .usage(' [-o | -w workloads] [-i number] [-f] [-d]')
  .option('-o, --official',  'Run all official workloads')
  .option('-r, --run',  'Run all detected workloads without check')
  .option('-i, --iterations <n>',  'Number of loops of complete workload list')
  .option('-d, --debug', 'Display Debug Output')
  .option('-l, --list', 'Display Benchmark List')
  .option('-w, --workload [workloads...]', 'Workloads to run, comma or space seperated list' )
  .option('-f, --fast-abort', 'Abort pending tests on a test failure' )
  .addOption(new Option('-p, --path [workloadsPath]', 'Path to Workloads Directory')
               .default('workloads/','folder to workloads'))
  .addOption(new Option('-g, --generate-official').hideHelp());

start();

function start() {

  var args = process.argv.slice(2);
  var iterations = 1;
  var workloads = new Array();

  program.parse(process.argv);
  const options = program.opts();

  Logger.log("\n"+program.version()+"\n");

  if (options.path) {
    workloadPath = options.path;
    if ((workloadPath.substr(-1) != '\\') && (workloadPath.substr(-1) != '/')) {
      workloadPath +='\\';
    }

    //Replace '/' with '\' in absolute and relative paths if exists
    workloadPath = workloadPath.replace(new RegExp("/",'g'),'\\');

    if (!fs.existsSync(options.path)) {
      Logger.error(workloadPath + " path doesn't exist - pass workload directory path via --path parameter");
    }

    if (!(fs.lstatSync(workloadPath).isDirectory())) {
      Logger.error("Workload path must be a Directory (Eg; workloads\\)");
    }
  }

  if (options.generateOfficial) {
    JSONHandler.createOfficialJSON();
    process.exit();
  }

  global.officialList = Helper.getOfficialWorkloads();  //Workload list from OfficialRun.json - Used map for name and version

  //Display help if missing official or workload option
  if (!(options.official || options.workload || options.run)) {
    Helper.printWorkloadList();
    program.help();
  }

  if (options.official) {

    availableList = Helper.getAvailableWorkloads();

    // Compare offical and available workloads
    if (Helper.compare()) {
      global.officialWorkloads = Object.keys(availableList);
      workloads = officialWorkloads;
      global.officialRun = true;
    }
  }

  if (options.run) {
    availableList = Helper.getAvailableWorkloads();
    workloads = Object.keys(availableList);
  }

  if (options.list) {
    Helper.printWorkloadList();
    process.exit();
  }
  if (options.iterations)  iterations = options.iterations;
  if (options.workload)  workloads = options.workload;
  if (options.fastAbort) global.fastAbort = true;

  if (options.debug) {
    global.debugEnabled = true;
    Logger.log(options);
  }

  if (global.officialRun && options.workload) {
    Logger.error("Official run(-o) and Workload list(-w) options cannot be used at the same time");
  }

  //Exit if user provided an empty list
  if (typeof workloads.length == 'undefined') {
    Logger.error("Empty workload list provided");
  }

  //Handle user provided workload list (skipped if official run was selected)
  if (!global.officialRun && options.workload)
  {
    availableList = Helper.getAvailableWorkloads();
    var availableWorkloads = Object.keys(availableList);   //Checks the workload directory

    //Remove Duplicate workloads in the list
    //TODO: Create warning/error message if workload is added more than once
    const RemoveDuplicates = arry => arry.filter((item, index) => arry.indexOf(item) == index)
    workloads = RemoveDuplicates(workloads);

    Logger.log("Selected workloads:");
    //Parse workloads list
    for (var i=0; i < workloads.length; i++) {
      if ((index = availableWorkloads.findIndex(item => workloads[i].toLowerCase() === item.toLowerCase())) < 0) {
        Logger.log(workloads[i] + " is not an available workload");
        process.exit();
      }

      if (global.officialList[workloads[i]].hash !== global.availableList[workloads[i]].hash) {
        Logger.log( workloads[i] + "  (Warning! workload.json Hash Mismatch)");
      }
      else {
        Logger.log(workloads[i]);
      }
    }
    Logger.log("");
  }

  // Generate validTests.json by querying workloads
  Validate.generateValidTestsJson(workloads)

  //Print valid and invalid tests
  Validate.checkTests(workloads);

  //Compare official workloads and valid tests
  if (global.officialRun) {
    for (var workload in officialList) {
      if ((index = validTests.indexOf(officialList[workload].name)) < 0) {
        Logger.error("all official workloads must be present and valid for a valid official run [Invalid workload: "
                      + officialList[workload].name + "]");
      }
    }
  }

  //Run SPECworkstation
  Benchmark.runBenchmarkSuite(workloads, iterations);
}
