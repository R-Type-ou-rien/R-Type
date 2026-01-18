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

namespace engine::voice {

struct VoiceConfig {
    // Audio capture settings
    int sampleRate = 48000;     // Opus native sample rate
    int framesPerBuffer = 960;  // 20ms a 48kHz
    int channels = 1;           // Mono
    int bitsPerSample = 16;

    // Opus encoder settings
    int opusBitrate = 24000;  // 24 kbps
    int opusComplexity = 5;

    // Network settings
    uint16_t voiceUdpPort = 4243;  // Separate UDP port for voice
    int maxPacketSize = 1400;
};

struct VoicePacket {
    uint32_t senderId;
    uint32_t sequenceNumber;
    uint32_t timestamp;
    std::vector<uint8_t> encodedData;
};

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

    void setConfig(const VoiceConfig& config);

    void setSendCallback(SendCallback callback);

    void setStateChangeCallback(StateChangeCallback callback);

    bool start();
    void stop();

    void stopAndWait();

    bool isRunning() const;

    State getState() const;

    void receivePacket(const VoicePacket& packet);

    void setLocalPlayerId(uint32_t id);

    void setMuted(bool muted);
    bool isMuted() const;

    void setInputVolume(float volume);

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
    std::atomic<float> _inputVolume{0.35f};
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
