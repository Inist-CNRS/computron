# Computron
Computron is a Node.js library to apply XSLT stylesheets to XML documents. It's a [C++ addon for Node.js](https://nodejs.org/api/addons.html) that uses [libxml2](http://www.xmlsoft.org/) and [libxslt](http://xmlsoft.org/libxslt/).

## Disclaimer
**This library is only intended to be used on Linux.**

## Requirements
You must have libxml2 and libxslt1 installed on your system.
```bash
sudo apt install libxml2-dev libxslt1-dev
```

You should already have a C++ compiler installed on your system, if it's not the case install g++.
```bash
sudo apt install g++
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
```bash
npm test
```

You can debug the C++ code with in VSCode, to do so run:
```bash
npm run build:config
```
to build the VSCode debugger configuration then simply press `F5` (Or go to the "Run and Debug" tab and click on "Debug").
