// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media2/base/audio_decoder.h"

#include "media2/base/audio_buffer.h"

namespace media {

AudioDecoder::AudioDecoder() {}

AudioDecoder::~AudioDecoder() {}

bool AudioDecoder::NeedsBitstreamConversion() const {
  return false;
}

}  // namespace media
