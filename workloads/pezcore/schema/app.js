/*
SPECworkstation
JSON schema validator for workload.json and query.json

*/

// validateJson.exe requirements:
const Ajv = require("ajv")
const ajv = new Ajv({allErrors:true}) // validating json with schema
const fs = require("fs"); // file handling
const { exit } = require("process");
const yargs = require('yargs'); // parsing parameters
const path = require('path');
// schema used https://json-schema.org/draft-04/schema#
workloadSchemaPath = path.join(__dirname, "reference_schema_files/" + "workload_schema.json");
querySchemaPath = path.join(__dirname, "reference_schema_files/" + "query_schema.json");

main();

/*
app.js - main()
The entry point of the program
*/

// checks if file exist
function checkFileExist(dataPath) {

    if (!fs.existsSync(dataPath)){
        console.log("File doesnt exist")
        exit();
    }

}

// get path of json type
function getDataPath(parameters)
{
    if (parameters["workload"]){

        return parameters["workload"]
    }
    else if (parameters["query"]) {

        return parameters["query"]
    }
    else {

        console.log("Invalid parameters provided \n### Help ### :\nvalidateJson.exe --workload workload.json \nvalidateJson.exe --query query.json");
        exit();
    }
}

// parse data
function getData(dataPath){
    var data;
    try {
        const jsonString = fs.readFileSync(dataPath)
        data = JSON.parse(jsonString);
        return data
    } catch(err) {
        console.log("Error while parsing data");
        console.log(err);
        exit();
    }
}

// get respective schema
function getSchema(parameters){
    if (parameters["workload"])
    {
        return workloadSchemaPath;
    }
    else if (parameters["query"])
    {
        return querySchemaPath;
    }
}

// main function
function main() {

    let parameters = yargs.argv;
    var dataPath = getDataPath(parameters);
    checkFileExist(dataPath);
    var dataToValidate = getData(dataPath);
    var validationSchema =  getData(getSchema(parameters));

    const validate = ajv.compile(validationSchema)
    const isValid = validate(dataToValidate)

    if (isValid) {
        console.log("Successful validation")
    }
    else {
        console.log("Failed validation, in following criterias");
        console.log(ajv.errorsText(validate.errors))
    }

}