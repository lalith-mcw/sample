### Schema Validation 

Node js script used to validate workload.json and query.json with workload_schema.json and query_schema.json files

### Pre requesties
- Node installations

- Node modules required
    - Ajv
    - pkg
    - Yargs

#### Installation

- npm install package.json

To build .exe package 

- npm run make_exe

Above command will run "pkg app.js --config package.json -o jsonValidate" internally


### Usage

jsonValidate.exe --workload workload.json

jsonValidate.exe --query query.json



