// Copyright (c) 2017 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/browser/api/atom_api_browser_view.h"

#include "atom/browser/api/atom_api_web_contents.h"
#include "atom/browser/browser.h"
#include "atom/common/native_mate_converters/value_converter.h"
#include "atom/common/options_switches.h"
#include "native_mate/constructor.h"
#include "native_mate/dictionary.h"

#include "atom/common/node_includes.h"

namespace atom {

namespace api {

BrowserView::BrowserView(v8::Isolate* isolate, v8::Local<v8::Object> wrapper,
               const mate::Dictionary& options) {
  mate::Handle<class WebContents> web_contents;
  // If no WebContents was passed to the constructor, create it from options.
  if (!options.Get("webContents", &web_contents)) {
    // Use options.webPreferences to create WebContents.
    mate::Dictionary web_preferences = mate::Dictionary::CreateEmpty(isolate);
    options.Get(options::kWebPreferences, &web_preferences);

    // Copy the backgroundColor to webContents.
    v8::Local<v8::Value> value;
    if (options.Get(options::kBackgroundColor, &value))
      web_preferences.Set(options::kBackgroundColor, value);

    // Creates the WebContents used by BrowserView.
    web_contents = WebContents::Create(isolate, web_preferences);

    mate::Dictionary load_options = mate::Dictionary::CreateEmpty(isolate);
    web_contents->LoadURL(GURL("https://www.figma.com"), load_options);
  }

  Init(isolate, wrapper, options, web_contents);
}

void BrowserView::Init(v8::Isolate* isolate,
                  v8::Local<v8::Object> wrapper,
                  const mate::Dictionary& options,
                  mate::Handle<class WebContents> web_contents) {
  web_contents_.Reset(isolate, web_contents.ToV8());
  api_web_contents_ = web_contents.get();

  InitWith(isolate, wrapper);
}

BrowserView::~BrowserView() {
}

// static
mate::WrappableBase* BrowserView::New(mate::Arguments* args) {
  if (!Browser::Get()->is_ready()) {
    args->ThrowError("Cannot create BrowserView before app is ready");
    return nullptr;
  }

  if (args->Length() > 1) {
    args->ThrowError();
    return nullptr;
  }

  mate::Dictionary options;
  if (!(args->Length() == 1 && args->GetNext(&options))) {
    options = mate::Dictionary::CreateEmpty(args->isolate());
  }

  return new BrowserView(args->isolate(), args->GetThis(), options);
}

int32_t BrowserView::ID() const {
  return weak_map_id();
}

v8::Local<v8::Value> BrowserView::WebContents(v8::Isolate* isolate) {
  if (web_contents_.IsEmpty()) {
    return v8::Null(isolate);
  }

  return v8::Local<v8::Value>::New(isolate, web_contents_);
}

// static
void BrowserView::BuildPrototype(v8::Isolate* isolate,
                                 v8::Local<v8::FunctionTemplate> prototype) {
  prototype->SetClassName(mate::StringToV8(isolate, "BrowserView"));
  mate::ObjectTemplateBuilder(isolate, prototype->PrototypeTemplate())
      .MakeDestroyable()
      .SetProperty("id", &BrowserView::ID)
      .SetProperty("webContents", &BrowserView::WebContents);
}

}  // namespace api

}  // namespace atom

namespace {

using atom::api::BrowserView;

void Initialize(v8::Local<v8::Object> exports,
                v8::Local<v8::Value> unused,
                v8::Local<v8::Context> context,
                void* priv) {
  v8::Isolate* isolate = context->GetIsolate();
  BrowserView::SetConstructor(isolate, base::Bind(&BrowserView::New));

  mate::Dictionary browser_view(
      isolate, BrowserView::GetConstructor(isolate)->GetFunction());

  mate::Dictionary dict(isolate, exports);
  dict.Set("BrowserView", browser_view);
}

}  // namespace

NODE_MODULE_CONTEXT_AWARE_BUILTIN(atom_browser_view, Initialize)
