// Copyright 2016 The Cobalt Authors. All Rights Reserved.
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

// Module Overview: Starboard DRM module
//
// Provides definitions that allow for DRM support, which are common
// between Player and Decoder interfaces.

#ifndef STARBOARD_DRM_H_
#define STARBOARD_DRM_H_

#include "starboard/export.h"
#include "starboard/types.h"

#ifdef __cplusplus
extern "C" {
#endif

// --- Types -----------------------------------------------------------------

// The type of the session request.
// https://www.w3.org/TR/encrypted-media/#idl-def-mediakeymessagetype
typedef enum SbDrmSessionRequestType {
  kSbDrmSessionRequestTypeLicenseRequest,
  kSbDrmSessionRequestTypeLicenseRenewal,
  kSbDrmSessionRequestTypeLicenseRelease,
  kSbDrmSessionRequestTypeIndividualizationRequest,
} SbDrmSessionRequestType;

// The status of session related operations.
// Used by |SbDrmSessionUpdateRequestFunc|, |SbDrmSessionUpdatedFunc|, and
// |SbDrmServerCertificateUpdatedFunc| to indicate the status of the operation.
// https://w3c.github.io/encrypted-media/#error-names
typedef enum SbDrmStatus {
  kSbDrmStatusSuccess,
  kSbDrmStatusTypeError,
  kSbDrmStatusNotSupportedError,
  kSbDrmStatusInvalidStateError,
  kSbDrmStatusQuotaExceededError,
  // The following error can be used when the error status cannot be mapped to
  // one of the above errors.
  kSbDrmStatusUnknownError,
} SbDrmStatus;

// Status of a particular media key.
// https://w3c.github.io/encrypted-media/#idl-def-MediaKeyStatus
typedef enum SbDrmKeyStatus {
  kSbDrmKeyStatusUsable,
  kSbDrmKeyStatusExpired,
  kSbDrmKeyStatusReleased,
  kSbDrmKeyStatusRestricted,
  kSbDrmKeyStatusDownscaled,
  kSbDrmKeyStatusPending,
  kSbDrmKeyStatusError,
} SbDrmKeyStatus;

// A mapping of clear and encrypted bytes for a single subsample. All
// subsamples within a sample must be encrypted with the same encryption
// parameters. The clear bytes always appear first in the sample.
typedef struct SbDrmSubSampleMapping {
  // How many bytes of the corresponding subsample are not encrypted
  int32_t clear_byte_count;

  // How many bytes of the corresponding subsample are encrypted.
  int32_t encrypted_byte_count;
} SbDrmSubSampleMapping;

#if SB_HAS(DRM_KEY_STATUSES)
typedef struct SbDrmKeyId {
  // The ID of the license (or key) required to decrypt this sample. For
  // PlayReady, this is the license GUID in packed little-endian binary form.
  uint8_t identifier[16];
  int identifier_size;
} SbDrmKeyId;
#endif  // SB_HAS(DRM_KEY_STATUSES)

// All the optional information needed per sample for encrypted samples.
typedef struct SbDrmSampleInfo {
  // The Initialization Vector needed to decrypt this sample.
  uint8_t initialization_vector[16];
  int initialization_vector_size;

  // The ID of the license (or key) required to decrypt this sample. For
  // PlayReady, this is the license GUID in packed little-endian binary form.
  uint8_t identifier[16];
  int identifier_size;

  // The number of subsamples in this sample, must be at least 1.
  int32_t subsample_count;

  // The clear/encrypted mapping of each subsample in this sample. This must be
  // an array of |subsample_count| mappings.
  const SbDrmSubSampleMapping* subsample_mapping;
} SbDrmSampleInfo;

// A handle to a DRM system which can be used with either an SbDecoder or a
// SbPlayer.
typedef struct SbDrmSystemPrivate* SbDrmSystem;

// A callback that will receive generated session update request when requested
// from a SbDrmSystem. |drm_system| will be the DRM system that
// SbDrmGenerateSessionUpdateRequest() was called on. |context| will be the same
// context that was passed into the call to SbDrmCreateSystem().
//
#if SB_API_VERSION >= 10
// |status| is the status of the session request.
//
// |type| is the status of the session request.
//
// |error_message| may contain an optional error message when |status| isn't
// |kSbDrmStatusSuccess| to provide more details about the error.  It may be
// NULL if |status| is |kSbDrmStatusSuccess| or if no error message can be
// provided.
#endif  // SB_API_VERSION >= 10
// |ticket| will be the same ticket that was passed to
// SbDrmGenerateSessionUpdateRequest() or |kSbDrmTicketInvalid| if the update
// request was generated by the DRM system.
//
// |session_id| can be NULL if there was an error generating the request.
#if SB_API_VERSION >= 10
typedef void (*SbDrmSessionUpdateRequestFunc)(SbDrmSystem drm_system,
                                              void* context,
                                              int ticket,
                                              SbDrmStatus status,
                                              SbDrmSessionRequestType type,
                                              const char* error_message,
                                              const void* session_id,
                                              int session_id_size,
                                              const void* content,
                                              int content_size,
                                              const char* url);
#else   // SB_API_VERSION >= 10
typedef void (*SbDrmSessionUpdateRequestFunc)(SbDrmSystem drm_system,
                                              void* context,
                                              int ticket,
                                              const void* session_id,
                                              int session_id_size,
                                              const void* content,
                                              int content_size,
                                              const char* url);
#endif  // SB_API_VERSION >= 10

// A callback for notifications that a session has been added, and subsequent
// encrypted samples are actively ready to be decoded. |drm_system| will be the
// DRM system that SbDrmUpdateSession() was called on. |context| will be the
// same context passed into that call to SbDrmCreateSystem().
//
// |ticket| will be the same ticket that was passed to SbDrmUpdateSession().
//
#if SB_API_VERSION >= 10
// |status| is the status of the session request.
//
// |error_message| may contain an optional error message when |status| isn't
// |kSbDrmStatusSuccess| to provide more details about the error.  It may be
// NULL if |status| is |kSbDrmStatusSuccess| or if no error message can be
// provided.
#endif  // SB_API_VERSION >= 10
// |succeeded| is whether the session was successfully updated or not.
#if SB_API_VERSION >= 10
typedef void (*SbDrmSessionUpdatedFunc)(SbDrmSystem drm_system,
                                        void* context,
                                        int ticket,
                                        SbDrmStatus status,
                                        const char* error_message,
                                        const void* session_id,
                                        int session_id_size);
#else   // SB_API_VERSION >= 10
typedef void (*SbDrmSessionUpdatedFunc)(SbDrmSystem drm_system,
                                        void* context,
                                        int ticket,
                                        const void* session_id,
                                        int session_id_size,
                                        bool succeeded);
#endif  // SB_API_VERSION >= 10

// A callback for notifications that the status of one or more keys in a session
// has been changed.  All keys of the session and their new status will be
// passed along.  Any keys not in the list is considered as deleted.
#if SB_HAS(DRM_KEY_STATUSES)
typedef void (*SbDrmSessionKeyStatusesChangedFunc)(
    SbDrmSystem drm_system,
    void* context,
    const void* session_id,
    int session_id_size,
    int number_of_keys,
    const SbDrmKeyId* key_ids,
    const SbDrmKeyStatus* key_statuses);
#endif  // SB_HAS(DRM_KEY_STATUSES)

// A callback for signalling that a session has been closed by the SbDrmSystem
#if SB_HAS(DRM_SESSION_CLOSED)
typedef void (*SbDrmSessionClosedFunc)(
    SbDrmSystem drm_system,
    void* context,
    const void* session_id,
    int session_id_size);
#endif  // SB_HAS(DRM_SESSION_CLOSED))

#if SB_API_VERSION >= 10
// A callback to notify the caller of SbDrmUpdateServerCertificate() whether the
// update has been successfully updated or not.
typedef void (*SbDrmServerCertificateUpdatedFunc)(SbDrmSystem drm_system,
                                                  void* context,
                                                  int ticket,
                                                  SbDrmStatus status,
                                                  const char* error_message);
#endif  // SB_API_VERSION >= 10

// --- Constants -------------------------------------------------------------

// An invalid SbDrmSystem.
#define kSbDrmSystemInvalid ((SbDrmSystem)NULL)

// A ticket for callback invocations initiated by the DRM system.
#define kSbDrmTicketInvalid kSbInvalidInt

// --- Functions -------------------------------------------------------------

// Indicates whether |drm_system| is a valid SbDrmSystem.
static SB_C_FORCE_INLINE bool SbDrmSystemIsValid(SbDrmSystem drm) {
  return drm != kSbDrmSystemInvalid;
}

// Indicates whether |ticket| is a valid ticket.
static SB_C_FORCE_INLINE bool SbDrmTicketIsValid(int ticket) {
  return ticket != kSbDrmTicketInvalid;
}

// Creates a new DRM system that can be used when constructing an SbPlayer
// or an SbDecoder.
//
// This function returns kSbDrmSystemInvalid if |key_system| is unsupported.
//
// Also see the documentation of SbDrmGenerateSessionUpdateRequest() and
// SbDrmUpdateSession() for more details.
//
// |key_system|: The DRM key system to be created. The value should be in the
// form of "com.example.somesystem" as suggested by
// https://w3c.github.io/encrypted-media/#key-system. All letters in the value
// should be lowercase and will be matched exactly with known DRM key systems
// of the platform.
// |context|: A value passed when any of this function's callback parameters
// are called.
// |update_request_callback|: A function that is called every time after
// SbDrmGenerateSessionUpdateRequest() is called.
// |session_updated_callback|: A function that is called every time after
// SbDrmUpdateSession() is called.
// |key_statuses_changed_callback|: A function that can be called to indicate
// that key statuses have changed.
// |server_certificate_updated_callback|: A function that is called to report
// whether the server certificate has been successfully updated.  It is called
// once and only once.  It is possible that the callback is called before the
// function returns.
// |session_closed_callback|: A function that can be called to indicate that a
// session has closed.
#if SB_API_VERSION >= 10
// If |NULL| is passed for any of the callbacks (|update_request_callback|,
// |session_updated_callback|, |key_statuses_changed_callback|,
// |server_certificate_updated_callback|, or |session_closed_callback|), then
// |kSbDrmSystemInvalid| must be returned.
#endif  // SB_API_VERSION >= 10

#if SB_API_VERSION >= 10

SB_EXPORT SbDrmSystem SbDrmCreateSystem(
    const char* key_system,
    void* context,
    SbDrmSessionUpdateRequestFunc update_request_callback,
    SbDrmSessionUpdatedFunc session_updated_callback,
    SbDrmSessionKeyStatusesChangedFunc key_statuses_changed_callback,
    SbDrmServerCertificateUpdatedFunc server_certificate_updated_callback,
    SbDrmSessionClosedFunc session_closed_callback);

#elif SB_HAS(DRM_SESSION_CLOSED)

#if !SB_HAS(DRM_KEY_STATUSES)
#error "Platforms with SB_HAS_DRM_SESSION_CLOSED must also set "
    "SB_HAS_DRM_KEY_STATUSES"
#endif  // !SB_HAS(DRM_KEY_STATUSES)

SB_EXPORT SbDrmSystem SbDrmCreateSystem(
    const char* key_system,
    void* context,
    SbDrmSessionUpdateRequestFunc update_request_callback,
    SbDrmSessionUpdatedFunc session_updated_callback,
    SbDrmSessionKeyStatusesChangedFunc key_statuses_changed_callback,
    SbDrmSessionClosedFunc session_closed_callback);

#elif SB_HAS(DRM_KEY_STATUSES)

SB_EXPORT SbDrmSystem SbDrmCreateSystem(
    const char* key_system,
    void* context,
    SbDrmSessionUpdateRequestFunc update_request_callback,
    SbDrmSessionUpdatedFunc session_updated_callback,
    SbDrmSessionKeyStatusesChangedFunc key_statuses_changed_callback);

#else  // SB_HAS(DRM_KEY_STATUSES)

SB_EXPORT SbDrmSystem
SbDrmCreateSystem(const char* key_system,
                  void* context,
                  SbDrmSessionUpdateRequestFunc update_request_callback,
                  SbDrmSessionUpdatedFunc session_updated_callback);

#endif  // SB_HAS(DRM_KEY_STATUSES)

// Asynchronously generates a session update request payload for
// |initialization_data|, of |initialization_data_size|, in case sensitive
// |type|, extracted from the media stream, in |drm_system|'s key system.
//
// This function calls |drm_system|'s |update_request_callback| function,
// which is defined when the DRM system is created by SbDrmCreateSystem. When
// calling that function, this function either sends |context| (also from
// |SbDrmCreateSystem|) and a populated request, or it sends NULL |session_id|
// if an error occurred.
//
// |drm_system|'s |context| may be used to route callbacks back to an object
// instance.
//
// Callbacks may be called either from the current thread before this function
// returns or from another thread.
//
// |drm_system|: The DRM system that defines the key system used for the
// session update request payload as well as the callback function that is
// called as a result of the function being called.
//
// |ticket|: The opaque ID that allows to distinguish callbacks from multiple
// concurrent calls to SbDrmGenerateSessionUpdateRequest(), which will be passed
// to |update_request_callback| as-is. It is the responsibility of the caller to
// establish ticket uniqueness, issuing multiple requests with the same ticket
// may result in undefined behavior. The value |kSbDrmTicketInvalid| must not be
// used.
//
// |type|: The case-sensitive type of the session update request payload.
// |initialization_data|: The data for which the session update request payload
// is created.
// |initialization_data_size|: The size of the session update request payload.
SB_EXPORT void SbDrmGenerateSessionUpdateRequest(
    SbDrmSystem drm_system,
    int ticket,
    const char* type,
    const void* initialization_data,
    int initialization_data_size);

// Update session with |key|, in |drm_system|'s key system, from the license
// server response. Calls |session_updated_callback| with |context| and whether
// the update succeeded. |context| may be used to route callbacks back to
// an object instance.
//
// |ticket| is the opaque ID that allows to distinguish callbacks from
// multiple concurrent calls to SbDrmUpdateSession(), which will be passed
// to |session_updated_callback| as-is. It is the responsibility of the caller
// to establish ticket uniqueness, issuing multiple calls with the same ticket
// may result in undefined behavior.
//
// Once the session is successfully updated, an SbPlayer or SbDecoder associated
// with that DRM key system will be able to decrypt encrypted samples.
//
// |drm_system|'s |session_updated_callback| may called either from the
// current thread before this function returns or from another thread.
SB_EXPORT void SbDrmUpdateSession(SbDrmSystem drm_system,
                                  int ticket,
                                  const void* key,
                                  int key_size,
                                  const void* session_id,
                                  int session_id_size);

// Clear any internal states/resources related to the specified |session_id|.
SB_EXPORT void SbDrmCloseSession(SbDrmSystem drm_system,
                                 const void* session_id,
                                 int session_id_size);

// Returns the number of keys installed in |drm_system|.
//
// |drm_system|: The system for which the number of installed keys is retrieved.
SB_EXPORT int SbDrmGetKeyCount(SbDrmSystem drm_system);

// Gets |out_key|, |out_key_size|, and |out_status| for the key with |index|
// in |drm_system|. Returns whether a key is installed at |index|.
// If not, the output parameters, which all must not be NULL, will not be
// modified.
SB_EXPORT bool SbDrmGetKeyStatus(SbDrmSystem drm_system,
                                 const void* session_id,
                                 int session_id_size,
                                 int index,
                                 void** out_key,
                                 int* out_key_size,
                                 SbDrmKeyStatus* out_status);

// Removes all installed keys for |drm_system|. Any outstanding session update
// requests are also invalidated.
//
// |drm_system|: The DRM system for which keys should be removed.
SB_EXPORT void SbDrmRemoveAllKeys(SbDrmSystem drm_system);

#if SB_API_VERSION >= 10

// Returns true if server certificate of |drm_system| can be updated via
// SbDrmUpdateServerCertificate().  The return value should remain the same
// during the life time of |drm_system|.
//
// |drm_system|: The DRM system to check if its server certificate is updatable.
SB_EXPORT bool SbDrmIsServerCertificateUpdatable(SbDrmSystem drm_system);

// Update the server certificate of |drm_system|.  The function can be called
// multiple times.  It is possible that a call to it happens before the callback
// of a previous call is called.
// Note that this function should only be called after
// |SbDrmIsServerCertificateUpdatable| is called first and returned true.
//
// |drm_system|: The DRM system whose server certificate is being updated.
// |ticket|: The opaque ID that allows to distinguish callbacks from multiple
// concurrent calls to SbDrmUpdateServerCertificate(), which will be passed to
// |server_certificate_updated_callback| as-is. It is the responsibility of the
// caller to establish ticket uniqueness, issuing multiple requests with the
// same ticket may result in undefined behavior. The value |kSbDrmTicketInvalid|
// must not be used.
// |certificate|: Pointer to the server certificate data.
// |certificate_size|: Size of the server certificate data.
SB_EXPORT void SbDrmUpdateServerCertificate(SbDrmSystem drm_system,
                                            int ticket,
                                            const void* certificate,
                                            int certificate_size);

#endif  // SB_API_VERSION >= 10

// Destroys |drm_system|, which implicitly removes all keys installed in it and
// invalidates all outstanding session update requests. A DRM system cannot be
// destroyed unless any associated SbPlayer or SbDecoder has first been
// destroyed.
//
// All callbacks are guaranteed to be finished when this function returns.
// As a result, if this function is called from a callback that is passed
// to SbDrmCreateSystem(), a deadlock will occur.
//
// |drm_system|: The DRM system to be destroyed.
SB_EXPORT void SbDrmDestroySystem(SbDrmSystem drm_system);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // STARBOARD_DRM_H_
