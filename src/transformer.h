#if !defined(TRANSFORMER_H)
#define TRANSFORMER_H

#include <node_api.h>
#include <libxslt/xsltInternals.h>
#include <string>

class Transformer {
public:
  static napi_value Init(napi_env env, napi_value exports);
  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

private:
  explicit Transformer();
  ~Transformer();

  static napi_value New(napi_env env, napi_callback_info info);
  static napi_value loadStylesheet(napi_env env, napi_callback_info info);
  static napi_value apply(napi_env env, napi_callback_info info);
  static napi_ref constructor;
  xsltStylesheetPtr stylesheetPtr;
  const char *params[8] = {""};
  napi_env env_;
  napi_ref wrapper_;
};


#endif // TRANSFORMER_H
