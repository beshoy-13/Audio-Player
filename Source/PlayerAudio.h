#pragma once
#include <JuceHeader.h>

class PlayerAudio : public juce::AudioSource
{
public:
    PlayerAudio();
    ~PlayerAudio() override;
    
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    
    void loadFile(const juce::File& file);
    void loadMixerFile(const juce::File& file);
    void play();
    void pause();
    void stop();
    void setPosition(double pos);
    double getLengthInSeconds() const;
    void setGain(float g);
    void setMixerGain(float g);
    bool isPlaying() const;
    void setLooping(bool shouldLoop);
    bool isLooping() const;
    void toggleMute();
    bool isMuted = false;
    void jumpForward(double seconds);
    void jumpBackward(double seconds);
    
    juce::String getTitle() const { return title; }
    juce::String getArtist() const { return artist; }
    juce::String getAlbum() const { return album; }
    double getDuration() const { return duration; }
    
    juce::String getMixerTitle() const { return mixerTitle; }
    juce::String getMixerArtist() const { return mixerArtist; }
    juce::String getMixerAlbum() const { return mixerAlbum; }
    double getMixerDuration() const { return mixerDuration; }
    
    bool hasMixerTrack() const { return mixerReaderSource.get() != nullptr; }

private:
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    juce::AudioTransportSource mixerTransportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> mixerReaderSource;
    
    juce::MixerAudioSource mixer;
    
    float gain = 1.0f;
    float mixerGain = 1.0f;
    bool looping = false;
    
    juce::String title;
    juce::String artist;
    juce::String album;
    double duration = 0.0;
    
    juce::String mixerTitle;
    juce::String mixerArtist;
    juce::String mixerAlbum;
    double mixerDuration = 0.0;
};
