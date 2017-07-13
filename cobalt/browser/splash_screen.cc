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

#include "base/bind.h"
#include "base/threading/platform_thread.h"

namespace cobalt {
namespace browser {

SplashScreen::SplashScreen(base::ApplicationState initial_application_state,
                           const WebModule::OnRenderTreeProducedCallback&
                               render_tree_produced_callback,
                           network::NetworkModule* network_module,
                           const math::Size& window_dimensions,
                           render_tree::ResourceProvider* resource_provider,
                           float layout_refresh_rate, const GURL& url)
    : render_tree_produced_callback_(render_tree_produced_callback),
      is_ready_(true, false) {
  WebModule::Options web_module_options;
  web_module_options.name = "SplashScreenWebModule";

  // We want the splash screen to load and appear as quickly as possible, so
  // we set it and its image decoding thread to be high priority.
  web_module_options.thread_priority = base::kThreadPriority_High;
  web_module_options.loader_thread_priority = base::kThreadPriority_High;
  web_module_options.animated_image_decode_thread_priority =
      base::kThreadPriority_High;

  web_module_.reset(new WebModule(
      url, initial_application_state,
      base::Bind(&SplashScreen::OnRenderTreeProduced, base::Unretained(this)),
      base::Bind(&SplashScreen::OnError, base::Unretained(this)),
      base::Bind(&SplashScreen::OnWindowClosed, base::Unretained(this)),
      base::Closure(),  // window_minimize_callback
      &stub_media_module_, network_module, window_dimensions, resource_provider,
      stub_media_module_.system_window(), layout_refresh_rate,
      web_module_options));
}

SplashScreen::~SplashScreen() {
  // Destroy the web module first to prevent our callbacks from being called
  // (from another thread) while member objects are being destroyed.
  web_module_.reset();
}

void SplashScreen::WaitUntilReady() {
  is_ready_.Wait();
}

void SplashScreen::OnRenderTreeProduced(
    const browser::WebModule::LayoutResults& layout_results) {
  is_ready_.Signal();
  render_tree_produced_callback_.Run(layout_results);
}

void SplashScreen::OnWindowClosed() {
  is_ready_.Signal();
}

}  // namespace browser
}  // namespace cobalt
