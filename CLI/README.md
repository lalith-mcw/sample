
**Steps to create binary "SPECworkstation.exe":**
1. Install npm - follow the steps in https://phoenixnap.com/kb/install-node-js-npm-on-windows
2. Install pkg - run "npm install -g pkg" in cmd [https://github.com/vercel/pkg]
3. Run CreateBinary.bat in the root folder

**Library Used:**
1. Commander  // Helps in Command Line parsing https://github.com/tj/commander.js/

**Note :** 
1. Pezcore format workloads are provided with prebuilt binary[Release]
2. Current setup can be run with node binary alone(primary requirement) - 
      It can also run be with electron(if installed) but yet to add the electron dependencies in the package

**Instructions to Run :** 

       SPECworkstation.exe [options]

**Supported spec workloads :** 

      Poisson and Convolution

**Supported CmdLine options:** 

      --official | -o                 Run in Official mode. Default is false 
      --iterations | -i <iterations>  Number of iterations to complete of benchmark list. Default is 1
      --debug | -d                    Display Debug Output. Default is false
      --help | -h                     Display Command Usage
      --list | -l                     Display Benchmark List
      --version | -V                  Display version
      --workload | -w                 Specify workloads to run
      --fast-abort | -f               Abort pending tests on a test failure
      --path | -p <Folder>            Path to Workloads Directory. Default is 'workloads\'

 **Examples:**
 
      1. SPECworkstation.exe --help  // To display Usage
      2. SPECworkstation.exe -o      // official run - runs the available workloads
      3. SPECworkstation.exe -w poisson Convolution // Runs the specified workloads alone
      4. SPECworkstation.exe -i 2 -w poisson // Specify iterations
      5. SPECworkstation.exe -l -p ..\Documents\workloads\ // list all workloads available in the given path

