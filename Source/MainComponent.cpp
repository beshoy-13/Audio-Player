#include "MainComponent.h"

MainComponent::MainComponent() : gui(audio)
{
    addAndMakeVisible(gui);
    setSize(1100, 260);
    setAudioChannels(0, 2);
}
MainComponent::~MainComponent()
{
    shutdownAudio();
}
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audio.prepareToPlay(samplesPerBlockExpected, sampleRate);
}
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    audio.getNextAudioBlock(bufferToFill);
}
void MainComponent::releaseResources()
{
    audio.releaseResources();
}
void MainComponent::resized()
{
    gui.setBounds(getLocalBounds());
}
