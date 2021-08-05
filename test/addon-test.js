'use strict';
/* eslint-env mocha */
/* eslint-disable no-unused-expressions */
const pkg = require('../package.json');
const Transformer = require('../addon.js');
const expect = require('chai').expect;
const path = require('path');
const xpath = require('xpath');
const { DOMParser } = require('xmldom');

describe(pkg.name + '/addon.js', function () {
  describe('loadStylesheet', function () {
    it('should load stylesheet', function (done) {
      const transformer = new Transformer();
      expect(transformer).to.be.an.instanceof(Transformer);
      const stylesheetPath = path.join(__dirname, 'data/stylesheet.xsl');
      transformer.loadStylesheet(stylesheetPath, error => {
        if (error) return done(error);
        done();
      });
    });

    it('should reload another stylesheet', function (done) {
      const transformer = new Transformer();
      expect(transformer).to.be.an.instanceof(Transformer);
      const stylesheetPath = path.join(__dirname, 'data/stylesheet.xsl');
      const anotherStylesheetPath = path.join(__dirname, 'data/another-stylesheet.xsl');
      transformer.loadStylesheet(stylesheetPath, error => {
        if (error) return done(error);
        transformer.loadStylesheet(anotherStylesheetPath, error => {
          if (error) return done(error);
          done();
        });
      });
    });

    it('should fail to load stylesheet', function (done) {
      const transformer = new Transformer();
      expect(transformer).to.be.an.instanceof(Transformer);
      const stylesheetPath = path.join(__dirname, 'data/nope.xsl');
      transformer.loadStylesheet(stylesheetPath, error => {
        expect(error).to.be.an('error');
        done();
      });
    });
  });

  describe('apply', function () {
    it('should apply stylesheet for xml file', function (done) {
      const transformer = new Transformer();
      expect(transformer).to.be.an.instanceof(Transformer);
      const stylesheetPath = path.join(__dirname, 'data/stylesheet.xsl');
      const xmlDocument = path.join(__dirname, 'data/example.xml');
      transformer.loadStylesheet(stylesheetPath, error => {
        if (error) return done(error);
        transformer.apply(xmlDocument, (error, result) => {
          if (error) return done(error);
          expect(result).to.be.a('string');
          done();
        });
      });
    });

    it('should apply stylesheet whith param for xml file', function (done) {
      const transformer = new Transformer();
      expect(transformer).to.be.an.instanceof(Transformer);
      const stylesheetPath = path.join(__dirname, 'data/stylesheet.xsl');
      const xmlDocument = path.join(__dirname, 'data/example.xml');
      const param = {
        givenName: 'Bob',
        familyName: 'Gedolf'
      };
      transformer.loadStylesheet(stylesheetPath, error => {
        if (error) return done(error);
        transformer.apply(xmlDocument, param, (error, result) => {
          if (error) return done(error);
          expect(result).to.be.a('string');
          const doc = new DOMParser().parseFromString(result);
          const nodes = xpath.select('/html/body/h2', doc);
          expect(nodes.toString()).to.equal('<h2>My CD Collection by BobGedolf</h2>');
          done();
        });
      });
    });

    it('should fail to apply stylesheet', function (done) {
      const transformer = new Transformer();
      expect(transformer).to.be.an.instanceof(Transformer);
      const stylesheetPath = path.join(__dirname, 'data/stylesheet.xsl');
      const xmlDocument = path.join(__dirname, 'data/nope.xml');
      transformer.loadStylesheet(stylesheetPath, error => {
        if (error) return done(error);
        transformer.apply(xmlDocument, error => {
          expect(error).to.be.an('error');
          done();
        });
      });
    });
  });
});
