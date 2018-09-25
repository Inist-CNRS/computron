#include "transformer.h"
#include <assert.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltutils.h>
#include <string>

napi_ref Transformer::constructor;

Transformer::Transformer() {
  xmlSubstituteEntitiesDefault(1);
  xmlLoadExtDtdDefaultValue = 1;
  stylesheetPtr = nullptr;
}

Transformer::~Transformer() {
  if (stylesheetPtr) xsltFreeStylesheet(stylesheetPtr);
  xsltCleanupGlobals();
  xmlCleanupParser();
  xmlMemoryDump();
  napi_delete_reference(env_, wrapper_);
}

void Transformer::Destructor(napi_env env, void *nativeObject, void *) {
  reinterpret_cast<Transformer *>(nativeObject)->~Transformer();
}

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }

napi_value Transformer::Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_property_descriptor properties[] = {
      DECLARE_NAPI_METHOD("loadStylesheet", loadStylesheet),
      DECLARE_NAPI_METHOD("apply", apply),
  };

  napi_value cons;
  status = napi_define_class(env, "Transformer", NAPI_AUTO_LENGTH, New, nullptr, 2, properties, &cons);
  assert(status == napi_ok);

  status = napi_create_reference(env, cons, 1, &constructor);
  assert(status == napi_ok);

  status = napi_set_named_property(env, exports, "Transformer", cons);
  assert(status == napi_ok);
  return exports;
}

napi_value Transformer::New(napi_env env, napi_callback_info info) {
  napi_status status;

  napi_value target;
  status = napi_get_new_target(env, info, &target);
  assert(status == napi_ok);
  bool is_constructor = target != nullptr;

  if (is_constructor) {
    // Invoked as constructor: `new Transformer(...)`
    size_t argc = 1;
    napi_value args[1];
    napi_value jsthis;
    status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    assert(status == napi_ok);

    Transformer *obj = new Transformer();

    obj->env_ = env;
    status = napi_wrap(env, jsthis, reinterpret_cast<void *>(obj),
                       Transformer::Destructor,
                       nullptr,
                       &obj->wrapper_);
    assert(status == napi_ok);

    return jsthis;
  } else {
    // Invoked as plain function `Transformer(...)`, turn into construct call.
    napi_value cons;
    status = napi_get_reference_value(env, constructor, &cons);
    assert(status == napi_ok);

    size_t argc;
    napi_value instance;
    status = napi_new_instance(env, cons, argc, nullptr, &instance);
    assert(status == napi_ok);

    return instance;
  }
}

napi_value Transformer::loadStylesheet(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 1;
  napi_value value;
  napi_value jsthis;
  status = napi_get_cb_info(env, info, &argc, &value, &jsthis, nullptr);
  assert(status == napi_ok);

  Transformer *obj;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void **>(&obj));
  assert(status == napi_ok);

  char stylesheetPath[1024];
  size_t str_len;
  status = napi_get_value_string_utf8(env, value, (char *) &stylesheetPath, 1024, &str_len);
  assert(status == napi_ok);

  if (obj->stylesheetPtr != nullptr) xsltFreeStylesheet(obj->stylesheetPtr);
  obj->stylesheetPtr = xsltParseStylesheetFile((const xmlChar *)stylesheetPath);

  return jsthis;
}

napi_value Transformer::apply(napi_env env, napi_callback_info info) {
  napi_status status;
  xmlDocPtr inputXmlDocument, outputXmlDocument;

  size_t argc = 1;
  napi_value value;
  napi_value jsthis;
  status = napi_get_cb_info(env, info, &argc, &value, &jsthis, nullptr);
  assert(status == napi_ok);

  Transformer *obj;
  status = napi_unwrap(env, jsthis, reinterpret_cast<void **>(&obj));
  assert(status == napi_ok);

  char xmlDocumentPath[1024];
  size_t str_len;
  status = napi_get_value_string_utf8(env, value, (char *) &xmlDocumentPath, 1024, &str_len);
  assert(status == napi_ok);

  inputXmlDocument = xmlReadFile(xmlDocumentPath, NULL, 0);
  if (inputXmlDocument == NULL) {
    std::string message = "failed to parse " + (std::string)xmlDocumentPath;
    throw napi_throw_error(env, NULL, message.c_str());
  }

  outputXmlDocument = xsltApplyStylesheet(obj->stylesheetPtr, inputXmlDocument, obj->params);
  if (outputXmlDocument == NULL) {
    std::string message = "failed to transform " + (std::string)xmlDocumentPath;
    throw napi_throw_error(env, NULL, message.c_str());
  }

  xmlChar *stringResult;
  int stringResultLength;
  xsltSaveResultToString(&stringResult, &stringResultLength, outputXmlDocument, obj->stylesheetPtr);

  napi_value result;
  status = napi_create_string_utf8(env, (const char *)stringResult, NAPI_AUTO_LENGTH, &result);
  assert(status == napi_ok);

  xmlFreeDoc(inputXmlDocument);
  xmlFreeDoc(outputXmlDocument);
  xmlCleanupParser();
  xmlMemoryDump();

  return result;
}
