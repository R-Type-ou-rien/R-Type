#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "../NetworkRType.hpp"

class VoiceChat {
   public:
    VoiceChat();
    ~VoiceChat();

    void Start(uint32_t player_id, uint32_t lobby_id);
    void Stop();

    void SetSendCallback(std::function<void(const voice_packet&)> callback);
    void ReceivePacket(const voice_packet& packet);

    void SetMuted(bool muted);
    bool IsMuted() const;

   private:
    void CaptureLoop();
    void PlaybackLoop();

    // PortAudio and Opus private implementation hidden in .cpp usually
    // But for simplicity in this assistant flow without PIMPL ptr (unless requested),
    // I will include necessary handles as void* or opaque types if possible,
    // OR just put the logic in .cpp and keep members here.
    // Since I can't easily forward declare PA structs without includes, I'll use void* for handles
    // or include headers here if I assume they are available system-wide.
    // To be safe and clean, I'll use pointers for the PA/Opus states.

    void* _stream;   // PaStream*
    void* _encoder;  // OpusEncoder*
    void* _decoder;  // OpusDecoder*

    std::atomic<bool> _running{false};
    std::atomic<bool> _muted{false};
    uint32_t _player_id = 0;
    uint32_t _lobby_id = 0;
    uint32_t _sequence_number = 0;

    std::thread _captureThread;
    std::thread _playbackThread;

    std::mutex _playbackMutex;
    std::queue<voice_packet> _playbackQueue;

    std::function<void(const voice_packet&)> _sendCallback;

    // Config
    const int SAMPLE_RATE = 48000;
    const int FRAMES_PER_BUFFER = 480;  // 10ms
    const int CHANNELS = 1;
};
