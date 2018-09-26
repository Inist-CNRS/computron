const addon = require('bindings')('addon');

const transformer = new addon.Transformer();
console.log(transformer);
transformer.loadStylesheet('dataset/otherxslt.xsl');
transformer.apply('dataset/draft.xml', result => {
  console.log(result);
});
