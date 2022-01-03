const assert = require('assert');
const path = require('path');

// Use the debug build of Computron while using the debugger and the release build otherwise.
// The DEBUG environment variable is set in the debugger config (cf. .vscode/launch.json)
const { Computron } = require(`../build/${process.env.DEBUG ? 'Debug' : 'Release'}/computron-native`);

assert(Computron, 'The expected class is undefined');

const datasetPath = path.join(__dirname, 'dataset');
const computron = new Computron();

function loadCorrectStylesheet () {
  return new Promise((resolve, reject) => {
    computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'), (err, result) => {
      assert.strictEqual(err, undefined, 'loadCorrectStylesheet returned an error');
      assert.strictEqual(result, undefined, 'loadCorrectStylesheet returned a result');

      err ? reject(err) : resolve(result);
    });
  });
}

function loadBadStylesheet () {
  return new Promise((resolve, reject) => {
    computron.loadStylesheet(path.join(datasetPath, 'error.xsl'), (err, result) => {
      assert.strictEqual(err instanceof Error, true, 'loadBadStylesheet didn\'t return an error');
      assert.strictEqual(result, undefined, 'loadBadStylesheet returned a result');

      err ? reject(err) : resolve(result);
    });
  });
}

function loadInexistentStylesheet () {
  return new Promise((resolve, reject) => {
    computron.loadStylesheet(path.join(datasetPath, 'inexistent.xsl'), (err, result) => {
      assert.strictEqual(err instanceof Error, true, 'loadInexistentStylesheet didn\'t return an error');
      assert.strictEqual(result, undefined, 'loadInexistentStylesheet returned a result');

      err ? reject(err) : resolve(result);
    });
  });
}

function applyCorrectStylesheetToCorrectXml () {
  return new Promise((resolve, reject) => {
    computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'), () => {
      computron.apply(path.join(datasetPath, 'correct.xml'), null, (err, result) => {
        assert.strictEqual(err, undefined, 'applyCorrectStylesheetToCorrectXml returned an error');
        assert.strictEqual(result.length > 0, true, 'applyCorrectStylesheetToCorrectXml returned an empty result');

        err ? reject(err) : resolve(result);
      });
    });
  });
}

function applyCorrectStylesheetToBadXml () {
  return new Promise((resolve, reject) => {
    computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'), () => {
      computron.apply(path.join(datasetPath, 'error.xml'), null, (err, result) => {
        assert.strictEqual(err instanceof Error, true, 'applyCorrectStylesheetToBadXml didn\'t return an error');
        assert.strictEqual(result, undefined, 'applyCorrectStylesheetToBadXml returned a result');

        err ? reject(err) : resolve(result);
      });
    });
  });
}

function applyStylesheetWithParams () {
  return new Promise((resolve, reject) => {
    computron.loadStylesheet(path.join(datasetPath, 'correct-params.xsl'), (_err, _result) => {
      if (_err) reject(_err);

      computron.apply(path.join(datasetPath, 'correct.xml'), { givenName: 'Bob', familyName: 'Gedolf' }, (err, result) => {
        assert.strictEqual(err, undefined, 'applyStylesheetWithParams returned an error');
        assert.strictEqual(result.length > 0, true, 'applyStylesheetWithParams returned an empty result');

        err ? reject(err) : resolve(result);
      });
    });
  });
}

(async function () {
  await assert.doesNotReject(loadCorrectStylesheet);
  await assert.rejects(loadBadStylesheet);
  await assert.rejects(loadInexistentStylesheet);

  await assert.doesNotReject(applyCorrectStylesheetToCorrectXml);
  await assert.rejects(applyCorrectStylesheetToBadXml);
  await assert.doesNotReject(applyStylesheetWithParams);

  console.info('\nTests passed - everything looks OK!');
})();
