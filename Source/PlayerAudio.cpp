#include "PlayerAudio.h"

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
    transportSource.stop();
    transportSource.setSource(nullptr);
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
        bufferToFill.clearActiveBufferRegion();
    else
        transportSource.getNextAudioBlock(bufferToFill);
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
}

void PlayerAudio::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr) return;

    transportSource.stop();
    transportSource.setSource(nullptr);

    readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    transportSource.setGain(gain);
    transportSource.setPosition(0.0);
}

void PlayerAudio::play()
{
    transportSource.start();
}

void PlayerAudio::stop()
{
    transportSource.stop();
}

void PlayerAudio::setPosition(double pos)
{
    transportSource.setPosition(pos);
}

void PlayerAudio::setGain(float g)
{
    gain = g;
    transportSource.setGain(gain);
}

double PlayerAudio::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

bool PlayerAudio::isPlaying() const
{
    return transportSource.isPlaying();
}

//Mute
void PlayerAudio::toggleMute()
{
    if (isMuted)
    {
        transportSource.setGain(1.0f); // رجع الصوت
        isMuted = false;
    }
    else
    {
        transportSource.setGain(0.0f); // كتم الصوت
        isMuted = true;
    }
}

