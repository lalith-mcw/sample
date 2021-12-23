//version.js
var exports = module.exports = {};

const name = "SPECworkstation 2022";
const version = "1.0-alpha";


exports.getName = function(message) {
    return name;
}

exports.getVersion = function(message) {
   return version;
}
