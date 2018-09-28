[![Build Status](https://travis-ci.org/Inist-CNRS/computron.svg?branch=master)](https://travis-ci.org/Inist-CNRS/computron)

# Computron

:warning: Still working in progress :warning:

Computron is a Node.js library for applying XSLT stylesheets to XML documents. It's also an [C++ addon for nodejs](https://nodejs.org/api/n-api.html) which means it use pure C++ code whith the help of libxslt library.

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
const transformer = new Transformer();

transformer.loadStylesheet('path/to/my/file.xsl', error => {
  if (error) return console.log('dammit');
  
  transformer.apply('path/to/my/file.xml', (error, result) => {
    if (error) return console.log('bloody hell');
    // do something with the result
  });
});
```
