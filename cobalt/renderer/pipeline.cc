/*
 * Copyright 2014 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cobalt/renderer/pipeline.h"

#include "base/bind.h"

namespace cobalt {
namespace renderer {

namespace {
// The frequency we signal a new rasterization fo the render tree.
const float kRefreshRate = 60.0f;
}  // namespace

Pipeline::Pipeline(scoped_ptr<Rasterizer> rasterizer,
                   const scoped_refptr<backend::RenderTarget>& render_target)
    : rasterizer_(rasterizer.Pass()),
      render_target_(render_target),
      refresh_rate_(kRefreshRate),
      rasterizer_thread_(base::in_place, "Rasterizer") {
  // The actual Pipeline can be constructed from any thread, but we want
  // rasterizer_thread_checker_ to be associated with the rasterizer thread,
  // so we detach it here and let it reattach itself to the rasterizer thread
  // when CalledOnValidThread() is called on rasterizer_thread_checker_ below.
  rasterizer_thread_checker_.DetachFromThread();
  rasterizer_thread_->Start();
}

Pipeline::~Pipeline() {
  // Submit a shutdown task to the rasterizer thread so that it can shutdown
  // anything that must be shutdown from that thread.
  rasterizer_thread_->message_loop()->PostTask(
      FROM_HERE,
      base::Bind(&Pipeline::ShutdownRasterizerThread, base::Unretained(this)));

  rasterizer_thread_ = base::nullopt;
}

void Pipeline::Submit(const scoped_refptr<render_tree::Node>& render_tree) {
  // Execute the actual set of the new render tree on the rasterizer tree.
  rasterizer_thread_->message_loop()->PostTask(
      FROM_HERE, base::Bind(&Pipeline::SetNewRenderTree, base::Unretained(this),
                            render_tree));
}

void Pipeline::SetNewRenderTree(
    const scoped_refptr<render_tree::Node>& render_tree) {
  DCHECK(rasterizer_thread_checker_.CalledOnValidThread());
  DCHECK(render_tree.get());

  current_tree_ = render_tree;

  // Start the rasterization timer if it is not yet started.
  if (!refresh_rate_timer_) {
    // We add 1 to the refresh rate in the following timer_interval calculation
    // to ensure that we are submitting frames to the rasterizer at least
    // as fast as it can consume them, thus ensuring we don't miss a frame.
    // The rasterizer is responsible for pacing us if we submit too quickly,
    // though this can result in a backed up submission log which can result
    // in input lag.  Eventually (likely after a profiling system is introduced
    // to better measure timing issues like this), we will investigate adding
    // regulator code to attempt to make render tree submissions at times that
    // result in the least amount of input lag while still providing a
    // submission for each VSync.
    // TODO(***REMOVED***): It should instead be investigated if we can trigger
    //               rasterizer submits based on a platform-specific VSync
    //               signal instead of relying on a timer that may or may not
    //               match the precise VSync interval of the hardware.
    const float timer_interval =
        base::Time::kMicrosecondsPerSecond * 1.0f / (kRefreshRate + 1.0f);

    refresh_rate_timer_.emplace(
        FROM_HERE,
        base::TimeDelta::FromMicroseconds(timer_interval),
        base::Bind(&Pipeline::RasterizeCurrentTree, base::Unretained(this)),
        true);
    refresh_rate_timer_->Reset();
  }
}

void Pipeline::RasterizeCurrentTree() {
  DCHECK(rasterizer_thread_checker_.CalledOnValidThread());
  DCHECK(current_tree_.get());

  // Rasterize the last submitted render tree.
  rasterizer_->Submit(current_tree_, render_target_);
}

void Pipeline::ShutdownRasterizerThread() {
  DCHECK(rasterizer_thread_checker_.CalledOnValidThread());
  // Stop and shutdown the raterizer timer.
  refresh_rate_timer_ = base::nullopt;

  // Do not retain any more references to the current render tree (which
  // may refer to rasterizer resources).
  current_tree_ = NULL;

  // Finally, destroy the rasterizer.
  rasterizer_.reset();
}

}  // namespace renderer
}  // namespace cobalt
