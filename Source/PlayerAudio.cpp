#include "PlayerAudio.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
    mixer.addInputSource(&transportSource, false);
    mixer.addInputSource(&mixerTransportSource, false);
}

PlayerAudio::~PlayerAudio()
{
    mixer.removeAllInputs();
    transportSource.stop();
    transportSource.setSource(nullptr);
    mixerTransportSource.stop();
    mixerTransportSource.setSource(nullptr);
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixerTransportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    mixer.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    mixer.getNextAudioBlock(bufferToFill);
    
    double currentPos = transportSource.getCurrentPosition();
    double length = getLengthInSeconds();
    
    if (looping && length > 0.0 && currentPos >= length - 0.05)
    {
        transportSource.setPosition(0.0);
        transportSource.start();
        if (mixerReaderSource.get() != nullptr)
        {
            mixerTransportSource.setPosition(0.0);
            mixerTransportSource.start();
        }
    }
}

void PlayerAudio::releaseResources()
{
    mixer.releaseResources();
    transportSource.releaseResources();
    mixerTransportSource.releaseResources();
}

void PlayerAudio::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return;
    
    title = reader->metadataValues.getValue("title", file.getFileNameWithoutExtension());
    artist = reader->metadataValues.getValue("artist", "Unknown Artist");
    duration = reader->lengthInSamples / reader->sampleRate;
    
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
    readerSource->setLooping(looping);
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    transportSource.setGain(gain);
    transportSource.setPosition(0.0);
    duration = transportSource.getLengthInSeconds();
    
    if (artist == "Unknown Artist")
    {
        juce::ChildProcess ffprobe;
        juce::StringArray args;
        args.add("ffprobe");
        args.add("-v");
        args.add("error");
        args.add("-show_entries");
        args.add("format_tags=artist,title");
        args.add("-of");
        args.add("default=nw=1:nk=1");
        args.add(file.getFullPathName());
        
        if (ffprobe.start(args))
        {
            juce::StringArray lines;
            lines.addLines(ffprobe.readAllProcessOutput().trim());
            if (lines.size() >= 2)
            {
                artist = lines[0];
                title = lines[1];
            }
        }
    }
}

void PlayerAudio::loadMixerFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return;
    
    mixerTitle = reader->metadataValues.getValue("title", file.getFileNameWithoutExtension());
    mixerArtist = reader->metadataValues.getValue("artist", "Unknown Artist");
    mixerDuration = reader->lengthInSamples / reader->sampleRate;
    
    mixerTransportSource.stop();
    mixerTransportSource.setSource(nullptr);
    mixerReaderSource.reset(new juce::AudioFormatReaderSource(reader, true));
    mixerReaderSource->setLooping(looping);
    mixerTransportSource.setSource(mixerReaderSource.get(), 0, nullptr, reader->sampleRate);
    mixerTransportSource.setGain(mixerGain);
    mixerTransportSource.setPosition(0.0);
    mixerDuration = mixerTransportSource.getLengthInSeconds();
    
    if (mixerArtist == "Unknown Artist")
    {
        juce::ChildProcess ffprobe;
        juce::StringArray args;
        args.add("ffprobe");
        args.add("-v");
        args.add("error");
        args.add("-show_entries");
        args.add("format_tags=artist,title");
        args.add("-of");
        args.add("default=nw=1:nk=1");
        args.add(file.getFullPathName());
        
        if (ffprobe.start(args))
        {
            juce::StringArray lines;
            lines.addLines(ffprobe.readAllProcessOutput().trim());
            if (lines.size() >= 2)
            {
                mixerArtist = lines[0];
                mixerTitle = lines[1];
            }
        }
    }
}

void PlayerAudio::play()
{
    if (!transportSource.isPlaying())
        transportSource.start();
    if (mixerReaderSource.get() != nullptr && !mixerTransportSource.isPlaying())
        mixerTransportSource.start();
}

void PlayerAudio::pause()
{
    transportSource.stop();
    if (mixerReaderSource.get() != nullptr)
        mixerTransportSource.stop();
}

void PlayerAudio::stop()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
    if (mixerReaderSource.get() != nullptr)
    {
        mixerTransportSource.stop();
        mixerTransportSource.setPosition(0.0);
    }
}

void PlayerAudio::setPosition(double pos)
{
    transportSource.setPosition(pos);
    if (mixerReaderSource.get() != nullptr)
        mixerTransportSource.setPosition(pos);
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

void PlayerAudio::setMixerGain(float g)
{
    mixerGain = g;
    if (mixerReaderSource.get() != nullptr)
        mixerTransportSource.setGain(mixerGain);
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
    if (mixerReaderSource)
        mixerReaderSource->setLooping(looping);
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
        if (mixerReaderSource.get() != nullptr)
            mixerTransportSource.setGain(mixerGain);
        isMuted = false;
    }
    else
    {
        transportSource.setGain(0.0f);
        if (mixerReaderSource.get() != nullptr)
            mixerTransportSource.setGain(0.0f);
        isMuted = true;
    }
}

void PlayerAudio::jumpForward(double seconds)
{
    double newPosition = transportSource.getCurrentPosition() + seconds;
    if (newPosition < transportSource.getLengthInSeconds())
    {
        transportSource.setPosition(newPosition);
        if (mixerReaderSource.get() != nullptr)
            mixerTransportSource.setPosition(newPosition);
    }
    else
    {
        transportSource.stop();
        if (mixerReaderSource.get() != nullptr)
            mixerTransportSource.stop();
    }
}

void PlayerAudio::jumpBackward(double seconds)
{
    double newPosition = transportSource.getCurrentPosition() - seconds;
    if (newPosition > 0.0)
    {
        transportSource.setPosition(newPosition);
        if (mixerReaderSource.get() != nullptr)
            mixerTransportSource.setPosition(newPosition);
    }
    else
    {
        transportSource.setPosition(0.0);
        if (mixerReaderSource.get() != nullptr)
            mixerTransportSource.setPosition(0.0);
    }
}
