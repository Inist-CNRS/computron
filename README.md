[![Build and test status](https://github.com/Inist-CNRS/computron/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/Inist-CNRS/computron/actions/workflows/build-and-test.yml)
[![npm version](https://badge.fury.io/js/computron.svg)](https://badge.fury.io/js/computron)

# Computron
Computron is a Node.js library to apply XSLT stylesheets to XML documents. It's a [C++ addon for Node.js](https://nodejs.org/api/addons.html) that uses [libxml2](http://www.xmlsoft.org/) and [libxslt](http://xmlsoft.org/libxslt/).

## Disclaimer
**This library is only intended to be used on Linux.**

## Requirements
You must have libxml2 and libxslt1 installed on your system.
```
sudo apt install libxml2-dev libxslt1-dev
```

You should already have a C++ compiler installed on your system, if it's not the case install g++.
```
sudo apt install g++
```

Computron uses [node-gyp](https://github.com/nodejs/node-gyp) as a build system so you need to install it to be able to compile Computron
```
npm install -g node-gyp
```

## Usage
Basic example:
```JS
const Computron = require('computron');
const computron = new Computron();

computron.loadStylesheet('/path/to/stylesheet', (_err) => {
  if (_err) throw _err;

  // null is passed as second argument because the stylesheet doesn't take any parameters
  computron.apply('/path/to/xml', null, (err, result) => {
    if (err) throw err;

    console.log(result);
  });
});
```

Using a stylesheet that takes parameters:
```JS
const Computron = require('computron');
const computron = new Computron();

computron.loadStylesheet('/path/to/stylesheet-with-params', (_err) => {
  if (_err) throw _err;

  computron.apply('/path/to/xml', { param1Name: 'param1Value', param2Name: 'param2Value' }, (err, result) => {
    if (err) throw err;

    console.log(result);
  });
});
```

## Development
To build and run the tests in release mode run:
```
npm test
```

You can debug the C++ code with in VSCode, to do so run:
```
npm run build:config
```
This will build the VSCode debugger configuration, you can then simply press `F5` or go to the "Run and Debug" tab and click on "Debug".
