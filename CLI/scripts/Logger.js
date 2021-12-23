//Logger.js

var exports = module.exports = {};
const moment = require("moment.min.js");
const fs = require('fs');

exports.debug = function(message) {
  if (global.debugEnabled) {
    console.log("DEBUG:\t" + message.replace(/(?:\r\n|\r|\n)/g, '\n\t'));
  }
}

exports.error = function(message) {
  console.error("Error: " + message);
  process.exit();
}

exports.logStamp = function(message, resultPath, timestamp = false) {
  if (timestamp == true) {
    var timestamp = moment().format("Y-MM-DDTHH:mm:ss");
    message = "[" + timestamp + "] "+ message;
  }
  verboseLog(message, resultPath);
  console.log(message);
}

exports.log = function(message) {
  console.log(message);
}

function verboseLog(message, resultPath,) {
  fs.writeFileSync(resultPath + "run-log.txt", message + "\n", {flag: "a"});
}
