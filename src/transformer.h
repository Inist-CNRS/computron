#if !defined(TRANSFORMER_H)
#define TRANSFORMER_H

#include <napi.h>
#include <libxslt/xsltInternals.h>

class Transformer : public Napi::ObjectWrap<Transformer> {
public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Transformer(const Napi::CallbackInfo& info);
  ~Transformer();

private:
  static Napi::FunctionReference constructor;

  Napi::Value loadStylesheet(const Napi::CallbackInfo& info);
  Napi::Value apply(const Napi::CallbackInfo& info);
  
  xsltStylesheetPtr stylesheetPtr;
};


#endif // TRANSFORMER_H
