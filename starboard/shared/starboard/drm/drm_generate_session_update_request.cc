// Copyright 2016 Google Inc. All Rights Reserved.
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

#include "starboard/drm.h"

#include "starboard/log.h"
#include "starboard/shared/starboard/drm/drm_system_internal.h"

void SbDrmGenerateSessionUpdateRequest(SbDrmSystem drm_system,
#if SB_API_VERSION >= SB_DRM_SESSION_UPDATE_REQUEST_TICKET_VERSION
                                       int ticket,
#endif  // SB_API_VERSION >= SB_DRM_SESSION_UPDATE_REQUEST_TICKET_VERSION
                                       const char* type,
                                       const void* initialization_data,
                                       int initialization_data_size) {
  if (!SbDrmSystemIsValid(drm_system)) {
    SB_DLOG(WARNING) << "Invalid drm system";
    return;
  }

  drm_system->GenerateSessionUpdateRequest(
#if SB_API_VERSION >= SB_DRM_SESSION_UPDATE_REQUEST_TICKET_VERSION
      ticket,
#endif  // SB_API_VERSION >= SB_DRM_SESSION_UPDATE_REQUEST_TICKET_VERSION
      type, initialization_data, initialization_data_size);
}
