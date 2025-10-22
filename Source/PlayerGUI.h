#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
                  public juce::Button::Listener,
                  public juce::Slider::Listener
{
public:
    PlayerGUI(PlayerAudio& audioRef);
    ~PlayerGUI() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    PlayerAudio& audio;

    juce::DrawableButton loadButton   {"load", juce::DrawableButton::ImageFitted};
    juce::DrawableButton restartButton{"restart", juce::DrawableButton::ImageFitted};
    juce::DrawableButton stopButton   {"stop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton playPauseButton{"playpause", juce::DrawableButton::ImageFitted};
    juce::DrawableButton startButton  {"start", juce::DrawableButton::ImageFitted};
    juce::DrawableButton endButton    {"end", juce::DrawableButton::ImageFitted};
    juce::TextButton muteButton { "Mute" };


    juce::Slider volumeSlider;
    juce::Label metadataLabel;
    std::unique_ptr<juce::Drawable> loadIcon, restartIcon, stopIcon, playIcon, pauseIcon, startIcon, endIcon;
    std::unique_ptr<juce::FileChooser> fileChooser;
    bool isPlaying = false;

    juce::File getSVGFile(const juce::String& name);
    void safeSetButtonImage(juce::DrawableButton& btn, std::unique_ptr<juce::Drawable>& drawable, const juce::String& fallbackText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};

