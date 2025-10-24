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
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);

    double currentPos = transportSource.getCurrentPosition();
    double length = getLengthInSeconds();

    if (looping && length > 0.0 && currentPos >= length - 0.05)
    {
        transportSource.setPosition(0.0);
        transportSource.start();
    }
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
}

void PlayerAudio::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return;

    transportSource.stop();
    transportSource.setSource(nullptr);

    readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
    readerSource->setLooping(looping);
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    transportSource.setGain(gain);
    transportSource.setPosition(0.0);
}

void PlayerAudio::play()
{
    if (!transportSource.isPlaying())
        transportSource.start();
}

void PlayerAudio::pause()
{
        transportSource.stop();
}

void PlayerAudio::stop()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void PlayerAudio::setPosition(double pos)
{
    transportSource.setPosition(pos);
}

double PlayerAudio::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

void PlayerAudio::setGain(float g)
{
    gain = g;
    transportSource.setGain(gain);
}

bool PlayerAudio::isPlaying() const
{
    return transportSource.isPlaying();
}

void PlayerAudio::setLooping(bool shouldLoop)
{
    looping = shouldLoop;
    if (readerSource)
        readerSource->setLooping(looping);
}

bool PlayerAudio::isLooping() const
{
    return looping;
}

void PlayerAudio::toggleMute()
{
    if (isMuted)
    {
        transportSource.setGain(gain); 
        isMuted = false;
    }
    else
    {
        transportSource.setGain(0.0f);
        isMuted = true;
    }
}

void PlayerAudio::jumpForward(double seconds)
{
    double newPosition = transportSource.getCurrentPosition() + seconds;
    if (newPosition < transportSource.getLengthInSeconds())
        transportSource.setPosition(newPosition);
    else
        transportSource.stop();
}

void PlayerAudio::jumpBackward(double seconds)
{
    double newPosition = transportSource.getCurrentPosition() - seconds;
    if (newPosition > 0.0)
        transportSource.setPosition(newPosition);
    else
        transportSource.setPosition(0.0);
}

