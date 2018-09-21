#if !defined(SRC_DOCUMENT_H)
#define SRC_DOCUMENT_H

#include <node_api.h>

class Document {
 public:
  static napi_value Init(napi_env env, napi_value exports);
  static void Destructor(napi_env env, void* nativeObject, void* finalize_hint);

 private:
  explicit Document(double value_ = 0);
  ~Document();

  static napi_value New(napi_env env, napi_callback_info info);
  static napi_value GetValue(napi_env env, napi_callback_info info);
  static napi_value SetValue(napi_env env, napi_callback_info info);
  static napi_ref constructor;
  double value_;
  napi_env env_;
  napi_ref wrapper_;
};


#endif // SRC_DOCUMENT_H
