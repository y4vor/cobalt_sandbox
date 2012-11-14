// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/environment.h"
#include "base/file_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/synchronization/lock.h"
#include "base/test/test_timeouts.h"
#include "base/time.h"
#include "build/build_config.h"
#include "media/audio/audio_io.h"
#include "media/audio/audio_manager_base.h"
#include "media/audio/audio_util.h"
#include "media/base/seekable_buffer.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_LINUX) || defined(OS_OPENBSD)
#include "media/audio/linux/audio_manager_linux.h"
#elif defined(OS_MACOSX)
#include "media/audio/mac/audio_manager_mac.h"
#elif defined(OS_WIN)
#include "base/win/scoped_com_initializer.h"
#include "media/audio/win/audio_manager_win.h"
#elif defined(OS_ANDROID)
#include "media/audio/android/audio_manager_android.h"
#endif

namespace media {

#if defined(OS_LINUX) || defined(OS_OPENBSD)
typedef AudioManagerLinux AudioManagerAnyPlatform;
#elif defined(OS_MACOSX)
typedef AudioManagerMac AudioManagerAnyPlatform;
#elif defined(OS_WIN)
typedef AudioManagerWin AudioManagerAnyPlatform;
#elif defined(OS_ANDROID)
typedef AudioManagerAndroid AudioManagerAnyPlatform;
#endif

// Limits the number of delay measurements we can store in an array and
// then write to file at end of the WASAPIAudioInputOutputFullDuplex test.
static const size_t kMaxDelayMeasurements = 1000;

// Name of the output text file. The output file will be stored in the
// directory containing media_unittests.exe.
// Example: \src\build\Debug\audio_delay_values_ms.txt.
// See comments for the WASAPIAudioInputOutputFullDuplex test for more details
// about the file format.
static const char* kDelayValuesFileName = "audio_delay_values_ms.txt";

// Contains delay values which are reported during the full-duplex test.
// Total delay = |buffer_delay_ms| + |input_delay_ms| + |output_delay_ms|.
struct AudioDelayState {
  AudioDelayState()
      : delta_time_ms(0),
        buffer_delay_ms(0),
        input_delay_ms(0),
        output_delay_ms(0) {
  }

  // Time in milliseconds since last delay report. Typical value is ~10 [ms].
  int delta_time_ms;

  // Size of internal sync buffer. Typical value is ~0 [ms].
  int buffer_delay_ms;

  // Reported capture/input delay. Typical value is ~10 [ms].
  int input_delay_ms;

  // Reported render/output delay. Typical value is ~40 [ms].
  int output_delay_ms;
};

// This class mocks the platform specific audio manager and overrides
// the GetMessageLoop() method to ensure that we can run our tests on
// the main thread instead of the audio thread.
class MockAudioManager : public AudioManagerAnyPlatform {
 public:
  MockAudioManager() {}
  virtual ~MockAudioManager() {}

  virtual scoped_refptr<base::MessageLoopProxy> GetMessageLoop() OVERRIDE {
    return MessageLoop::current()->message_loop_proxy();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(MockAudioManager);
};

// Test fixture class.
class AudioLowLatencyInputOutputTest : public testing::Test {
 protected:
  AudioLowLatencyInputOutputTest() {}

  virtual ~AudioLowLatencyInputOutputTest() {}

  AudioManager* audio_manager() { return &mock_audio_manager_; }
  MessageLoopForUI* message_loop() { return &message_loop_; }

  // Convenience method which ensures that we are not running on the build
  // bots and that at least one valid input and output device can be found.
  bool CanRunAudioTests() {
    bool input = audio_manager()->HasAudioInputDevices();
    bool output = audio_manager()->HasAudioOutputDevices();
    LOG_IF(WARNING, !input) << "No input device detected.";
    LOG_IF(WARNING, !output) << "No output device detected.";
    return input && output;
  }

 private:
  MessageLoopForUI message_loop_;
  MockAudioManager mock_audio_manager_;

  DISALLOW_COPY_AND_ASSIGN(AudioLowLatencyInputOutputTest);
};

// This audio source/sink implementation should be used for manual tests
// only since delay measurements are stored on an output text file.
// All incoming/recorded audio packets are stored in an intermediate media
// buffer which the renderer reads from when it needs audio for playout.
// The total effect is that recorded audio is played out in loop back using
// a sync buffer as temporary storage.
class FullDuplexAudioSinkSource
    : public AudioInputStream::AudioInputCallback,
      public AudioOutputStream::AudioSourceCallback {
 public:
  FullDuplexAudioSinkSource(int sample_rate,
                            int samples_per_packet,
                            int channels)
    : sample_rate_(sample_rate),
      samples_per_packet_(samples_per_packet),
      channels_(channels),
      input_elements_to_write_(0),
      output_elements_to_write_(0),
      previous_write_time_(base::Time::Now()) {
    // Size in bytes of each audio frame (4 bytes for 16-bit stereo PCM).
    frame_size_ = (16 / 8) * channels_;

    // Start with the smallest possible buffer size. It will be increased
    // dynamically during the test if required.
    buffer_.reset(
        new media::SeekableBuffer(0, samples_per_packet_ * frame_size_));

    frames_to_ms_ = static_cast<double>(1000.0 / sample_rate_);
    delay_states_.reset(new AudioDelayState[kMaxDelayMeasurements]);
  }

  virtual ~FullDuplexAudioSinkSource() {
    // Get complete file path to output file in the directory containing
    // media_unittests.exe. Example: src/build/Debug/audio_delay_values_ms.txt.
    FilePath file_name;
    EXPECT_TRUE(PathService::Get(base::DIR_EXE, &file_name));
    file_name = file_name.AppendASCII(kDelayValuesFileName);

    FILE* text_file = file_util::OpenFile(file_name, "wt");
    DLOG_IF(ERROR, !text_file) << "Failed to open log file.";
    LOG(INFO) << ">> Output file " << file_name.value() << " has been created.";

    // Write the array which contains time-stamps, buffer size and
    // audio delays values to a text file.
    size_t elements_written = 0;
    while (elements_written <
        std::min(input_elements_to_write_, output_elements_to_write_)) {
      const AudioDelayState state = delay_states_[elements_written];
      fprintf(text_file, "%d %d %d %d\n",
              state.delta_time_ms,
              state.buffer_delay_ms,
              state.input_delay_ms,
              state.output_delay_ms);
      ++elements_written;
    }

    file_util::CloseFile(text_file);
  }

  // AudioInputStream::AudioInputCallback.
  virtual void OnData(AudioInputStream* stream,
                      const uint8* src, uint32 size,
                      uint32 hardware_delay_bytes,
                      double volume) OVERRIDE {
    base::AutoLock lock(lock_);

    // Update three components in the AudioDelayState for this recorded
    // audio packet.
    base::Time now_time = base::Time::Now();
    int diff = (now_time - previous_write_time_).InMilliseconds();
    previous_write_time_ = now_time;
    if (input_elements_to_write_ < kMaxDelayMeasurements) {
      delay_states_[input_elements_to_write_].delta_time_ms = diff;
      delay_states_[input_elements_to_write_].buffer_delay_ms =
          BytesToMilliseconds(buffer_->forward_bytes());
      delay_states_[input_elements_to_write_].input_delay_ms =
          BytesToMilliseconds(hardware_delay_bytes);
      ++input_elements_to_write_;
    }

    // Store the captured audio packet in a seekable media buffer.
    if (!buffer_->Append(src, size)) {
      // An attempt to write outside the buffer limits has been made.
      // Double the buffer capacity to ensure that we have a buffer large
      // enough to handle the current sample test scenario.
      buffer_->set_forward_capacity(2 * buffer_->forward_capacity());
      buffer_->Clear();
    }
  }

  virtual void OnClose(AudioInputStream* stream) OVERRIDE {}
  virtual void OnError(AudioInputStream* stream, int code) OVERRIDE {}

  // AudioOutputStream::AudioSourceCallback.
  virtual int OnMoreData(AudioBus* audio_bus,
                         AudioBuffersState buffers_state) OVERRIDE {
    base::AutoLock lock(lock_);

    // Update one component in the AudioDelayState for the packet
    // which is about to be played out.
    if (output_elements_to_write_ < kMaxDelayMeasurements) {
      int output_delay_bytes = buffers_state.hardware_delay_bytes;
#if defined(OS_WIN)
      // Special fix for Windows in combination with Wave where the
      // pending bytes field of the audio buffer state is used to
      // report the delay.
      if (!media::IsWASAPISupported()) {
        output_delay_bytes = buffers_state.pending_bytes;
      }
#endif
      delay_states_[output_elements_to_write_].output_delay_ms =
          BytesToMilliseconds(output_delay_bytes);
      ++output_elements_to_write_;
    }

    int size;
    const uint8* source;
    // Read the data from the seekable media buffer which contains
    // captured data at the same size and sample rate as the output side.
    if (buffer_->GetCurrentChunk(&source, &size) && size > 0) {
      EXPECT_EQ(channels_, audio_bus->channels());
      size = std::min(audio_bus->frames() * frame_size_, size);
      EXPECT_EQ(static_cast<size_t>(size) % sizeof(*audio_bus->channel(0)), 0U);
      audio_bus->FromInterleaved(
          source, size / frame_size_, frame_size_ / channels_);
      buffer_->Seek(size);
      return size / frame_size_;
    }

    return 0;
  }

  virtual int OnMoreIOData(AudioBus* source,
                           AudioBus* dest,
                           AudioBuffersState buffers_state) OVERRIDE {
    NOTREACHED();
    return 0;
  }

  virtual void OnError(AudioOutputStream* stream, int code) OVERRIDE {}
  virtual void WaitTillDataReady() OVERRIDE {}

 protected:
  // Converts from bytes to milliseconds taking the sample rate and size
  // of an audio frame into account.
  int BytesToMilliseconds(uint32 delay_bytes) const {
    return static_cast<int>((delay_bytes / frame_size_) * frames_to_ms_ + 0.5);
  }

 private:
  base::Lock lock_;
  scoped_ptr<media::SeekableBuffer> buffer_;
  int sample_rate_;
  int samples_per_packet_;
  int channels_;
  int frame_size_;
  double frames_to_ms_;
  scoped_array<AudioDelayState> delay_states_;
  size_t input_elements_to_write_;
  size_t output_elements_to_write_;
  base::Time previous_write_time_;
};

class AudioInputStreamTraits {
 public:
  typedef AudioInputStream StreamType;

  static int HardwareSampleRate() {
    return static_cast<int>(media::GetAudioInputHardwareSampleRate(
        AudioManagerBase::kDefaultDeviceId));
  }

  // TODO(henrika): add support for GetAudioInputHardwareBufferSize in media.
  static int HardwareBufferSize() {
    return static_cast<int>(media::GetAudioHardwareBufferSize());
  }

  static StreamType* CreateStream(AudioManager* audio_manager,
      const AudioParameters& params) {
    return audio_manager->MakeAudioInputStream(params,
      AudioManagerBase::kDefaultDeviceId);
  }
};

class AudioOutputStreamTraits {
 public:
  typedef AudioOutputStream StreamType;

  static int HardwareSampleRate() {
    return static_cast<int>(media::GetAudioHardwareSampleRate());
  }

  static int HardwareBufferSize() {
    return static_cast<int>(media::GetAudioHardwareBufferSize());
  }

  static StreamType* CreateStream(AudioManager* audio_manager,
      const AudioParameters& params) {
    return audio_manager->MakeAudioOutputStream(params);
  }
};

// Traits template holding a trait of StreamType. It encapsulates
// AudioInputStream and AudioOutputStream stream types.
template <typename StreamTraits>
class StreamWrapper {
 public:
  typedef typename StreamTraits::StreamType StreamType;

  explicit StreamWrapper(AudioManager* audio_manager)
      :
#if defined(OS_WIN)
        com_init_(base::win::ScopedCOMInitializer::kMTA),
#endif
        audio_manager_(audio_manager),
        format_(AudioParameters::AUDIO_PCM_LOW_LATENCY),
#if defined(OS_ANDROID)
        channel_layout_(CHANNEL_LAYOUT_MONO),
#else
        channel_layout_(CHANNEL_LAYOUT_STEREO),
#endif
        bits_per_sample_(16) {
    // Use the preferred sample rate.
    sample_rate_ = StreamTraits::HardwareSampleRate();

    // Use the preferred buffer size. Note that the input side uses the same
    // size as the output side in this implementation.
    samples_per_packet_ = StreamTraits::HardwareBufferSize();
  }

  virtual ~StreamWrapper() {}

  // Creates an Audio[Input|Output]Stream stream object using default
  // parameters.
  StreamType* Create() {
    return CreateStream();
  }

  int channels() const {
    return ChannelLayoutToChannelCount(channel_layout_);
  }
  int bits_per_sample() const { return bits_per_sample_; }
  int sample_rate() const { return sample_rate_; }
  int samples_per_packet() const { return samples_per_packet_; }

 private:
  StreamType* CreateStream() {
    StreamType* stream = StreamTraits::CreateStream(audio_manager_,
        AudioParameters(format_, channel_layout_, sample_rate_,
            bits_per_sample_, samples_per_packet_));
    EXPECT_TRUE(stream);
    return stream;
  }

#if defined(OS_WIN)
  base::win::ScopedCOMInitializer com_init_;
#endif

  AudioManager* audio_manager_;
  AudioParameters::Format format_;
  ChannelLayout channel_layout_;
  int bits_per_sample_;
  int sample_rate_;
  int samples_per_packet_;
};

typedef StreamWrapper<AudioInputStreamTraits> AudioInputStreamWrapper;
typedef StreamWrapper<AudioOutputStreamTraits> AudioOutputStreamWrapper;

// This test is intended for manual tests and should only be enabled
// when it is required to make a real-time test of audio in full duplex and
// at the same time create a text file which contains measured delay values.
// The file can later be analyzed off line using e.g. MATLAB.
// MATLAB example:
//   D=load('audio_delay_values_ms.txt');
//   x=cumsum(D(:,1));
//   plot(x, D(:,2), x, D(:,3), x, D(:,4), x, D(:,2)+D(:,3)+D(:,4));
//   axis([0, max(x), 0, max(D(:,2)+D(:,3)+D(:,4))+10]);
//   legend('buffer delay','input delay','output delay','total delay');
//   xlabel('time [msec]')
//   ylabel('delay [msec]')
//   title('Full-duplex audio delay measurement');
TEST_F(AudioLowLatencyInputOutputTest, DISABLED_FullDuplexDelayMeasurement) {
  if (!CanRunAudioTests())
    return;

  AudioInputStreamWrapper aisw(audio_manager());
  AudioInputStream* ais = aisw.Create();
  EXPECT_TRUE(ais);

  AudioOutputStreamWrapper aosw(audio_manager());
  AudioOutputStream* aos = aosw.Create();
  EXPECT_TRUE(aos);

  // This test only supports identical parameters in both directions.
  // TODO(henrika): it is possible to cut delay here by using different
  // buffer sizes for input and output.
  if (aisw.sample_rate() != aosw.sample_rate() ||
      aisw.samples_per_packet() != aosw.samples_per_packet() ||
      aisw.channels()!= aosw.channels() ||
      aisw.bits_per_sample() != aosw.bits_per_sample()) {
    LOG(ERROR) << "This test requires symmetric input and output parameters. "
        "Ensure that sample rate and number of channels are identical in "
        "both directions";
    aos->Close();
    ais->Close();
    return;
  }

  EXPECT_TRUE(ais->Open());
  EXPECT_TRUE(aos->Open());

  FullDuplexAudioSinkSource full_duplex(
      aisw.sample_rate(), aisw.samples_per_packet(), aisw.channels());

  LOG(INFO) << ">> You should now be able to hear yourself in loopback...";
  DLOG(INFO) << "   sample_rate       : " << aisw.sample_rate();
  DLOG(INFO) << "   samples_per_packet: " << aisw.samples_per_packet();
  DLOG(INFO) << "   channels          : " << aisw.channels();

  ais->Start(&full_duplex);
  aos->Start(&full_duplex);

  // Wait for approximately 10 seconds. The user shall hear his own voice
  // in loop back during this time. At the same time, delay recordings are
  // performed and stored in the output text file.
  message_loop()->PostDelayedTask(FROM_HERE,
      MessageLoop::QuitClosure(), TestTimeouts::action_timeout());
  message_loop()->Run();

  aos->Stop();
  ais->Stop();

  // All Close() operations that run on the mocked audio thread,
  // should be synchronous and not post additional close tasks to
  // mocked the audio thread. Hence, there is no need to call
  // message_loop()->RunUntilIdle() after the Close() methods.
  aos->Close();
  ais->Close();
}

}  // namespace media
