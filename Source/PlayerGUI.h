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
    void updateMetadata(const juce::String& title, const juce::String& artist, const juce::String& album, double duration);


private:
    PlayerAudio& audio;

    juce::DrawableButton loadButton   {"load", juce::DrawableButton::ImageFitted};
    juce::DrawableButton restartButton{"restart", juce::DrawableButton::ImageFitted};
    juce::DrawableButton stopButton   {"stop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton playPauseButton{"playpause", juce::DrawableButton::ImageFitted};
    juce::DrawableButton startButton  {"start", juce::DrawableButton::ImageFitted};
    juce::DrawableButton endButton    {"end", juce::DrawableButton::ImageFitted};
    juce::DrawableButton loopButton   {"loop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton muteButton{"mute", juce::DrawableButton::ImageFitted};
    juce::DrawableButton forwardButton{"forward", juce::DrawableButton::ImageFitted};
    juce::DrawableButton backwardButton{"backward", juce::DrawableButton::ImageFitted};

    juce::Slider volumeSlider;
    juce::Label metadataLabel;
    juce::Label titleLabel;
    juce::Label artistLabel;
    juce::Label albumLabel;
    juce::Label durationLabel;

    std::unique_ptr<juce::Drawable> loadIcon, restartIcon, stopIcon, playIcon, pauseIcon, startIcon, endIcon, loopIcon, muteIcon, unmuteIcon, backwardIcon, forwardIcon;
    std::unique_ptr<juce::FileChooser> fileChooser;
    bool isPlaying = false;
    bool isLooping = false;

    juce::File getSVGFile(const juce::String& name);
    void safeSetButtonImage(juce::DrawableButton& btn, std::unique_ptr<juce::Drawable>& drawable, const juce::String& fallbackText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};

