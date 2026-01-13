#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

// Forward declarations for audio libraries
// You'll need to include the actual headers based on your setup:
// #include <portaudio.h>
// #include <opus/opus.h>

namespace engine::voice {

/**
 * @brief Configuration for the voice chat system
 */
struct VoiceConfig {
    // Audio capture settings
    int sampleRate = 48000;     // Opus native sample rate
    int framesPerBuffer = 960;  // 20ms at 48kHz
    int channels = 1;           // Mono
    int bitsPerSample = 16;     // 16-bit PCM

    // Opus encoder settings
    int opusBitrate = 24000;  // 24 kbps - good for voice
    int opusComplexity = 5;   // 0-10, higher = better quality but more CPU

    // Network settings
    uint16_t voiceUdpPort = 4243;  // Separate UDP port for voice
    int maxPacketSize = 1400;      // MTU-safe packet size
};

/**
 * @brief Represents a voice packet ready for network transmission
 */
struct VoicePacket {
    uint32_t senderId;
    uint32_t sequenceNumber;
    uint32_t timestamp;
    std::vector<uint8_t> encodedData;
};

/**
 * @brief Manages voice chat lifecycle with thread-safe start/stop
 *
 * This class handles:
 * - Audio capture from microphone
 * - Opus encoding/decoding
 * - UDP voice packet transmission/reception
 * - Thread-safe activation/deactivation
 *
 * Usage:
 *   VoiceManager voiceManager;
 *   voiceManager.setConfig(config);
 *   voiceManager.setSendCallback([&](const VoicePacket& p) { networkSend(p); });
 *   voiceManager.start();  // Call when entering lobby
 *   // ...
 *   voiceManager.stop();   // Call when leaving lobby
 */
class VoiceManager {
   public:
    enum class State { STOPPED, STARTING, RUNNING, STOPPING };

    using SendCallback = std::function<void(const VoicePacket&)>;
    using StateChangeCallback = std::function<void(State)>;

    VoiceManager();
    ~VoiceManager();

    // Disable copy
    VoiceManager(const VoiceManager&) = delete;
    VoiceManager& operator=(const VoiceManager&) = delete;

    /**
     * @brief Configure the voice manager (must be called before start)
     */
    void setConfig(const VoiceConfig& config);

    /**
     * @brief Set callback for sending voice packets over the network
     */
    void setSendCallback(SendCallback callback);

    /**
     * @brief Set callback for state changes (useful for UI updates)
     */
    void setStateChangeCallback(StateChangeCallback callback);

    /**
     * @brief Start voice capture and processing (non-blocking)
     *
     * Creates the audio threads and begins capturing/sending voice.
     * Safe to call from any thread. If already running, does nothing.
     *
     * @return true if started successfully, false if failed
     */
    bool start();

    /**
     * @brief Stop voice capture and processing (non-blocking initiation)
     *
     * Signals threads to stop and joins them asynchronously.
     * Safe to call from any thread. If already stopped, does nothing.
     * The actual cleanup happens on a background thread.
     */
    void stop();

    /**
     * @brief Synchronous stop - blocks until fully stopped
     *
     * Use this when you need to guarantee voice is stopped before proceeding.
     */
    void stopAndWait();

    /**
     * @brief Check if voice is currently active
     */
    bool isRunning() const;

    /**
     * @brief Get current state
     */
    State getState() const;

    /**
     * @brief Process an incoming voice packet from the network
     *
     * Thread-safe. Queues the packet for playback processing.
     *
     * @param packet The received voice packet
     */
    void receivePacket(const VoicePacket& packet);

    /**
     * @brief Set local player ID for packet identification
     */
    void setLocalPlayerId(uint32_t id);

    /**
     * @brief Mute/unmute local microphone
     */
    void setMuted(bool muted);
    bool isMuted() const;

    /**
     * @brief Set input volume (0.0 to 1.0)
     */
    void setInputVolume(float volume);

    /**
     * @brief Set output volume (0.0 to 1.0)
     */
    void setOutputVolume(float volume);

   private:
    // Thread functions
    void captureThreadFunc();
    void playbackThreadFunc();
    void cleanupThread();

    // Audio processing
    bool initializeAudio();
    void shutdownAudio();
    bool initializeOpus();
    void shutdownOpus();

    // State notification
    void notifyStateChange(State newState);

    // Encoding/Decoding
    std::vector<uint8_t> encodeAudio(const int16_t* samples, size_t count);
    std::vector<int16_t> decodeAudio(const std::vector<uint8_t>& data, uint32_t senderId);

    // Configuration
    VoiceConfig _config;

    // State management
    std::atomic<State> _state{State::STOPPED};
    std::atomic<bool> _shouldRun{false};
    std::atomic<bool> _muted{false};
    std::atomic<float> _inputVolume{1.0f};
    std::atomic<float> _outputVolume{1.0f};
    std::atomic<uint32_t> _localPlayerId{0};
    std::atomic<uint32_t> _sequenceNumber{0};
    std::atomic<bool> _isTalking{false};

    // Threads
    std::unique_ptr<std::thread> _captureThread;
    std::unique_ptr<std::thread> _playbackThread;
    std::unique_ptr<std::thread> _cleanupThread;

    // Thread synchronization
    std::mutex _stateMutex;
    std::condition_variable _stateCV;

    // Incoming packet queue (thread-safe)
    std::queue<VoicePacket> _incomingPackets;
    std::mutex _incomingMutex;
    std::condition_variable _incomingCV;

    // Callbacks
    SendCallback _sendCallback;
    StateChangeCallback _stateChangeCallback;
    std::mutex _callbackMutex;

    // PortAudio handles (opaque pointers)
    void* _inputStream = nullptr;
    void* _outputStream = nullptr;

    // Opus handles (opaque pointers)
    void* _encoder = nullptr;
    void* _decoder = nullptr;

    // Per-sender decoders for multi-user voice
    std::unordered_map<uint32_t, void*> _decoders;
    std::mutex _decodersMutex;
};

}  // namespace engine::voice
