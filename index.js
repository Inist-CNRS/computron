const addon = require('bindings')('addon');
const transformer = new addon.Transformer();
console.log(transformer);
transformer.loadStylesheet('dataset/otherxslt.xsl');
transformer.apply('dataset/draft.xml', (error, result) => {
  if (error) return console.error('error', error);
  console.log('result : ', result);
});

console.log('dude ?');
