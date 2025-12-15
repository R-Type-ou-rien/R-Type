#include "VoiceChat.hpp"
#include <portaudio.h>
#include <opus/opus.h>
#include <iostream>
#include <cstring>

VoiceChat::VoiceChat() : _stream(nullptr), _encoder(nullptr), _decoder(nullptr) {
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }

    // Initialize Opus Encoder
    int error;
    _encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK) {
        std::cerr << "Opus encoder error: " << opus_strerror(error) << std::endl;
    }

    // Initialize Opus Decoder
    _decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        std::cerr << "Opus decoder error: " << opus_strerror(error) << std::endl;
    }
}

VoiceChat::~VoiceChat() {
    Stop();

    if (_stream) {
        Pa_CloseStream((PaStream*)_stream);
    }
    Pa_Terminate();

    if (_encoder) {
        opus_encoder_destroy((OpusEncoder*)_encoder);
    }
    if (_decoder) {
        opus_decoder_destroy((OpusDecoder*)_decoder);
    }
}

void VoiceChat::Start(uint32_t player_id, uint32_t lobby_id) {
    _player_id = player_id;
    _lobby_id = lobby_id;
    _running = true;
    _sequence_number = 0;  // Reset sequence

    // Open PortAudio Stream
    PaError err = Pa_OpenDefaultStream((PaStream**)&_stream,
                                       CHANNELS,   // Input channels
                                       CHANNELS,   // Output channels
                                       paFloat32,  // Sample format
                                       SAMPLE_RATE, FRAMES_PER_BUFFER,
                                       nullptr,  // We use blocking I/O in threads for simplicity here
                                       nullptr);
    if (err != paNoError) {
        std::cerr << "PortAudio open stream error: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    err = Pa_StartStream((PaStream*)_stream);
    if (err != paNoError) {
        std::cerr << "PortAudio start stream error: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    _captureThread = std::thread([this]() { CaptureLoop(); });
    _playbackThread = std::thread([this]() { PlaybackLoop(); });
}

void VoiceChat::Stop() {
    _running = false;

    if (_captureThread.joinable())
        _captureThread.join();
    if (_playbackThread.joinable())
        _playbackThread.join();

    if (_stream) {
        Pa_StopStream((PaStream*)_stream);
    }
}

void VoiceChat::SetSendCallback(std::function<void(const voice_packet&)> callback) {
    _sendCallback = callback;
}

void VoiceChat::ReceivePacket(const voice_packet& packet) {
    std::lock_guard<std::mutex> lock(_playbackMutex);
    _playbackQueue.push(packet);
}

void VoiceChat::SetMuted(bool muted) {
    _muted = muted;
}

bool VoiceChat::IsMuted() const {
    return _muted;
}

void VoiceChat::CaptureLoop() {
    float buffer[FRAMES_PER_BUFFER * CHANNELS];
    unsigned char compressed_data[480];  // Max size matches struct

    while (_running) {
        if (!_stream) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        PaError err = Pa_ReadStream((PaStream*)_stream, buffer, FRAMES_PER_BUFFER);
        if (err != paNoError) {
            // std::cerr << "PA Read Error: " << Pa_GetErrorText(err) << std::endl;
            continue;
        }

        if (!_muted && _sendCallback) {
            int len = opus_encode_float((OpusEncoder*)_encoder, buffer, FRAMES_PER_BUFFER, compressed_data,
                                        sizeof(compressed_data));
            if (len < 0) {
                std::cerr << "Opus encode error: " << opus_strerror(len) << std::endl;
                continue;
            }

            voice_packet packet;
            packet.sender_id = _player_id;
            packet.lobby_id = _lobby_id;
            packet.sequence_number = _sequence_number++;
            packet.data_size = len;
            std::memcpy(packet.audio_data, compressed_data, len);

            _sendCallback(packet);
        }
    }
}

void VoiceChat::PlaybackLoop() {
    float buffer[FRAMES_PER_BUFFER * CHANNELS];

    while (_running) {
        if (!_stream) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        voice_packet packet;
        bool has_packet = false;

        {
            std::lock_guard<std::mutex> lock(_playbackMutex);
            if (!_playbackQueue.empty()) {
                packet = _playbackQueue.front();
                _playbackQueue.pop();
                has_packet = true;
            }
        }

        int frame_size;
        if (has_packet) {
            frame_size = opus_decode_float((OpusDecoder*)_decoder, packet.audio_data, packet.data_size, buffer,
                                           FRAMES_PER_BUFFER, 0);
        } else {
            // PLC (Packet Loss Concealment)
            frame_size = opus_decode_float((OpusDecoder*)_decoder, nullptr, 0, buffer, FRAMES_PER_BUFFER, 0);
        }

        if (frame_size < 0) {
            std::cerr << "Opus decode error: " << opus_strerror(frame_size) << std::endl;
            continue;
        }

        PaError err = Pa_WriteStream((PaStream*)_stream, buffer, FRAMES_PER_BUFFER);
        if (err != paNoError && err != paOutputUnderflow) {  // Ignore underflow which happens if we have no data
            // std::cerr << "PA Write Error: " << Pa_GetErrorText(err) << std::endl;
        }
    }
}
