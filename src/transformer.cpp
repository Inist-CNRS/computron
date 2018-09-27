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

  Napi::Function func = DefineClass(env, "Transformer",
                                    {InstanceMethod("loadStylesheet", &Transformer::loadStylesheet),
                                     InstanceMethod("apply", &Transformer::apply)});

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Transformer", func);
  return exports;
}

Transformer::Transformer(const Napi::CallbackInfo &info) : Napi::ObjectWrap<Transformer>(info) {
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

class LoadStylesheetAsync : public Napi::AsyncWorker {
 public:
  LoadStylesheetAsync(Napi::Function& callback, std::string stylesheetPath, xsltStylesheetPtr *stylesheetPtr)
    : Napi::AsyncWorker(callback), stylesheetPath(stylesheetPath), stylesheetPtr(stylesheetPtr) {}
  ~LoadStylesheetAsync() {}

  void Execute () {
    if (stylesheetPtr != nullptr) xsltFreeStylesheet(*stylesheetPtr);
    *stylesheetPtr = xsltParseStylesheetFile((const xmlChar *)stylesheetPath.c_str());
    if (*stylesheetPtr == nullptr) {
      std::string message = "failed to parse " + (std::string)stylesheetPath;
      return Napi::AsyncWorker::SetError(message);
    }
  }

  void OnOK() {
    Napi::HandleScope scope(Env());
    Callback().Call({});
  }

   void OnError(const Napi::Error& error) {
    Napi::HandleScope scope(Env());
    Callback().Call({error.Value()});
  }

 private:
  std::string stylesheetPath;
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
  std::string stylesheetPath = value.Utf8Value();
  Napi::Function callback = info[1].As<Napi::Function>();

  LoadStylesheetAsync *loadStylesheetAsync = new LoadStylesheetAsync(callback, stylesheetPath, &stylesheetPtr);
  loadStylesheetAsync->Queue();
  return info.Env().Undefined();
}

class ApplyAsync : public Napi::AsyncWorker {
public:
  ApplyAsync(Napi::Function &callback, std::string xmlDocumentPath, xsltStylesheetPtr stylesheetPtr,
             const char **params)
      : Napi::AsyncWorker(callback), xmlDocumentPath(xmlDocumentPath), stylesheetPtr(stylesheetPtr),
        params(params) {}
  ~ApplyAsync() {}

  void Execute() {
    if (stylesheetPtr == nullptr) {
      return Napi::AsyncWorker::SetError("no stylesheet loaded");
    }

    xmlDocPtr inputXmlDocument = xmlReadFile(xmlDocumentPath.c_str(), NULL, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (inputXmlDocument == NULL) {
      std::string message = "failed to parse " + (std::string)xmlDocumentPath;
      return Napi::AsyncWorker::SetError(message);
    }

    xmlDocPtr outputXmlDocument = xsltApplyStylesheet(stylesheetPtr, inputXmlDocument, params);
    if (outputXmlDocument == NULL) {
      xmlFreeDoc(inputXmlDocument);
      xmlCleanupParser();
      xmlMemoryDump();
      std::string message = "failed to transform " + (std::string)xmlDocumentPath;
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

  void OnError(const Napi::Error& error) {
    Napi::HandleScope scope(Env());
    Callback().Call({error.Value()});
  }

private:
  std::string xmlDocumentPath;
  xsltStylesheetPtr stylesheetPtr;
  std::string result;
  const char **params;
};

Napi::Value Transformer::apply(const Napi::CallbackInfo &info) {
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
  Napi::Function callback = info[1].As<Napi::Function>();

  ApplyAsync *applyAsync = new ApplyAsync(callback, xmlDocumentPath, stylesheetPtr, params);
  applyAsync->Queue();
  return info.Env().Undefined();
}
