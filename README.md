[![Build Status](https://travis-ci.org/Inist-CNRS/computron.svg?branch=master)](https://travis-ci.org/Inist-CNRS/computron)

# Computron

:warning: Work in progress :warning:

Computron is a Node.js library for applying XSLT stylesheets to XML documents. It's also a [C++ addon for nodejs](https://nodejs.org/api/n-api.html) which means it uses pure C++ code whith the help of libxslt library.

## Requirements
This version has been tested in Ubuntu 18.04 with NodeJS v10.

```bash
sudo apt install cmake g++ libxml2-dev libxslt1-dev
```

## Install
```bash
npm install computron
```

## Usage 
```js
const Computron = require('computron');
const transformer = new Computron();

transformer.loadStylesheet('path/to/my/file.xsl', error => {
  if (error) return console.log('dammit');
  
  transformer.apply('path/to/my/file.xml', (error, result) => {
    if (error) return console.log('bloody hell');
    // do something with the result
  });
});
```

Promises with the help of [bluebird package](https://www.npmjs.com/package/bluebird)
```js
const Promise = require('bluebird');
const Computron = require('computron');
Promise.promisifyAll(Computron.prototype);
const transformer = new Computron();

transformer.loadStylesheetAsync('path/to/my/file.xsl')
  .then(() => transformer.applyAsync('path/to/my/file.xml'))
  .then(result => {
    // do something with the result
  })
  .catch(console.error)
});
```

## API

**Computron.loadStylesheet(string, callback)**
```js
Computron.loadStylesheet('path/to/my/file.xsl', error => {
  // ...
});
```

**Computron.apply(string, [obj], callback)**
```js
// With params
Computron.apply('path/to/my/file.xml', { name: 'John DOE' }, (error, result) => {
  // ...
});

// Without params
Computron.apply('path/to/my/file.xml', (error, result) => {
  // ...
});
```
