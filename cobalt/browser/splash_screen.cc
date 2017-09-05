// Copyright 2015 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "cobalt/browser/splash_screen.h"

#include <string>

#include "base/bind.h"
#include "base/callback.h"
#include "base/cancelable_callback.h"
#include "base/threading/platform_thread.h"
#include "base/time.h"
#include "cobalt/browser/splash_screen_cache.h"
#include "cobalt/loader/cache_fetcher.h"

namespace cobalt {
namespace browser {
namespace {

const int kSplashShutdownSeconds = 2;

void PostCallbackToMessageLoop(const base::Closure& callback,
                               MessageLoop* message_loop) {
  DCHECK(message_loop);
  message_loop->PostTask(FROM_HERE, callback);
}

// TODO: consolidate definitions of BindToLoop / BindToCurrentLoop
// from here and media in base.
base::Closure BindToLoop(const base::Closure& callback,
                         MessageLoop* message_loop) {
  return base::Bind(&PostCallbackToMessageLoop, callback, message_loop);
}

void OnError(const GURL& /* url */, const std::string& error) {
  LOG(ERROR) << error;
}

}  // namespace

SplashScreen::SplashScreen(
    base::ApplicationState initial_application_state,
    const WebModule::OnRenderTreeProducedCallback&
        render_tree_produced_callback,
    network::NetworkModule* network_module, const math::Size& window_dimensions,
    render_tree::ResourceProvider* resource_provider, float layout_refresh_rate,
    const GURL& fallback_splash_screen_url,
    const GURL& initial_main_web_module_url,
    SplashScreenCache* splash_screen_cache,
    const base::Callback<void()>& on_splash_screen_shutdown_complete)
    : render_tree_produced_callback_(render_tree_produced_callback),
      self_message_loop_(MessageLoop::current()),
      on_splash_screen_shutdown_complete_(on_splash_screen_shutdown_complete) {
  WebModule::Options web_module_options;
  web_module_options.name = "SplashScreenWebModule";

  // We want the splash screen to load and appear as quickly as possible, so
  // we set it and its image decoding thread to be high priority.
  web_module_options.thread_priority = base::kThreadPriority_High;
  web_module_options.loader_thread_priority = base::kThreadPriority_High;
  web_module_options.animated_image_decode_thread_priority =
      base::kThreadPriority_High;
  GURL url_to_pass = fallback_splash_screen_url;

  // Use the cached URL rather than the passed in URL if it exists.
  base::optional<std::string> key =
      SplashScreenCache::GetKeyForStartUrl(initial_main_web_module_url);
  if (key && splash_screen_cache &&
      splash_screen_cache->IsSplashScreenCached(*key)) {
    url_to_pass = GURL(loader::kCacheScheme + ("://" + *key));
    web_module_options.can_fetch_cache = true;
    web_module_options.splash_screen_cache = splash_screen_cache;
  }

  base::Callback<void()> on_window_close(
      BindToLoop(on_splash_screen_shutdown_complete, self_message_loop_));

  web_module_options.on_before_unload_fired_but_not_handled = on_window_close;

  web_module_.reset(new WebModule(
      url_to_pass, initial_application_state, render_tree_produced_callback_,
      base::Bind(&OnError), on_window_close,
      base::Closure(),  // window_minimize_callback
      &stub_media_module_, network_module, window_dimensions,
      1.f /*video_pixel_ratio*/, resource_provider, layout_refresh_rate,
      web_module_options));
}

SplashScreen::~SplashScreen() {
  DCHECK_EQ(MessageLoop::current(), self_message_loop_);
  // Destroy the web module first to prevent our callbacks from being called
  // (from another thread) while member objects are being destroyed.
  web_module_.reset();
  // Cancel any pending run of the splash screen shutdown callback.
  on_splash_screen_shutdown_complete_.Cancel();
}

void SplashScreen::Shutdown() {
  DCHECK_EQ(MessageLoop::current(), self_message_loop_);
  DCHECK(web_module_);
  if (!on_splash_screen_shutdown_complete_.callback().is_null()) {
    MessageLoop::current()->PostDelayedTask(
        FROM_HERE, on_splash_screen_shutdown_complete_.callback(),
        base::TimeDelta::FromSeconds(kSplashShutdownSeconds));
  }
  web_module_->InjectBeforeUnloadEvent();
}

}  // namespace browser
}  // namespace cobalt
