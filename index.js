const ironhide = require('bindings')('addon');

const doc = new ironhide.Document(10);
console.log(doc.value);
