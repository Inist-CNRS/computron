#include "transformer.h"
#include <cstring>
#include <functional>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>
#include <string>
#include <vector>

using namespace std;

// Disable warning and error parsing
void xslt_generic_error_handler(void *ctx, const char *msg, ...) {
  // Do nothing but do it well
}

Napi::FunctionReference Transformer::constructor;

Napi::Object Transformer::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Transformer",
                                    {InstanceMethod("loadStylesheet", &Transformer::loadStylesheet),
                                     InstanceMethod("apply", &Transformer::apply)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Transformer", func);
  return exports;
}

Transformer::Transformer(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Transformer>(info) {
  exsltRegisterAll();
  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  xsltSetGenericErrorFunc(nullptr, xslt_generic_error_handler);
  stylesheetPtr = nullptr;
}

Transformer::~Transformer() {
  if (stylesheetPtr) xsltFreeStylesheet(stylesheetPtr);
  xsltCleanupGlobals();
  xmlCleanupParser();
  xmlMemoryDump();
}

class LoadStylesheetAsync : public Napi::AsyncWorker {
public:
  LoadStylesheetAsync(Napi::Function &callback, string stylesheetPath, xsltStylesheetPtr *stylesheetPtr)
      : Napi::AsyncWorker(callback), stylesheetPath(stylesheetPath), stylesheetPtr(stylesheetPtr) {}
  ~LoadStylesheetAsync() {}

  void Execute() {
    if (stylesheetPtr != nullptr) xsltFreeStylesheet(*stylesheetPtr);
    *stylesheetPtr = xsltParseStylesheetFile((const xmlChar *)stylesheetPath.c_str());
    if (*stylesheetPtr == nullptr) {
      string message = "failed to parse " + (string)stylesheetPath;
      return Napi::AsyncWorker::SetError(message);
    }
  }

  void OnOK() {
    Napi::HandleScope scope(Env());
    Callback().Call({});
  }

  void OnError(const Napi::Error &error) {
    Napi::HandleScope scope(Env());
    Callback().Call({error.Value()});
  }

private:
  string stylesheetPath;
  xsltStylesheetPtr *stylesheetPtr;
};

Napi::Value Transformer::loadStylesheet(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();

  if (length <= 0 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  if (length <= 0 || !info[1].IsFunction()) {
    Napi::TypeError::New(env, "callback function expected").ThrowAsJavaScriptException();
  }

  Napi::String value = info[0].As<Napi::String>();
  string stylesheetPath = value.Utf8Value();
  Napi::Function callback = info[1].As<Napi::Function>();

  LoadStylesheetAsync *loadStylesheetAsync =
      new LoadStylesheetAsync(callback, stylesheetPath, &stylesheetPtr);
  loadStylesheetAsync->Queue();
  return info.Env().Undefined();
}

class ApplyAsync : public Napi::AsyncWorker {
public:
  ApplyAsync(Napi::Function &callback, string xmlDocumentPath, xsltStylesheetPtr stylesheetPtr,
             vector<const char *> params)
      : Napi::AsyncWorker(callback), xmlDocumentPath(xmlDocumentPath), stylesheetPtr(stylesheetPtr),
        params(params) {}
  ~ApplyAsync() {}

  void Execute() {
    if (stylesheetPtr == nullptr) {
      return Napi::AsyncWorker::SetError("no stylesheet loaded");
    }

    xmlDocPtr inputXmlDocument = xmlReadFile(xmlDocumentPath.c_str(), NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (inputXmlDocument == NULL) {
      string message = "failed to parse " + (string)xmlDocumentPath;
      return Napi::AsyncWorker::SetError(message);
    }

    xmlDocPtr outputXmlDocument = xsltApplyStylesheet(stylesheetPtr, inputXmlDocument, params.data());
    if (outputXmlDocument == NULL) {
      xmlFreeDoc(inputXmlDocument);
      xmlCleanupParser();
      xmlMemoryDump();
      string message = "failed to transform " + (string)xmlDocumentPath;
      return Napi::AsyncWorker::SetError(message);
    }

    xmlChar *stringResult;
    int stringResultLength;
    xsltSaveResultToString(&stringResult, &stringResultLength, outputXmlDocument, stylesheetPtr);
    result = (char *)stringResult;
    xmlFreeDoc(inputXmlDocument);
    xmlFreeDoc(outputXmlDocument);
    xmlCleanupParser();
    xmlMemoryDump();
  }

  void OnOK() {
    Napi::HandleScope scope(Env());
    Callback().Call({Env().Undefined(), Napi::String::New(Env(), result)});
  }

  void OnError(const Napi::Error &error) {
    Napi::HandleScope scope(Env());
    Callback().Call({error.Value()});
  }

private:
  string xmlDocumentPath;
  xsltStylesheetPtr stylesheetPtr;
  string result;
  vector<const char *> params;
};

Napi::Value Transformer::apply(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  vector<string> params{};
  vector<const char *> cParams{};

  int length = info.Length();

  if (length <= 0) {
    Napi::TypeError::New(env, "Arguments expected").ThrowAsJavaScriptException();
  }

  if (!info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  if (!info[length - 1].IsFunction()) {
    Napi::TypeError::New(env, "Callback function expected").ThrowAsJavaScriptException();
  }

  Napi::String value = info[0].As<Napi::String>();
  string xmlDocumentPath = value.Utf8Value();
  Napi::Function callback = info[length - 1].As<Napi::Function>();

  if (length > 2) {
    if (!info[1].IsObject()) {
      Napi::TypeError::New(env, "Object expected").ThrowAsJavaScriptException();
    }
    Napi::Object paramObj = info[1].As<Napi::Object>();
    Napi::Array keys = paramObj.GetPropertyNames();
    uint32_t length = keys.Length();

    for (uint32_t i = 0; i < length; i++) {
      const string key = keys.Get(i).As<Napi::String>().Utf8Value();
      params.push_back(key);
      string value = paramObj.Get(key).As<Napi::String>().Utf8Value();
      value = "\"" + value + "\"";
      params.push_back(value);
    }

    cParams.reserve(params.size());
    transform(params.begin(), params.end(), back_inserter(cParams), [](auto& param) { 
      return param.c_str(); 
    });
    cParams.push_back(NULL);
  }

  ApplyAsync *applyAsync = new ApplyAsync(callback, xmlDocumentPath, stylesheetPtr, cParams);
  applyAsync->Queue();
  return info.Env().Undefined();
}
