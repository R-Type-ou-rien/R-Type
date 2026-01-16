#include "VoiceManager.hpp"

#include <algorithm>
#include <map>
#include <chrono>
#include <cstring>
#include <iostream>

#ifdef RTYPE_USE_REAL_AUDIO
#include <portaudio.h>
#include <opus/opus.h>
#endif

namespace engine::voice {

// ============================================================================
// AUDIO BACKEND - Real or Stub based on RTYPE_USE_REAL_AUDIO
// ============================================================================

#ifdef RTYPE_USE_REAL_AUDIO

// Ring buffer for lock-free audio data transfer
template <typename T, size_t Capacity>
class RingBuffer {
   public:
    bool push(const T* data, size_t count) {
        size_t writePos = _writePos.load(std::memory_order_relaxed);
        size_t readPos = _readPos.load(std::memory_order_acquire);

        size_t available = Capacity - (writePos - readPos);
        if (count > available)
            return false;

        for (size_t i = 0; i < count; ++i) {
            _buffer[(writePos + i) % Capacity] = data[i];
        }
        _writePos.store(writePos + count, std::memory_order_release);
        return true;
    }

    size_t pop(T* data, size_t maxCount) {
        size_t writePos = _writePos.load(std::memory_order_acquire);
        size_t readPos = _readPos.load(std::memory_order_relaxed);

        size_t available = writePos - readPos;
        size_t toPop = std::min(available, maxCount);

        for (size_t i = 0; i < toPop; ++i) {
            data[i] = _buffer[(readPos + i) % Capacity];
        }
        _readPos.store(readPos + toPop, std::memory_order_release);
        return toPop;
    }

    size_t available() const {
        return _writePos.load(std::memory_order_acquire) - _readPos.load(std::memory_order_relaxed);
    }

    void clear() {
        _readPos.store(0, std::memory_order_relaxed);
        _writePos.store(0, std::memory_order_relaxed);
    }

   private:
    std::array<T, Capacity> _buffer;
    std::atomic<size_t> _readPos{0};
    std::atomic<size_t> _writePos{0};
};

// Callback-based audio stream context
struct AudioStreamContext {
    RingBuffer<int16_t, 65536>* captureBuffer = nullptr;  // Increased buffer size (~1.3s at 48k)
    RingBuffer<int16_t, 65536>* playbackBuffer = nullptr;
    std::atomic<bool>* running = nullptr;
    int channels = 1;
};

// PortAudio input callback - called from audio thread
static int inputCallback(const void* inputBuffer, void* /*outputBuffer*/, unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/,
                         void* userData) {
    auto* ctx = static_cast<AudioStreamContext*>(userData);
    if (!ctx || !ctx->captureBuffer || !ctx->running || !ctx->running->load()) {
        return paComplete;
    }

    const int16_t* input = static_cast<const int16_t*>(inputBuffer);
    if (input) {
        ctx->captureBuffer->push(input, framesPerBuffer * ctx->channels);
    }

    return paContinue;
}

// PortAudio output callback - called from audio thread
static int outputCallback(const void* /*inputBuffer*/, void* outputBuffer, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* /*timeInfo*/, PaStreamCallbackFlags /*statusFlags*/,
                          void* userData) {
    auto* ctx = static_cast<AudioStreamContext*>(userData);
    int16_t* output = static_cast<int16_t*>(outputBuffer);

    if (!ctx || !ctx->playbackBuffer || !ctx->running || !ctx->running->load() || !output) {
        // Fill with silence
        if (output) {
            std::memset(output, 0, framesPerBuffer * ctx->channels * sizeof(int16_t));
        }
        return paContinue;
    }

    size_t needed = framesPerBuffer * ctx->channels;
    size_t got = ctx->playbackBuffer->pop(output, needed);

    // Fill remaining with silence
    if (got < needed) {
        std::memset(output + got, 0, (needed - got) * sizeof(int16_t));
    }

    return paContinue;
}

// Real PortAudio/Opus implementation with callbacks
namespace audio_backend {

static bool g_paInitialized = false;
static AudioStreamContext g_inputContext;
static AudioStreamContext g_outputContext;
static RingBuffer<int16_t, 65536> g_captureRing;   // 64k samples
static RingBuffer<int16_t, 65536> g_playbackRing;  // 64k samples
static std::atomic<bool> g_streamRunning{false};

bool Pa_Init() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "[VoiceManager] PortAudio init failed: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    g_paInitialized = true;
    g_captureRing.clear();
    g_playbackRing.clear();
    std::cout << "[VoiceManager] PortAudio initialized" << std::endl;
    return true;
}

void Pa_Term() {
    if (g_paInitialized) {
        g_streamRunning.store(false);
        Pa_Terminate();
        g_paInitialized = false;
        std::cout << "[VoiceManager] PortAudio terminated" << std::endl;
    }
}

PaStream* Pa_OpenInputStream(int sampleRate, int channels, int framesPerBuffer) {
    PaStream* stream = nullptr;
    PaStreamParameters params;
    params.device = Pa_GetDefaultInputDevice();
    if (params.device == paNoDevice) {
        std::cerr << "[VoiceManager] No input device found" << std::endl;
        return nullptr;
    }

    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(params.device);
    if (!deviceInfo) {
        std::cerr << "[VoiceManager] Cannot get input device info" << std::endl;
        return nullptr;
    }
    std::cout << "[VoiceManager] Input device: " << deviceInfo->name
              << " (max channels: " << deviceInfo->maxInputChannels << ")" << std::endl;

    params.channelCount = channels;
    params.sampleFormat = paInt16;
    params.suggestedLatency = deviceInfo->defaultHighInputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    // Setup context for callback
    g_inputContext.captureBuffer = &g_captureRing;
    g_inputContext.playbackBuffer = nullptr;
    g_inputContext.running = &g_streamRunning;
    g_inputContext.channels = channels;

    // Open stream with callback
    PaError err =
        Pa_OpenStream(&stream, &params, nullptr, sampleRate, framesPerBuffer, 0, inputCallback, &g_inputContext);
    if (err != paNoError) {
        std::cerr << "[VoiceManager] Failed to open input: " << Pa_GetErrorText(err) << std::endl;
        return nullptr;
    }
    std::cout << "[VoiceManager] Input stream opened (callback mode)" << std::endl;
    return stream;
}

PaStream* Pa_OpenOutputStream(int sampleRate, int channels, int framesPerBuffer) {
    PaStream* stream = nullptr;
    PaStreamParameters params;
    params.device = Pa_GetDefaultOutputDevice();
    if (params.device == paNoDevice) {
        std::cerr << "[VoiceManager] No output device found" << std::endl;
        return nullptr;
    }

    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(params.device);
    if (!deviceInfo) {
        std::cerr << "[VoiceManager] Cannot get output device info" << std::endl;
        return nullptr;
    }
    std::cout << "[VoiceManager] Output device: " << deviceInfo->name
              << " (max channels: " << deviceInfo->maxOutputChannels << ")" << std::endl;

    params.channelCount = channels;
    params.sampleFormat = paInt16;
    params.suggestedLatency = deviceInfo->defaultHighOutputLatency;
    params.hostApiSpecificStreamInfo = nullptr;

    // Setup context for callback
    g_outputContext.captureBuffer = nullptr;
    g_outputContext.playbackBuffer = &g_playbackRing;
    g_outputContext.running = &g_streamRunning;
    g_outputContext.channels = channels;

    // Open stream with callback
    PaError err =
        Pa_OpenStream(&stream, nullptr, &params, sampleRate, framesPerBuffer, 0, outputCallback, &g_outputContext);
    if (err != paNoError) {
        std::cerr << "[VoiceManager] Failed to open output: " << Pa_GetErrorText(err) << std::endl;
        return nullptr;
    }
    std::cout << "[VoiceManager] Output stream opened (callback mode)" << std::endl;
    return stream;
}

bool StartStream(void* stream) {
    if (!stream)
        return false;
    g_streamRunning.store(true);
    PaError err = ::Pa_StartStream(static_cast<PaStream*>(stream));
    if (err != paNoError) {
        std::cerr << "[VoiceManager] Failed to start stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    std::cout << "[VoiceManager] Stream started" << std::endl;
    return true;
}

void CloseStream(void* stream) {
    if (stream) {
        g_streamRunning.store(false);
        ::Pa_StopStream(static_cast<PaStream*>(stream));
        ::Pa_CloseStream(static_cast<PaStream*>(stream));
    }
}

// Non-blocking read from capture ring buffer
size_t ReadStream(void* /*stream*/, int16_t* buffer, size_t frames) {
    return g_captureRing.pop(buffer, frames);
}

// Non-blocking write to playback ring buffer
bool WriteStream(void* /*stream*/, const int16_t* buffer, size_t frames) {
    return g_playbackRing.push(buffer, frames);
}

OpusEncoder* Opus_CreateEncoder(int sampleRate, int channels, int application) {
    int error;
    OpusEncoder* encoder = opus_encoder_create(sampleRate, channels, application, &error);
    if (error != OPUS_OK) {
        std::cerr << "[VoiceManager] Opus encoder error: " << opus_strerror(error) << std::endl;
        return nullptr;
    }
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(28000));  // Slightly higher bitrate
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(5));   // Better quality encoding
    opus_encoder_ctl(encoder, OPUS_SET_VBR(1));          // Variable Bit Rate (better quality)
    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));  // Cut freqs > 8kHz (Kills screeching)
    // opus_encoder_ctl(encoder, OPUS_SET_LSB_DEPTH, 16); // Removed unsupported macro
    std::cout << "[VoiceManager] Opus encoder created" << std::endl;
    return encoder;
}

OpusDecoder* Opus_CreateDecoder(int sampleRate, int channels) {
    int error;
    OpusDecoder* decoder = opus_decoder_create(sampleRate, channels, &error);
    if (error != OPUS_OK) {
        std::cerr << "[VoiceManager] Opus decoder error: " << opus_strerror(error) << std::endl;
        return nullptr;
    }
    // Set 16-bit depth on decoder too
    // Note: opus_decoder_ctl doesn't support OPUS_SET_LSB_DEPTH generally, but we can set gain/etc if needed.
    return decoder;
}

void Opus_DestroyEncoder(void* encoder) {
    if (encoder)
        opus_encoder_destroy(static_cast<OpusEncoder*>(encoder));
}

void Opus_DestroyDecoder(void* decoder) {
    if (decoder)
        opus_decoder_destroy(static_cast<OpusDecoder*>(decoder));
}

std::vector<uint8_t> Opus_Encode(void* encoder, const int16_t* samples, size_t frameSize) {
    if (!encoder)
        return {};
    std::vector<uint8_t> output(4000);
    int encodedBytes =
        opus_encode(static_cast<OpusEncoder*>(encoder), samples, frameSize, output.data(), output.size());
    if (encodedBytes < 0) {
        std::cerr << "[VoiceManager] Encode error: " << opus_strerror(encodedBytes) << std::endl;
        return {};
    }
    output.resize(encodedBytes);
    return output;
}

std::vector<int16_t> Opus_Decode(void* decoder, const std::vector<uint8_t>& data, size_t frameSize, int channels) {
    if (!decoder)
        return {};
    std::vector<int16_t> output(frameSize * channels);
    int decoded =
        opus_decode(static_cast<OpusDecoder*>(decoder), data.data(), data.size(), output.data(), frameSize, 0);
    if (decoded < 0) {
        std::cerr << "[VoiceManager] Decode error: " << opus_strerror(decoded) << std::endl;
        return {};
    }
    output.resize(decoded * channels);
    return output;
}

}  // namespace audio_backend

#else

// Stub implementation for testing without real audio
namespace audio_backend {

static bool g_paInitialized = false;

bool Pa_Init() {
    g_paInitialized = true;
    std::cout << "[VoiceManager] Audio subsystem initialized (stub)" << std::endl;
    return true;
}

void Pa_Term() {
    g_paInitialized = false;
    std::cout << "[VoiceManager] Audio subsystem terminated (stub)" << std::endl;
}

void* Pa_OpenInputStream(int sampleRate, int channels, int framesPerBuffer) {
    (void)sampleRate;
    (void)channels;
    (void)framesPerBuffer;
    return reinterpret_cast<void*>(0x1);
}

void* Pa_OpenOutputStream(int sampleRate, int channels, int framesPerBuffer) {
    (void)sampleRate;
    (void)channels;
    (void)framesPerBuffer;
    return reinterpret_cast<void*>(0x2);
}

bool StartStream(void* stream) {
    (void)stream;
    return true;
}

void CloseStream(void* stream) {
    (void)stream;
}

size_t ReadStream(void* stream, int16_t* buffer, size_t frames) {
    (void)stream;
    std::memset(buffer, 0, frames * sizeof(int16_t));
    return frames;
}

bool WriteStream(void* stream, const int16_t* buffer, size_t frames) {
    (void)stream;
    (void)buffer;
    (void)frames;
    return true;
}

void* Opus_CreateEncoder(int sampleRate, int channels, int application) {
    (void)sampleRate;
    (void)channels;
    (void)application;
    std::cout << "[VoiceManager] Opus encoder created (stub)" << std::endl;
    return reinterpret_cast<void*>(0x3);
}

void* Opus_CreateDecoder(int sampleRate, int channels) {
    (void)sampleRate;
    (void)channels;
    return reinterpret_cast<void*>(0x4);
}

void Opus_DestroyEncoder(void* encoder) {
    (void)encoder;
}
void Opus_DestroyDecoder(void* decoder) {
    (void)decoder;
}

std::vector<uint8_t> Opus_Encode(void* encoder, const int16_t* samples, size_t frameSize) {
    (void)encoder;
    std::vector<uint8_t> encoded(frameSize * 2);
    std::memcpy(encoded.data(), samples, frameSize * 2);
    return encoded;
}

std::vector<int16_t> Opus_Decode(void* decoder, const std::vector<uint8_t>& data, size_t frameSize, int channels) {
    (void)decoder;
    std::vector<int16_t> decoded(frameSize * channels);
    size_t copySize = std::min(data.size(), frameSize * channels * 2);
    std::memcpy(decoded.data(), data.data(), copySize);
    return decoded;
}

}  // namespace audio_backend

#endif  // RTYPE_USE_REAL_AUDIO

// ============================================================================
// VoiceManager Implementation
// ============================================================================

VoiceManager::VoiceManager() {
    std::cout << "[VoiceManager] Created" << std::endl;
}

VoiceManager::~VoiceManager() {
    stopAndWait();
    std::cout << "[VoiceManager] Destroyed" << std::endl;
}

void VoiceManager::setConfig(const VoiceConfig& config) {
    if (_state.load() != State::STOPPED) {
        std::cerr << "[VoiceManager] Cannot change config while running" << std::endl;
        return;
    }
    _config = config;
}

void VoiceManager::setSendCallback(SendCallback callback) {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _sendCallback = std::move(callback);
}

void VoiceManager::setStateChangeCallback(StateChangeCallback callback) {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    _stateChangeCallback = std::move(callback);
}

bool VoiceManager::start() {
    std::lock_guard<std::mutex> lock(_stateMutex);

    // Already running or starting
    if (_state.load() != State::STOPPED) {
        std::cout << "[VoiceManager] Already running or transitioning" << std::endl;
        return _state.load() == State::RUNNING;
    }

    std::cout << "[VoiceManager] Starting voice chat..." << std::endl;

    // Transition to STARTING state
    _state.store(State::STARTING);
    notifyStateChange(State::STARTING);

    // Initialize audio subsystems
    if (!initializeAudio()) {
        std::cerr << "[VoiceManager] Failed to initialize audio" << std::endl;
        _state.store(State::STOPPED);
        notifyStateChange(State::STOPPED);
        return false;
    }

    if (!initializeOpus()) {
        std::cerr << "[VoiceManager] Failed to initialize Opus" << std::endl;
        shutdownAudio();
        _state.store(State::STOPPED);
        notifyStateChange(State::STOPPED);
        return false;
    }

    // Start threads
    _shouldRun.store(true);
    _sequenceNumber.store(0);

    _captureThread = std::make_unique<std::thread>(&VoiceManager::captureThreadFunc, this);
    _playbackThread = std::make_unique<std::thread>(&VoiceManager::playbackThreadFunc, this);

    _state.store(State::RUNNING);
    notifyStateChange(State::RUNNING);
    _stateCV.notify_all();

    std::cout << "[VoiceManager] Voice chat started successfully" << std::endl;
    return true;
}

void VoiceManager::stop() {
    // Non-blocking stop - spawn cleanup thread
    State expected = State::RUNNING;
    if (!_state.compare_exchange_strong(expected, State::STOPPING)) {
        // Not running or already stopping
        return;
    }

    std::cout << "[VoiceManager] Stopping voice chat..." << std::endl;
    notifyStateChange(State::STOPPING);

    // Signal threads to stop
    _shouldRun.store(false);
    _incomingCV.notify_all();

    // Spawn cleanup thread to join worker threads without blocking
    if (_cleanupThread && _cleanupThread->joinable()) {
        _cleanupThread->join();
    }
    _cleanupThread = std::make_unique<std::thread>(&VoiceManager::cleanupThread, this);
}

void VoiceManager::stopAndWait() {
    stop();

    // Wait for cleanup to complete
    std::unique_lock<std::mutex> lock(_stateMutex);
    _stateCV.wait(lock, [this] { return _state.load() == State::STOPPED; });

    // Join cleanup thread
    if (_cleanupThread && _cleanupThread->joinable()) {
        _cleanupThread->join();
    }
}

bool VoiceManager::isRunning() const {
    return _state.load() == State::RUNNING;
}

VoiceManager::State VoiceManager::getState() const {
    return _state.load();
}

void VoiceManager::receivePacket(const VoicePacket& packet) {
    if (_state.load() != State::RUNNING) {
        std::cout << "[VoiceManager] receivePacket: state is not RUNNING" << std::endl;
        return;
    }

    // Don't play back our own voice
    uint32_t localId = _localPlayerId.load();
    if (packet.senderId == localId) {
        static uint32_t selfPacketsIgnored = 0;
        selfPacketsIgnored++;
        if (selfPacketsIgnored % 50 == 1) {
            std::cout << "[VoiceManager] Ignoring own packet (senderId=" << packet.senderId << ", localId=" << localId
                      << ")" << std::endl;
        }
        return;
    }

    static uint32_t packetsReceived = 0;
    packetsReceived++;
    if (packetsReceived % 50 == 1) {
        std::cout << "[VoiceManager] Received " << packetsReceived << " voice packets from player " << packet.senderId
                  << " (localId=" << localId << ", dataSize=" << packet.encodedData.size() << ")" << std::endl;
    }

    {
        std::lock_guard<std::mutex> lock(_incomingMutex);
        // Limit queue size to prevent memory issues
        if (_incomingPackets.size() < 100) {
            _incomingPackets.push(packet);
        }
    }
    _incomingCV.notify_one();
}

void VoiceManager::setLocalPlayerId(uint32_t id) {
    std::cout << "[VoiceManager] Setting local player ID to " << id << std::endl;
    _localPlayerId.store(id);
}

void VoiceManager::setMuted(bool muted) {
    _muted.store(muted);
    std::cout << "[VoiceManager] Microphone " << (muted ? "muted" : "unmuted") << std::endl;
}

bool VoiceManager::isMuted() const {
    return _muted.load();
}

void VoiceManager::setInputVolume(float volume) {
    _inputVolume.store(std::clamp(volume, 0.0f, 1.0f));
}

void VoiceManager::setOutputVolume(float volume) {
    _outputVolume.store(std::clamp(volume, 0.0f, 1.0f));
}

// ============================================================================
// Private Methods
// ============================================================================

void VoiceManager::notifyStateChange(State newState) {
    std::lock_guard<std::mutex> lock(_callbackMutex);
    if (_stateChangeCallback) {
        _stateChangeCallback(newState);
    }
}

void VoiceManager::captureThreadFunc() {
    std::cout << "[VoiceManager] Capture thread started" << std::endl;

    const size_t bufferSize = static_cast<size_t>(_config.framesPerBuffer) * _config.channels;
    std::vector<int16_t> captureBuffer(bufferSize);
    const auto frameInterval = std::chrono::milliseconds(20);  // 20ms per frame
    uint32_t packetsSent = 0;
    uint32_t silentFrames = 0;
    int16_t lastSample = 0;

    while (_shouldRun.load()) {
        auto frameStart = std::chrono::steady_clock::now();

        // Accumulate samples until we have a full frame
        size_t samplesRead = 0;
        size_t samplesNeeded = _config.framesPerBuffer * _config.channels;

        while (samplesRead < samplesNeeded && _shouldRun.load()) {
            size_t n = audio_backend::ReadStream(_inputStream, captureBuffer.data() + samplesRead,
                                                 samplesNeeded - samplesRead);
            samplesRead += n;
            if (samplesRead < samplesNeeded) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        if (samplesRead < samplesNeeded)
            continue;

        // Attenuation is now handled dynamically via _inputVolume below.

        // Check if there's actual audio (not silence)
        int32_t maxSample = 0;
        for (const auto& s : captureBuffer) {
            maxSample = std::max(maxSample, static_cast<int32_t>(std::abs(s)));
        }

        // Lower threshold to detect voice at low levels - Raised from 100 to 800 to filter background noise
        bool talking = (maxSample > 800);
        _isTalking.store(talking);

        // Skip if muted
        if (!_muted.load() && talking) {  // Only send if there's actual audio
            // Apply input volume
            float volume = _inputVolume.load();
            if (volume < 1.0f) {
                for (auto& sample : captureBuffer) {
                    sample = static_cast<int16_t>(sample * volume);
                }
            }

            // Encode and send (pass frame count, not sample count)
            auto encoded = encodeAudio(captureBuffer.data(), _config.framesPerBuffer);
            if (!encoded.empty()) {
                VoicePacket packet;
                packet.senderId = _localPlayerId.load();
                packet.sequenceNumber = _sequenceNumber.fetch_add(1);
                packet.timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                             std::chrono::steady_clock::now().time_since_epoch())
                                                             .count());
                packet.encodedData = std::move(encoded);

                // Send via callback
                {
                    std::lock_guard<std::mutex> lock(_callbackMutex);
                    if (_sendCallback) {
                        _sendCallback(packet);
                        packetsSent++;
                        if (packetsSent % 50 == 1) {  // Log every 50 packets (~1 second)
                            std::cout << "[VoiceManager] Sent " << packetsSent
                                      << " voice packets (audio level: " << maxSample
                                      << ", encoded size: " << packet.encodedData.size() << " bytes)" << std::endl;
                        }
                    } else {
                        if (packetsSent == 0) {
                            std::cerr << "[VoiceManager] WARNING: No send callback set!" << std::endl;
                        }
                    }
                }
            }
        }

        // Maintain frame timing
        auto elapsed = std::chrono::steady_clock::now() - frameStart;
        if (elapsed < frameInterval) {
            std::this_thread::sleep_for(frameInterval - elapsed);
        }
    }

    std::cout << "[VoiceManager] Capture thread stopped" << std::endl;
}

void VoiceManager::playbackThreadFunc() {
    std::cout << "[VoiceManager] Playback thread started" << std::endl;
    uint32_t packetsPlayed = 0;

    // Jitter buffer: accumulate packets before starting playback
    // Using a dynamic buffer
    std::mutex jitterMutex;
    std::map<uint32_t, std::queue<VoicePacket>> playerJitterBuffers;
    const size_t TARGET_JITTER_SIZE = 4;  // Start playing after 4 packets (~80ms)

    // Keep track of which players we are currently "playing" (have buffered enough)
    std::unordered_map<uint32_t, bool> isPlaying;

    while (_shouldRun.load()) {
        VoicePacket packet;
        bool hasPacket = false;

        // 1. Fetch incoming packet from network queue
        {
            std::unique_lock<std::mutex> lock(_incomingMutex);
            if (_incomingCV.wait_for(lock, std::chrono::milliseconds(5),
                                     [this] { return !_incomingPackets.empty() || !_shouldRun.load(); })) {
                if (!_shouldRun.load() && _incomingPackets.empty())
                    break;

                if (!_incomingPackets.empty()) {
                    packet = std::move(_incomingPackets.front());
                    _incomingPackets.pop();
                    hasPacket = true;
                }
            }
        }

        // 2. Add to player-specific jitter buffer
        if (hasPacket) {
            auto& queue = playerJitterBuffers[packet.senderId];

            // Limit queue size to prevent latency (max 10 packets = 200ms)
            while (queue.size() >= 10) {
                queue.pop();
            }
            queue.push(packet);

            // If we have enough packets, mark as playing
            if (!isPlaying[packet.senderId] && queue.size() >= TARGET_JITTER_SIZE) {
                isPlaying[packet.senderId] = true;
                std::cout << "[VoiceManager] Started playback for player " << packet.senderId << std::endl;
            }
        }

        // 3. Process playback for all active players
        // We mix audio from all players (simple addition with clamping)
        // Since we output mono 48k, we can just mix 960 samples from each player

        std::vector<int16_t> mixedBuffer;
        bool mixed = false;

        for (auto it = playerJitterBuffers.begin(); it != playerJitterBuffers.end();) {
            uint32_t playerId = it->first;
            std::queue<VoicePacket>& queue = it->second;

            if (!isPlaying[playerId]) {
                ++it;
                continue;
            }

            if (queue.empty()) {
                // Buffer underrun - stop playing this user to rebuild buffer
                isPlaying[playerId] = false;
                std::cout << "[VoiceManager] Buffer underrun for player " << playerId << std::endl;
                ++it;
                continue;
            }

            // Get packet
            VoicePacket p = queue.front();
            queue.pop();

            // Decode
            auto decoded = decodeAudio(p.encodedData, p.senderId);
            if (decoded.empty()) {
                ++it;
                continue;
            }

            // Mix
            if (!mixed) {
                mixedBuffer = decoded;  // First source, just copy
                mixed = true;
            } else {
                // Mix additively
                size_t mixLen = std::min(mixedBuffer.size(), decoded.size());
                for (size_t i = 0; i < mixLen; ++i) {
                    int32_t sum = static_cast<int32_t>(mixedBuffer[i]) + static_cast<int32_t>(decoded[i]);
                    mixedBuffer[i] = static_cast<int16_t>(std::clamp(sum, -32768, 32767));
                }
            }
            ++it;
        }

        // 4. Output mixed audio
        if (mixed) {
            // Apply volume & Ducking (Echo reduction)
            float volume = _outputVolume.load();
            if (_isTalking.load()) {
                volume *= 0.2f;  // Reduce volume by 80% when talking
            }

            if (volume < 0.99f) {
                for (auto& sample : mixedBuffer) {
                    sample = static_cast<int16_t>(sample * volume);
                }
            }

            size_t frames = mixedBuffer.size() / _config.channels;
            int retryCount = 0;
            while (!audio_backend::WriteStream(_outputStream, mixedBuffer.data(), frames)) {
                if (!_shouldRun.load())
                    break;
                std::this_thread::sleep_for(std::chrono::microseconds(500));
                retryCount++;
                if (retryCount > 200) {
                    std::cerr << "[VoiceManager] Output buffer full, dropping mixed frame" << std::endl;
                    break;
                }
            }
        } else {
            // If no audio from anyone, we need to prevent CPU spin but no sleep needed if we waited on CV
            // Actually, if we woke up due to timeout but have no audio to play, we loop
        }
    }

    std::cout << "[VoiceManager] Playback thread stopped" << std::endl;
}

void VoiceManager::cleanupThread() {
    std::cout << "[VoiceManager] Cleanup thread started" << std::endl;

    // Join capture thread
    if (_captureThread && _captureThread->joinable()) {
        _captureThread->join();
        _captureThread.reset();
    }

    // Join playback thread
    if (_playbackThread && _playbackThread->joinable()) {
        _playbackThread->join();
        _playbackThread.reset();
    }

    // Cleanup audio resources
    shutdownOpus();
    shutdownAudio();

    // Clear incoming queue
    {
        std::lock_guard<std::mutex> lock(_incomingMutex);
        std::queue<VoicePacket> empty;
        std::swap(_incomingPackets, empty);
    }

    // Transition to STOPPED
    _state.store(State::STOPPED);
    notifyStateChange(State::STOPPED);
    _stateCV.notify_all();

    std::cout << "[VoiceManager] Cleanup complete, voice chat stopped" << std::endl;
}

bool VoiceManager::initializeAudio() {
    // Initialize PortAudio
    if (!audio_backend::Pa_Init()) {
        return false;
    }

    // Open input stream (microphone) - but don't start it yet
    _inputStream = audio_backend::Pa_OpenInputStream(_config.sampleRate, _config.channels, _config.framesPerBuffer);
    if (!_inputStream) {
        std::cerr << "[VoiceManager] Failed to open input stream" << std::endl;
        audio_backend::Pa_Term();
        return false;
    }

    // Open output stream (speakers) - but don't start it yet
    _outputStream = audio_backend::Pa_OpenOutputStream(_config.sampleRate, _config.channels, _config.framesPerBuffer);
    if (!_outputStream) {
        std::cerr << "[VoiceManager] Failed to open output stream" << std::endl;
        audio_backend::CloseStream(_inputStream);
        _inputStream = nullptr;
        audio_backend::Pa_Term();
        return false;
    }

    // Small delay to let JACK settle
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Start output stream first
    if (!audio_backend::StartStream(_outputStream)) {
        std::cerr << "[VoiceManager] Failed to start output stream" << std::endl;
        audio_backend::CloseStream(_inputStream);
        audio_backend::CloseStream(_outputStream);
        _inputStream = nullptr;
        _outputStream = nullptr;
        audio_backend::Pa_Term();
        return false;
    }
    std::cout << "[VoiceManager] Output stream started" << std::endl;

    // Small delay between starting streams
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Start input stream
    if (!audio_backend::StartStream(_inputStream)) {
        std::cerr << "[VoiceManager] Failed to start input stream" << std::endl;
        audio_backend::CloseStream(_inputStream);
        audio_backend::CloseStream(_outputStream);
        _inputStream = nullptr;
        _outputStream = nullptr;
        audio_backend::Pa_Term();
        return false;
    }
    std::cout << "[VoiceManager] Input stream started" << std::endl;

    // Another small delay to ensure streams are stable
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    return true;
}

void VoiceManager::shutdownAudio() {
    if (_inputStream) {
        audio_backend::CloseStream(_inputStream);
        _inputStream = nullptr;
    }
    if (_outputStream) {
        audio_backend::CloseStream(_outputStream);
        _outputStream = nullptr;
    }
    audio_backend::Pa_Term();
}

bool VoiceManager::initializeOpus() {
    // Create encoder (OPUS_APPLICATION_VOIP = 2048)
    _encoder = audio_backend::Opus_CreateEncoder(_config.sampleRate, _config.channels, 2048);
    if (!_encoder) {
        return false;
    }

    // Create local decoder for testing
    _decoder = audio_backend::Opus_CreateDecoder(_config.sampleRate, _config.channels);
    if (!_decoder) {
        audio_backend::Opus_DestroyEncoder(_encoder);
        _encoder = nullptr;
        return false;
    }

    return true;
}

void VoiceManager::shutdownOpus() {
    if (_encoder) {
        audio_backend::Opus_DestroyEncoder(_encoder);
        _encoder = nullptr;
    }
    if (_decoder) {
        audio_backend::Opus_DestroyDecoder(_decoder);
        _decoder = nullptr;
    }

    // Clean up per-sender decoders
    std::lock_guard<std::mutex> lock(_decodersMutex);
    for (auto& [id, decoder] : _decoders) {
        audio_backend::Opus_DestroyDecoder(decoder);
    }
    _decoders.clear();
}

std::vector<uint8_t> VoiceManager::encodeAudio(const int16_t* samples, size_t count) {
    return audio_backend::Opus_Encode(_encoder, samples, count);
}

std::vector<int16_t> VoiceManager::decodeAudio(const std::vector<uint8_t>& data, uint32_t senderId) {
    void* decoder = nullptr;

    // Get or create decoder for this sender
    {
        std::lock_guard<std::mutex> lock(_decodersMutex);
        auto it = _decoders.find(senderId);
        if (it == _decoders.end()) {
            decoder = audio_backend::Opus_CreateDecoder(_config.sampleRate, _config.channels);
            if (decoder) {
                _decoders[senderId] = decoder;
            }
        } else {
            decoder = it->second;
        }
    }

    if (!decoder) {
        return {};
    }

    return audio_backend::Opus_Decode(decoder, data, _config.framesPerBuffer, _config.channels);
}

}  // namespace engine::voice
