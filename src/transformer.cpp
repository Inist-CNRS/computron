#include "transformer.h"
#include <assert.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltutils.h>
#include <string>

Napi::FunctionReference Transformer::constructor;

Napi::Object Transformer::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "MyObject", {
    InstanceMethod("loadStylesheet", &Transformer::loadStylesheet),
    InstanceMethod("apply", &Transformer::apply)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Transformer", func);
  return exports;
}

Transformer::Transformer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Transformer>(info) {
  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  stylesheetPtr = nullptr;
}

Transformer::~Transformer() {
  if (stylesheetPtr) xsltFreeStylesheet(stylesheetPtr);
  xsltCleanupGlobals();
  xmlCleanupParser();
  xmlMemoryDump();
}


Napi::Value Transformer::loadStylesheet(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();

  if (length <= 0 || !info[0].IsString()) {
    Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
  }

  Napi::String value = info[0].As<Napi::String>();
  std::string stylesheetPath = value.Utf8Value();
  if (stylesheetPtr != nullptr) xsltFreeStylesheet(stylesheetPtr);
  stylesheetPtr = xsltParseStylesheetFile((const xmlChar *)stylesheetPath.c_str());
}

Napi::Value Transformer::apply(const Napi::CallbackInfo& info) {
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
  std::string xmlDocumentPath = value.Utf8Value();
  Napi::Function cb = info[1].As<Napi::Function>();

  xmlDocPtr inputXmlDocument = xmlReadFile(xmlDocumentPath.c_str(), NULL, 0);
  if (inputXmlDocument == NULL) {
    std::string message = "failed to parse " + (std::string)xmlDocumentPath;
    Napi::TypeError::New(env, message).ThrowAsJavaScriptException();
  }

  xmlDocPtr outputXmlDocument = xsltApplyStylesheet(stylesheetPtr, inputXmlDocument, params);
  if (outputXmlDocument == NULL) {
    std::string message = "failed to transform " + (std::string)xmlDocumentPath;
    Napi::TypeError::New(env, message).ThrowAsJavaScriptException();
  }

  xmlChar *stringResult;
  int stringResultLength;
  xsltSaveResultToString(&stringResult, &stringResultLength, outputXmlDocument, stylesheetPtr);

  cb.Call(env.Global(), { Napi::String::New(env, (char *)stringResult) });

  xmlFreeDoc(inputXmlDocument);
  xmlFreeDoc(outputXmlDocument);
  xmlCleanupParser();
  xmlMemoryDump();
}
