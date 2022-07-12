const assert = require('assert');
const path = require('path');

// Use the debug build of Computron while using the debugger and the release build otherwise.
// The DEBUG environment variable is set in the debugger config (cf. .vscode/launch.json)
const { Computron } = require(`../build/${process.env.DEBUG ? 'Debug' : 'Release'}/computron-native`);

assert(Computron, 'The expected class is undefined');

const datasetPath = path.join(__dirname, 'dataset');
const computron = new Computron('default');

function wrongTypeInPathWhenLoadingStylesheet () {
  computron.loadStylesheet(3);
}

function loadCorrectStylesheet () {
  computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'));
}

function loadBadStylesheet () {
  computron.loadStylesheet(path.join(datasetPath, 'error.xsl'));
}

function loadInexistentStylesheet () {
  computron.loadStylesheet(path.join(datasetPath, 'inexistent.xsl'));
}

function wrongTypeInPathWhenApplying () {
  computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'));
  computron.apply(3);
}

function applyWithoutLoading () {
  const otherComputron = new Computron('custom');
  otherComputron.apply(path.join(datasetPath, 'correct.xml'));
}

function applyCorrectStylesheetToCorrectXml () {
  computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'));
  const result = computron.apply(path.join(datasetPath, 'correct.xml'));

  assert.strictEqual(result.length > 0, true, 'applyCorrectStylesheetToCorrectXml returned an empty result');
}

function applyCorrectStylesheetToBadXml () {
  computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'));
  computron.apply(path.join(datasetPath, 'error.xml'));
}

function applyStylesheetWithParams () {
  computron.loadStylesheet(path.join(datasetPath, 'correct-params.xsl'));
  const result = computron.apply(path.join(datasetPath, 'correct.xml'), { givenName: 'Bob', familyName: 'Gedolf' });

  assert.strictEqual(result.length > 0, true, 'applyStylesheetWithParams returned an empty result');
}

// This test makes sure applying a stylesheet a second time can still work even after getting an error the first time
function applyStylesheetAfterError () {
  computron.loadStylesheet(path.join(datasetPath, 'correct.xsl'));

  try {
    computron.apply(path.join(datasetPath, 'error.xml'));
  } catch (err) {
    assert.strictEqual(err instanceof Error, true, 'Applying the stylesheet to the first XML document didn\'t return an error in applyStylesheetAfterError');
  }

  const result = computron.apply(path.join(datasetPath, 'correct.xml'));

  assert.strictEqual(result.length > 0, true, 'applyStylesheetAfterError returned an empty result');
}

function applySameStylesheetWithDifferentParams () {
  computron.loadStylesheet(path.join(datasetPath, 'correct-params.xsl'));
  const xmlWithoutParams = computron.apply(path.join(datasetPath, 'correct.xml'));
  const xmlWithParams = computron.apply(path.join(datasetPath, 'correct.xml'), { givenName: 'Bob', familyName: 'Gedolf' });

  assert.strictEqual(xmlWithoutParams.length > 0, true, 'Applying the stylesheet without params returned an empty result');
  assert.strictEqual(xmlWithParams.length > 0, true, 'Applying the stylesheet with params returned an empty result');
  assert.strictEqual(xmlWithoutParams !== xmlWithParams, true, 'Applying the stylesheet with and without params returned the same result');
}

assert.throws(wrongTypeInPathWhenLoadingStylesheet, { message: 'String expected as first argument' });
assert.doesNotThrow(loadCorrectStylesheet);
assert.throws(loadBadStylesheet);
assert.throws(loadInexistentStylesheet);
assert.throws(wrongTypeInPathWhenApplying, { message: 'String expected as first argument' });
assert.throws(applyWithoutLoading, { message: 'You need to load a stylesheet first' });
assert.doesNotThrow(applyCorrectStylesheetToCorrectXml);
assert.throws(applyCorrectStylesheetToBadXml);
assert.doesNotThrow(applyStylesheetWithParams);
assert.doesNotThrow(applyStylesheetAfterError);
assert.doesNotThrow(applySameStylesheetWithDifferentParams);

console.info('\nTests passed - everything looks OK!');
