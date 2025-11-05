#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

struct Marker
{
    double timestamp;
    juce::String label;
};

class WaveformDisplay : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    WaveformDisplay(PlayerAudio& audioRef);
    ~WaveformDisplay() override;
    
    void paint(juce::Graphics& g) override;
    void loadWaveform(const juce::File& audioFile);
    void setCurrentPosition(double position);
    void setABMarkers(bool enabled, double startPos, double endPos);
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    
private:
    PlayerAudio& audio;
    juce::AudioThumbnailCache thumbnailCache{5};
    juce::AudioThumbnail thumbnail{512, audio.getFormatManager(), thumbnailCache};
    bool fileLoaded = false;
    double currentPosition = 0.0;
    bool abMarkersEnabled = false;
    double abStart = 0.0;
    double abEnd = 0.0;
};

class ABLoopDialog : public juce::Component
{
public:
    ABLoopDialog();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    std::function<void(int, int, int, int)> onConfirm;
    std::function<void()> onCancel;
    
    juce::TextEditor startMinutes;
    juce::TextEditor startSeconds;
    juce::TextEditor endMinutes;
    juce::TextEditor endSeconds;
    juce::TextButton confirmButton;
    juce::TextButton cancelButton;
    juce::Label titleLabel;
    juce::Label startLabel;
    juce::Label endLabel;
    juce::Label colonLabel1;
    juce::Label colonLabel2;
};

class PlayerGUI : public juce::Component,
                  public juce::Button::Listener,
                  public juce::Slider::Listener,
                  public juce::ListBoxModel,
                  public juce::Timer
{
public:
    PlayerGUI(PlayerAudio& audioRef);
    ~PlayerGUI() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;
    void updateMetadata(const juce::String& title, const juce::String& artist, const juce::String& album, double duration);
    void updateMixerMetadata(const juce::String& title, const juce::String& artist, const juce::String& album, double duration);
    void refreshPlaylist();
    void showABLoopDialog();
    void saveSession();
    void loadSession();
    void addMarker();
    void showMarkersDialog();

    bool keyPressed(const juce::KeyPress& key) override;
    
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked(int rowNumber, const juce::MouseEvent&) override;

    void setFile(const juce::File& file);

private:
    PlayerAudio& audio;
    
    juce::DrawableButton loadButton{"load", juce::DrawableButton::ImageFitted};
    juce::DrawableButton restartButton{"restart", juce::DrawableButton::ImageFitted};
    juce::DrawableButton stopButton{"stop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton playPauseButton{"playpause", juce::DrawableButton::ImageFitted};
    juce::DrawableButton startButton{"start", juce::DrawableButton::ImageFitted};
    juce::DrawableButton endButton{"end", juce::DrawableButton::ImageFitted};
    juce::DrawableButton loopButton{"loop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton muteButton{"mute", juce::DrawableButton::ImageFitted};
    juce::DrawableButton forwardButton{"forward", juce::DrawableButton::ImageFitted};
    juce::DrawableButton backwardButton{"backward", juce::DrawableButton::ImageFitted};
    juce::DrawableButton playlistButton{"playlist", juce::DrawableButton::ImageFitted};
    juce::DrawableButton mixerButton{"mixer", juce::DrawableButton::ImageFitted};
    juce::DrawableButton abLoopButton{"abloop", juce::DrawableButton::ImageFitted};
    juce::DrawableButton saveButton{"save", juce::DrawableButton::ImageFitted};
    juce::DrawableButton markerButton{"marker", juce::DrawableButton::ImageFitted};
    
    juce::Slider volumeSlider;
    juce::Slider mixerVolumeSlider;
    juce::Slider positionSlider;
    juce::Slider speedSlider;
    
    juce::Label metadataLabel;
    juce::Label mixerMetadataLabel;
    juce::Label titleLabel;
    juce::Label artistLabel;
    juce::Label albumLabel;
    juce::Label durationLabel;
    juce::Label currentTimeLabel;
    juce::Label totalTimeLabel;
    juce::Label speedLabel;
    
    WaveformDisplay waveformDisplay{audio};
    
    juce::ListBox playlistBox;
    juce::Array<juce::File> playlistFiles;
    bool playlistVisible = false;
    
    std::unique_ptr<juce::Drawable> loadIcon, restartIcon, stopIcon,
        playIcon, pauseIcon, startIcon, endIcon, loopIcon, muteIcon,
        unmuteIcon, backwardIcon, forwardIcon, playlistIcon, mixerIcon,
        abLoopIcon, saveIcon, markerIcon;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<ABLoopDialog> abDialog;
    
    bool isPlaying = false;
    bool isLooping = false;
    bool isDraggingPosition = false;
    int mixerFileCount = 0;
    juce::File pendingMixerFile1;
    juce::File pendingMixerFile2;

    juce::Array<Marker> markers;
    std::unique_ptr<juce::Component> markersDialog;
    
    juce::File getSVGFile(const juce::String& name);
    void safeSetButtonImage(juce::DrawableButton& btn, std::unique_ptr<juce::Drawable>& drawable, const juce::String& fallbackText);
    juce::String formatTime(double seconds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};

