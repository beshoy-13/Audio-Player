#include "PlayerGUI.h"

static juce::String formatDuration(double seconds)
{
    int totalSecs = static_cast<int>(seconds);
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
}

static std::unique_ptr<juce::Drawable> createDrawableFromSVGFile(const juce::File& svgFile)
{
    if (!svgFile.existsAsFile())
        return nullptr;
    juce::String contents = svgFile.loadFileAsString();
    if (contents.isEmpty())
        return nullptr;
    std::unique_ptr<juce::XmlElement> xml = juce::parseXML(contents);
    if (!xml)
        return nullptr;
    return juce::Drawable::createFromSVG(*xml);
}

WaveformDisplay::WaveformDisplay(PlayerAudio& audioRef)
    : audio(audioRef), thumbnailCache(5), thumbnail(512, audio.getFormatManager(), thumbnailCache)
{
    thumbnail.addChangeListener(this);
    startTimer(40);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("#FF1A1F2B"));
    g.setColour(juce::Colour::fromString("#FFFEE715"));
    
    if (thumbnail.getTotalLength() > 0.0)
    {
        thumbnail.drawChannels(g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 0.7f);
        
        if (abMarkersEnabled)
        {
            double totalLength = thumbnail.getTotalLength();
            if (totalLength > 0.0)
            {
                float startX = (float)((abStart / totalLength) * (double)getWidth());
                float endX = (float)((abEnd / totalLength) * (double)getWidth());
                
                g.setColour(juce::Colours::red.withAlpha(0.3f));
                g.fillRect(startX, 0.0f, endX - startX, (float)getHeight());
                
                g.setColour(juce::Colours::red);
                g.drawLine(startX, 0, startX, getHeight(), 2.0f);
                g.drawLine(endX, 0, endX, getHeight(), 2.0f);
            }
        }
        
        for (const auto& marker : displayMarkers)
        {
            double totalLength = thumbnail.getTotalLength();
            if (totalLength > 0.0)
            {
                float xPos = (float)((marker.timestamp / totalLength) * (double)getWidth());
                g.setColour(juce::Colours::green);
                g.drawLine(xPos, 0, xPos, getHeight(), 1.5f);
                g.setColour(juce::Colours::green.withAlpha(0.8f));
                g.drawText(marker.label, xPos + 2, 2, 100, 20, juce::Justification::left);
            }
        }
        
        double totalLength = thumbnail.getTotalLength();
        if (totalLength > 0.0)
        {
            float xPos = (float)((currentPosition / totalLength) * (double)getWidth());
            g.setColour(juce::Colours::white);
            g.drawLine(xPos, 0, xPos, getHeight(), 2.0f);
        }
    }
    else
    {
        g.drawText("No audio loaded", getLocalBounds(), juce::Justification::centred);
    }
}

void WaveformDisplay::loadWaveform(const juce::File& audioFile)
{
    thumbnail.clear();
    if (audioFile.existsAsFile())
    {
        thumbnail.setSource(new juce::FileInputSource(audioFile));
        fileLoaded = true;
    }
    repaint();
}

void WaveformDisplay::setCurrentPosition(double position)
{
    currentPosition = position;
}

void WaveformDisplay::setABMarkers(bool enabled, double startPos, double endPos)
{
    abMarkersEnabled = enabled;
    abStart = startPos;
    abEnd = endPos;
    repaint();
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

void WaveformDisplay::setMarkers(const juce::Array<Marker>& markersToShow)
{
    displayMarkers = markersToShow;
    repaint();
}

ABLoopDialog::ABLoopDialog()
{
    setSize(400, 250);
    
    titleLabel.setText("Set A-B Loop Markers", juce::dontSendNotification);
    juce::Font tf(juce::FontOptions().withHeight(20.0f).withStyle("Bold"));
    titleLabel.setFont(tf);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(titleLabel);
    
    startLabel.setText("Start (A):", juce::dontSendNotification);
    startLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(startLabel);
    
    endLabel.setText("End (B):", juce::dontSendNotification);
    endLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(endLabel);
    
    startMinutes.setInputRestrictions(2, "0123456789");
    startMinutes.setText("0");
    startMinutes.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    startMinutes.setColour(juce::TextEditor::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(startMinutes);
    
    startSeconds.setInputRestrictions(2, "0123456789");
    startSeconds.setText("0");
    startSeconds.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    startSeconds.setColour(juce::TextEditor::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(startSeconds);
    
    endMinutes.setInputRestrictions(2, "0123456789");
    endMinutes.setText("0");
    endMinutes.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    endMinutes.setColour(juce::TextEditor::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(endMinutes);
    
    endSeconds.setInputRestrictions(2, "0123456789");
    endSeconds.setText("0");
    endSeconds.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    endSeconds.setColour(juce::TextEditor::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(endSeconds);
    
    colonLabel1.setText(":", juce::dontSendNotification);
    colonLabel1.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    colonLabel1.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(colonLabel1);
    
    colonLabel2.setText(":", juce::dontSendNotification);
    colonLabel2.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    colonLabel2.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(colonLabel2);
    
    confirmButton.setButtonText("Confirm");
    confirmButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString("#FFFEE715"));
    confirmButton.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString("#FF101820"));
    confirmButton.onClick = [this]()
    {
        if (onConfirm)
        {
            int startMin = startMinutes.getText().getIntValue();
            int startSec = startSeconds.getText().getIntValue();
            int endMin = endMinutes.getText().getIntValue();
            int endSec = endSeconds.getText().getIntValue();
            onConfirm(startMin, startSec, endMin, endSec);
        }
    };
    addAndMakeVisible(confirmButton);
    
    cancelButton.setButtonText("Cancel");
    cancelButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    cancelButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    cancelButton.onClick = [this]()
    {
        if (onCancel)
            onCancel();
    };
    addAndMakeVisible(cancelButton);
}

void ABLoopDialog::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("#FF101820"));
    g.setColour(juce::Colour::fromString("#FFFEE715"));
    g.drawRect(getLocalBounds(), 2);
}

void ABLoopDialog::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(20);
    
    auto startRow = area.removeFromTop(40);
    startLabel.setBounds(startRow.removeFromLeft(100));
    startRow.removeFromLeft(10);
    startMinutes.setBounds(startRow.removeFromLeft(50));
    colonLabel1.setBounds(startRow.removeFromLeft(20));
    startSeconds.setBounds(startRow.removeFromLeft(50));
    
    area.removeFromTop(20);
    
    auto endRow = area.removeFromTop(40);
    endLabel.setBounds(endRow.removeFromLeft(100));
    endRow.removeFromLeft(10);
    endMinutes.setBounds(endRow.removeFromLeft(50));
    colonLabel2.setBounds(endRow.removeFromLeft(20));
    endSeconds.setBounds(endRow.removeFromLeft(50));
    
    area.removeFromTop(30);
    
    auto buttonRow = area.removeFromTop(40);
    confirmButton.setBounds(buttonRow.removeFromLeft(150));
    buttonRow.removeFromLeft(20);
    cancelButton.setBounds(buttonRow.removeFromLeft(150));
}

juce::File PlayerGUI::getSVGFile(const juce::String& name)
{
    juce::File execDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 5; ++i)
    {
        juce::File svgDir = execDir.getSiblingFile("Source").getChildFile("svgs");
        if (svgDir.exists()) return svgDir.getChildFile(name + ".svg");
        execDir = execDir.getParentDirectory();
    }
    return juce::File();
}

juce::String PlayerGUI::formatTime(double seconds)
{
    int totalSecs = static_cast<int>(seconds);
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
}

void PlayerGUI::safeSetButtonImage(juce::DrawableButton& btn, std::unique_ptr<juce::Drawable>& drawable, const juce::String& fallbackText)
{
    btn.setButtonStyle(juce::DrawableButton::ImageFitted);
    if (drawable) { btn.setImages(drawable.get()); btn.setButtonText(""); }
    else btn.setButtonText(fallbackText);
}
PlayerGUI::PlayerGUI(PlayerAudio& audioRef) : audio(audioRef), waveformDisplay(audioRef), mixerWaveformDisplay(audioRef)
{
    loadIcon     = createDrawableFromSVGFile(getSVGFile("upload"));
    restartIcon  = createDrawableFromSVGFile(getSVGFile("restart"));
    stopIcon     = createDrawableFromSVGFile(getSVGFile("stop"));
    playIcon     = createDrawableFromSVGFile(getSVGFile("start"));
    pauseIcon    = createDrawableFromSVGFile(getSVGFile("pause"));
    startIcon    = createDrawableFromSVGFile(getSVGFile("startsong"));
    endIcon      = createDrawableFromSVGFile(getSVGFile("endsong"));
    loopIcon     = createDrawableFromSVGFile(getSVGFile("loop"));
    muteIcon     = createDrawableFromSVGFile(getSVGFile("mute"));
    unmuteIcon   = createDrawableFromSVGFile(getSVGFile("unmute"));
    backwardIcon = createDrawableFromSVGFile(getSVGFile("backward"));
    forwardIcon  = createDrawableFromSVGFile(getSVGFile("forward"));
    playlistIcon = createDrawableFromSVGFile(getSVGFile("playlist"));
    mixerIcon    = createDrawableFromSVGFile(getSVGFile("mixer"));
    abLoopIcon   = createDrawableFromSVGFile(getSVGFile("abloop"));
    saveIcon     = createDrawableFromSVGFile(getSVGFile("save"));
    markerIcon   = createDrawableFromSVGFile(getSVGFile("marker"));
    
    safeSetButtonImage(loadButton, loadIcon, "Load");
    safeSetButtonImage(restartButton, restartIcon, "Restart");
    safeSetButtonImage(stopButton, stopIcon, "Stop");
    safeSetButtonImage(playPauseButton, playIcon, "Play");
    safeSetButtonImage(startButton, startIcon, "Start");
    safeSetButtonImage(endButton, endIcon, "End");
    safeSetButtonImage(loopButton, loopIcon, "Loop");
    safeSetButtonImage(muteButton, muteIcon, "Mute");
    safeSetButtonImage(backwardButton, backwardIcon, "backward 5s");
    safeSetButtonImage(forwardButton, forwardIcon, "forward 5s");
    safeSetButtonImage(playlistButton, playlistIcon, "Playlist");
    safeSetButtonImage(mixerButton, mixerIcon, "Mixer");
    safeSetButtonImage(abLoopButton, abLoopIcon, "A-B Loop");
    safeSetButtonImage(saveButton, saveIcon, "Save");
    safeSetButtonImage(markerButton, markerIcon, "Marker");
    safeSetButtonImage(track1PlayPauseButton, playIcon, "Play T1");
    safeSetButtonImage(track1MuteButton, muteIcon, "Mute T1");
    safeSetButtonImage(track1ForwardButton, forwardIcon, "Fwd T1");
    safeSetButtonImage(track1BackwardButton, backwardIcon, "Bwd T1");
    safeSetButtonImage(track2PlayPauseButton, playIcon, "Play T2");
    safeSetButtonImage(track2MuteButton, muteIcon, "Mute T2");
    safeSetButtonImage(track2ForwardButton, forwardIcon, "Fwd T2");
    safeSetButtonImage(track2BackwardButton, backwardIcon, "Bwd T2");
    
    for (auto* b : { &loadButton, &restartButton, &stopButton, &playPauseButton, &startButton, &endButton,
                 &loopButton, &muteButton, &backwardButton, &forwardButton, &playlistButton, &mixerButton,
                 &abLoopButton, &saveButton, &markerButton, &track1PlayPauseButton, &track1MuteButton, &track1ForwardButton, &track1BackwardButton, &track2PlayPauseButton, &track2MuteButton, &track2ForwardButton, &track2BackwardButton })
    {
        addAndMakeVisible(b);
        b->addListener(this);
    }
    
    volumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#FFFEE715"));
    volumeSlider.setColour(juce::Slider::trackColourId, juce::Colour::fromString("#FFFEE715"));
    volumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    volumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromString("#FFFEE715"));
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8, juce::dontSendNotification);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);
    
    mixerVolumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#FFFEE715"));
    mixerVolumeSlider.setColour(juce::Slider::trackColourId, juce::Colour::fromString("#FFFEE715"));
    mixerVolumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    mixerVolumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromString("#FFFEE715"));
    mixerVolumeSlider.setRange(0.0, 1.0, 0.01);
    mixerVolumeSlider.setValue(0.8, juce::dontSendNotification);
    mixerVolumeSlider.addListener(this);
    mixerVolumeSlider.setVisible(false);
    addAndMakeVisible(mixerVolumeSlider);
    
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#FFFEE715"));
    positionSlider.setColour(juce::Slider::trackColourId, juce::Colour::fromString("#FFFEE715"));
    positionSlider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    positionSlider.setRange(0.0, 100.0, 0.001);
    positionSlider.setValue(0.0, juce::dontSendNotification);
    positionSlider.addListener(this);
    addAndMakeVisible(positionSlider);
    
    positionSlider.onDragStart = [this]() { isDraggingPosition = true; };
    positionSlider.onDragEnd = [this]() { isDraggingPosition = false; };
    
    mixerPositionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixerPositionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixerPositionSlider.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#FFFEE715"));
    mixerPositionSlider.setColour(juce::Slider::trackColourId, juce::Colour::fromString("#FFFEE715"));
    mixerPositionSlider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    mixerPositionSlider.setRange(0.0, 100.0, 0.001);
    mixerPositionSlider.setValue(0.0, juce::dontSendNotification);
    mixerPositionSlider.addListener(this);
    mixerPositionSlider.setVisible(false);
    addAndMakeVisible(mixerPositionSlider);
    
    mixerPositionSlider.onDragStart = [this]() { isDraggingMixerPosition = true; };
    mixerPositionSlider.onDragEnd = [this]() { isDraggingMixerPosition = false; };
    
    speedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    speedSlider.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#FFFEE715"));
    speedSlider.setColour(juce::Slider::trackColourId, juce::Colour::fromString("#FFFEE715"));
    speedSlider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    speedSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromString("#FFFEE715"));
    speedSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    speedSlider.setRange(0.5, 2.0, 0.1);
    speedSlider.setValue(1.0, juce::dontSendNotification);
    speedSlider.setTextValueSuffix("x");
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);
    
    mixerSpeedSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixerSpeedSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 60, 20);
    mixerSpeedSlider.setColour(juce::Slider::thumbColourId, juce::Colour::fromString("#FFFEE715"));
    mixerSpeedSlider.setColour(juce::Slider::trackColourId, juce::Colour::fromString("#FFFEE715"));
    mixerSpeedSlider.setColour(juce::Slider::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    mixerSpeedSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromString("#FFFEE715"));
    mixerSpeedSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    mixerSpeedSlider.setRange(0.5, 2.0, 0.1);
    mixerSpeedSlider.setValue(1.0, juce::dontSendNotification);
    mixerSpeedSlider.setTextValueSuffix("x");
    mixerSpeedSlider.addListener(this);
    mixerSpeedSlider.setVisible(false);
    addAndMakeVisible(mixerSpeedSlider);
    
    speedLabel.setText("Speed:", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    speedLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(speedLabel);
    
    mixerSpeedLabel.setText("Speed:", juce::dontSendNotification);
    mixerSpeedLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    mixerSpeedLabel.setJustificationType(juce::Justification::centredRight);
    mixerSpeedLabel.setVisible(false);
    addAndMakeVisible(mixerSpeedLabel);
    
    track1Label.setText("TRACK 1", juce::dontSendNotification);
    track1Label.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    track1Label.setJustificationType(juce::Justification::centred);
    juce::Font trackFont(juce::FontOptions().withHeight(24.0f).withStyle("Bold"));
    track1Label.setFont(trackFont);
    addAndMakeVisible(track1Label);
    
    track2Label.setText("TRACK 2", juce::dontSendNotification);
    track2Label.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    track2Label.setJustificationType(juce::Justification::centred);
    track2Label.setFont(trackFont);
    track2Label.setVisible(false);
    addAndMakeVisible(track2Label);

    track1PlayPauseButton.setVisible(false);
    track1MuteButton.setVisible(false);
    track1ForwardButton.setVisible(false);
    track1BackwardButton.setVisible(false);
    track2PlayPauseButton.setVisible(false);
    track2MuteButton.setVisible(false);
    track2ForwardButton.setVisible(false);
    track2BackwardButton.setVisible(false);
    isTrack1Playing = false;
    isTrack2Playing = false;
    isTrack1Muted = false;
    isTrack2Muted = false;

    metadataLabel.setJustificationType(juce::Justification::centredLeft);
    metadataLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(metadataLabel);
    
    mixerMetadataLabel.setJustificationType(juce::Justification::centredLeft);
    mixerMetadataLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    mixerMetadataLabel.setVisible(false);
    addAndMakeVisible(mixerMetadataLabel);
    
    currentTimeLabel.setJustificationType(juce::Justification::centredLeft);
    currentTimeLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    currentTimeLabel.setText("0:00", juce::dontSendNotification);
    addAndMakeVisible(currentTimeLabel);
    
    totalTimeLabel.setJustificationType(juce::Justification::centredRight);
    totalTimeLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    totalTimeLabel.setText("0:00", juce::dontSendNotification);
    addAndMakeVisible(totalTimeLabel);
    
    mixerCurrentTimeLabel.setJustificationType(juce::Justification::centredLeft);
    mixerCurrentTimeLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    mixerCurrentTimeLabel.setText("0:00", juce::dontSendNotification);
    mixerCurrentTimeLabel.setVisible(false);
    addAndMakeVisible(mixerCurrentTimeLabel);
    
    mixerTotalTimeLabel.setJustificationType(juce::Justification::centredRight);
    mixerTotalTimeLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    mixerTotalTimeLabel.setText("0:00", juce::dontSendNotification);
    mixerTotalTimeLabel.setVisible(false);
    addAndMakeVisible(mixerTotalTimeLabel);
    
    addAndMakeVisible(waveformDisplay);
    addAndMakeVisible(mixerWaveformDisplay);
    mixerWaveformDisplay.setVisible(false);
    
    for (auto* l : { &titleLabel, &artistLabel, &albumLabel, &durationLabel })
    {
        l->setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
        l->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(l);
    }
    
    playlistBox.setModel(this);
    playlistBox.setColour(juce::ListBox::backgroundColourId, juce::Colour::fromString("#FF1A1F2B"));
    playlistBox.setColour(juce::ListBox::textColourId, juce::Colour::fromString("#FFFEE715"));
    playlistBox.setOutlineThickness(1);
    addAndMakeVisible(playlistBox);
    playlistBox.setVisible(false);
    
    startTimer(100);
    loadSession();
    setWantsKeyboardFocus(true);

    track1PlayPauseButton.setVisible(false);
    track2PlayPauseButton.setVisible(false);
    
    if (audio.getTitle().isNotEmpty())
    {
        updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
        double totalLength = audio.getLengthInSeconds();
        positionSlider.setRange(0.0, totalLength, 0.001);
        totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
    }
    
    if (audio.getMixerTitle().isNotEmpty())
    {
        updateMixerMetadata(audio.getMixerTitle(), audio.getMixerArtist(), audio.getMixerAlbum(), audio.getMixerDuration());
        mixerVolumeSlider.setVisible(true);
        mixerMetadataLabel.setVisible(true);
    }
}

PlayerGUI::~PlayerGUI()
{
    stopTimer();
    saveSession();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("#FF101820"));
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(10);
    int btnH = 60;
    int btnW = 60;
    int gap = 8;
    
    auto buttonArea = area.removeFromTop(btnH);
    int totalButtonWidth = (btnW * 15) + (gap * 14);
    int startX = (area.getWidth() - totalButtonWidth) / 2;
    buttonArea.removeFromLeft(startX);
    
    loadButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    playPauseButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    stopButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    restartButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    startButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    endButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    loopButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    muteButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    backwardButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    forwardButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    playlistButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    mixerButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    abLoopButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    saveButton.setBounds(buttonArea.removeFromLeft(btnW)); buttonArea.removeFromLeft(gap);
    markerButton.setBounds(buttonArea.removeFromLeft(btnW));
    
    area.removeFromTop(20);
    
    if (audio.hasMixerTrack())
    {
        int halfWidth = (area.getWidth() - 20) / 2;
        
        auto leftPanel = area.removeFromLeft(halfWidth);
        area.removeFromLeft(20);
        auto rightPanel = area;
        
        track1Label.setBounds(leftPanel.removeFromTop(30));
        leftPanel.removeFromTop(10);
        waveformDisplay.setBounds(leftPanel.removeFromTop(150));
        leftPanel.removeFromTop(10);
        auto track1ButtonRow = leftPanel.removeFromTop(40);
        track1PlayPauseButton.setBounds(track1ButtonRow.removeFromLeft(50));
        track1ButtonRow.removeFromLeft(5);
        track1MuteButton.setBounds(track1ButtonRow.removeFromLeft(50));
        track1ButtonRow.removeFromLeft(5);
        track1BackwardButton.setBounds(track1ButtonRow.removeFromLeft(50));
        track1ButtonRow.removeFromLeft(5);
        track1ForwardButton.setBounds(track1ButtonRow.removeFromLeft(50));
        leftPanel.removeFromTop(10);

        auto posArea = leftPanel.removeFromTop(50);
        auto timeLabels = posArea.removeFromTop(20);
        currentTimeLabel.setBounds(timeLabels.removeFromLeft(50));
        totalTimeLabel.setBounds(timeLabels.removeFromRight(50));
        positionSlider.setBounds(posArea);
        
        leftPanel.removeFromTop(10);
        auto speedArea = leftPanel.removeFromTop(30);
        speedLabel.setBounds(speedArea.removeFromLeft(60));
        speedSlider.setBounds(speedArea);
        
        leftPanel.removeFromTop(10);
        volumeSlider.setBounds(leftPanel.removeFromTop(40));
        leftPanel.removeFromTop(10);
        metadataLabel.setBounds(leftPanel.removeFromTop(30));
        
        track2Label.setBounds(rightPanel.removeFromTop(30));
        rightPanel.removeFromTop(10);
        mixerWaveformDisplay.setBounds(rightPanel.removeFromTop(150));
        rightPanel.removeFromTop(10);
        auto track2ButtonRow = rightPanel.removeFromTop(40);
        track2PlayPauseButton.setBounds(track2ButtonRow.removeFromLeft(50));
        track2ButtonRow.removeFromLeft(5);
        track2MuteButton.setBounds(track2ButtonRow.removeFromLeft(50));
        track2ButtonRow.removeFromLeft(5);
        track2BackwardButton.setBounds(track2ButtonRow.removeFromLeft(50));
        track2ButtonRow.removeFromLeft(5);
        track2ForwardButton.setBounds(track2ButtonRow.removeFromLeft(50));
        rightPanel.removeFromTop(10); 
        auto mixerPosArea = rightPanel.removeFromTop(50);
        auto mixerTimeLabels = mixerPosArea.removeFromTop(20);
        mixerCurrentTimeLabel.setBounds(mixerTimeLabels.removeFromLeft(50));
        mixerTotalTimeLabel.setBounds(mixerTimeLabels.removeFromRight(50));
        mixerPositionSlider.setBounds(mixerPosArea);
        
        rightPanel.removeFromTop(10);
        auto mixerSpeedArea = rightPanel.removeFromTop(30);
        mixerSpeedLabel.setBounds(mixerSpeedArea.removeFromLeft(60));
        mixerSpeedSlider.setBounds(mixerSpeedArea);
        
        rightPanel.removeFromTop(10);
        mixerVolumeSlider.setBounds(rightPanel.removeFromTop(40));
        rightPanel.removeFromTop(10);
        mixerMetadataLabel.setBounds(rightPanel.removeFromTop(30));
    }
    else
    {
        track1Label.setBounds(area.removeFromTop(30));
        area.removeFromTop(10);
        waveformDisplay.setBounds(area.removeFromTop(200));
        area.removeFromTop(10);
        
        auto posArea = area.removeFromTop(50);
        auto timeLabels = posArea.removeFromTop(20);
        currentTimeLabel.setBounds(timeLabels.removeFromLeft(50));
        totalTimeLabel.setBounds(timeLabels.removeFromRight(50));
        positionSlider.setBounds(posArea);
        
        area.removeFromTop(10);
        auto speedArea = area.removeFromTop(40);
        speedLabel.setBounds(speedArea.removeFromLeft(60));
        speedSlider.setBounds(speedArea);
        
        area.removeFromTop(10);
        volumeSlider.setBounds(area.removeFromTop(40));
        area.removeFromTop(10);
        metadataLabel.setBounds(area.removeFromTop(30));
    }
    
    if (playlistVisible)
        playlistBox.setBounds(area.removeFromBottom(200));
    else
        playlistBox.setBounds(0, 0, 0, 0);
}

void PlayerGUI::timerCallback()
{
    if (!isDraggingPosition)
    {
        double currentPos = audio.transportSource.getCurrentPosition();
        double totalLength = audio.getLengthInSeconds();
        
        if (totalLength > 0.0)
        {
            positionSlider.setValue(currentPos, juce::dontSendNotification);
            currentTimeLabel.setText(formatTime(currentPos), juce::dontSendNotification);
            totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
            waveformDisplay.setCurrentPosition(currentPos);
        }
    }
    
    if (!isDraggingMixerPosition && audio.hasMixerTrack())
    {
        double mixerPos = audio.mixerTransportSource.getCurrentPosition();
        double mixerLength = audio.getMixerLengthInSeconds();
        
        if (mixerLength > 0.0)
        {
            mixerPositionSlider.setValue(mixerPos, juce::dontSendNotification);
            mixerCurrentTimeLabel.setText(formatTime(mixerPos), juce::dontSendNotification);
            mixerTotalTimeLabel.setText(formatTime(mixerLength), juce::dontSendNotification);
            mixerWaveformDisplay.setCurrentPosition(mixerPos);
        }
    }
    if (audio.isPlaying())
    {
        double currentPos = audio.transportSource.getCurrentPosition();
        double totalLength = audio.getLengthInSeconds();
    
        if (totalLength > 0.0 && currentPos >= totalLength - 0.1)
        {
            playNextInPlaylist();
        }
    }
}
void PlayerGUI::showABLoopDialog()
{
    abDialog = std::make_unique<ABLoopDialog>();
    addAndMakeVisible(abDialog.get());
    int w = 400;
    int h = 250;
    abDialog->setBounds((getWidth() - w) / 2, (getHeight() - h) / 2, w, h);
    
    abDialog->onConfirm = [this](int startMin, int startSec, int endMin, int endSec)
    {
        double startTime = startMin * 60.0 + startSec;
        double endTime = endMin * 60.0 + endSec;
        
        if (startTime < endTime && endTime <= audio.getLengthInSeconds())
        {
            audio.setABLooping(true, startTime, endTime);
            waveformDisplay.setABMarkers(true, startTime, endTime);
            abLoopButton.setAlpha(1.0f);
        }
        
        removeChildComponent(abDialog.get());
        abDialog.reset();
    };
    
    abDialog->onCancel = [this]()
    {
        removeChildComponent(abDialog.get());
        abDialog.reset();
    };
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select an audio file...", juce::File(), "*.wav;*.mp3;*.aiff;*.flac");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file.existsAsFile())
                {
                    audio.loadFile(file);
                    updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
                    double totalLength = audio.getLengthInSeconds();
                    positionSlider.setRange(0.0, totalLength, 0.001);
                    totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
                    waveformDisplay.loadWaveform(file);
                    audio.play();
                    isPlaying = true;
                    if (pauseIcon) playPauseButton.setImages(pauseIcon.get()); else playPauseButton.setButtonText("Pause");
                }
            });
    }
    else if (button == &playPauseButton)
    {
        if (audio.isPlaying())
        {
            audio.pause();
            isPlaying = false;
            if (playIcon) playPauseButton.setImages(playIcon.get()); else playPauseButton.setButtonText("Play");
        }
        else
        {
            audio.play();
            isPlaying = true;
            if (pauseIcon) playPauseButton.setImages(pauseIcon.get()); else playPauseButton.setButtonText("Pause");
        }
    }
    else if (button == &stopButton)
    {
        audio.stop();
        audio.clearMixerTrack();
        isPlaying = false;
        
        positionSlider.setValue(0.0, juce::dontSendNotification);
        positionSlider.setRange(0.0, 100.0, 0.001);
        currentTimeLabel.setText("0:00", juce::dontSendNotification);
        totalTimeLabel.setText("0:00", juce::dontSendNotification);
        
        mixerPositionSlider.setValue(0.0, juce::dontSendNotification);
        mixerCurrentTimeLabel.setText("0:00", juce::dontSendNotification);
        mixerTotalTimeLabel.setText("0:00", juce::dontSendNotification);
        
        volumeSlider.setValue(0.8, juce::dontSendNotification);
        mixerVolumeSlider.setValue(0.8, juce::dontSendNotification);
        speedSlider.setValue(1.0, juce::dontSendNotification);
        mixerSpeedSlider.setValue(1.0, juce::dontSendNotification);
        
        metadataLabel.setText("", juce::dontSendNotification);
        mixerMetadataLabel.setText("", juce::dontSendNotification);
        mixerMetadataLabel.setVisible(false);
        mixerVolumeSlider.setVisible(false);
        mixerPositionSlider.setVisible(false);
        mixerSpeedSlider.setVisible(false);
        mixerSpeedLabel.setVisible(false);
        mixerCurrentTimeLabel.setVisible(false);
        mixerTotalTimeLabel.setVisible(false);
        track2Label.setVisible(false);
        mixerWaveformDisplay.setVisible(false);
        track1PlayPauseButton.setVisible(false);
        track2PlayPauseButton.setVisible(false);
        isTrack1Playing = false;
        isTrack2Playing = false;
        
        waveformDisplay.loadWaveform(juce::File());
        mixerWaveformDisplay.loadWaveform(juce::File());
        
        audio.setABLooping(false, 0.0, 0.0);
        waveformDisplay.setABMarkers(false, 0.0, 0.0);
        
        audio.setLooping(false);
        
        markers.clear();
        
        if (playIcon) playPauseButton.setImages(playIcon.get()); 
        else playPauseButton.setButtonText("Play");
        
        resized();
    }
    else if (button == &restartButton || button == &startButton)
    {
        audio.setPosition(0.0);
        positionSlider.setValue(0.0, juce::dontSendNotification);
        currentTimeLabel.setText("0:00", juce::dontSendNotification);
        
        if (audio.hasMixerTrack())
        {
            audio.setMixerPosition(0.0);
            mixerPositionSlider.setValue(0.0, juce::dontSendNotification);
            mixerCurrentTimeLabel.setText("0:00", juce::dontSendNotification);
        }
    }
    else if (button == &endButton)
    {
        playNextInPlaylist();
    }

    else if (button == &loopButton)
    {
        bool newLoopState = !audio.isLooping();
        audio.setLooping(newLoopState);
        loopButton.setAlpha(newLoopState ? 1.0f : 0.5f);
    }
    else if (button == &muteButton)
    {
        audio.toggleMute();
        if (audio.isMuted)
            safeSetButtonImage(muteButton, unmuteIcon, "Unmute");
        else
            safeSetButtonImage(muteButton, muteIcon, "Mute");
    }
    else if (button == &backwardButton)
        audio.jumpBackward(5.0);
    else if (button == &forwardButton)
        audio.jumpForward(5.0);
    else if (button == &abLoopButton)
    {
        if (audio.isABLooping())
        {
            audio.setABLooping(false, 0.0, 0.0);
            waveformDisplay.setABMarkers(false, 0.0, 0.0);
            abLoopButton.setAlpha(0.5f);
        }
        else
        {
            showABLoopDialog();
        }
    }
    else if (button == &playlistButton)
    {
        playlistVisible = !playlistVisible;
        playlistBox.setVisible(playlistVisible);
        if (playlistVisible && playlistFiles.isEmpty())
        {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select audio files...", juce::File(), "*.wav;*.mp3;*.aiff;*.flac");
            fileChooser->launchAsync(
                juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectMultipleItems,
                [this](const juce::FileChooser& chooser)
                {
                    auto results = chooser.getResults();
                    for (auto& file : results)
                        if (file.existsAsFile())
                            playlistFiles.add(file);
                    playlistBox.updateContent();
                    resized();
                    repaint();
                });
        }
        else
        {
            resized();
            repaint();
        }
    }
    else if (button == &mixerButton)
    {
        mixerFileCount = 0;
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select first audio file for mixer...", juce::File(), "*.wav;*.mp3;*.aiff;*.flac");
        fileChooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file.existsAsFile())
                {
                    pendingMixerFile1 = file;
                    mixerFileCount++;
                    fileChooser = std::make_unique<juce::FileChooser>(
                        "Select second audio file for mixer...", juce::File(), "*.wav;*.mp3;*.aiff;*.flac");
                    fileChooser->launchAsync(
                        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this](const juce::FileChooser& chooser2)
                        {
                            auto file2 = chooser2.getResult();
                            if (file2.existsAsFile())
                            {
                                pendingMixerFile2 = file2;
                                mixerFileCount++;
                                
                                audio.loadFile(pendingMixerFile1);
                                audio.loadMixerFile(pendingMixerFile2);
                                
                                updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
                                updateMixerMetadata(audio.getMixerTitle(), audio.getMixerArtist(), audio.getMixerAlbum(), audio.getMixerDuration());
                                
                                double totalLength = audio.getLengthInSeconds();
                                positionSlider.setRange(0.0, totalLength, 0.001);
                                totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
                                
                                double mixerLength = audio.getMixerLengthInSeconds();
                                mixerPositionSlider.setRange(0.0, mixerLength, 0.001);
                                mixerTotalTimeLabel.setText(formatTime(mixerLength), juce::dontSendNotification);
                                
                                waveformDisplay.loadWaveform(pendingMixerFile1);
                                mixerWaveformDisplay.loadWaveform(pendingMixerFile2);
                                
                                mixerVolumeSlider.setVisible(true);
                                mixerMetadataLabel.setVisible(true);
                                mixerPositionSlider.setVisible(true);
                                mixerSpeedSlider.setVisible(true);
                                mixerSpeedLabel.setVisible(true);
                                mixerCurrentTimeLabel.setVisible(true);
                                mixerTotalTimeLabel.setVisible(true);
                                track2Label.setVisible(true);
                                mixerWaveformDisplay.setVisible(true);
                                track1PlayPauseButton.setVisible(true);
                                track1MuteButton.setVisible(true);
                                track1ForwardButton.setVisible(true);
                                track1BackwardButton.setVisible(true);
                                track2PlayPauseButton.setVisible(true);
                                track2MuteButton.setVisible(true);
                                track2ForwardButton.setVisible(true);
                                track2BackwardButton.setVisible(true);

                                audio.play();
                                isPlaying = true;
                                if (pauseIcon) playPauseButton.setImages(pauseIcon.get());
                                else playPauseButton.setButtonText("Pause");
                                
                                resized();
                                repaint();
                            }
                        });
                }
            });
    }
    else if (button == &saveButton)
    {
        saveSession();
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Session Saved", "Your session has been saved successfully!");
    }
    else if (button == &markerButton)
    {
        showMarkersDialog();
    }
    else if (button == &track1PlayPauseButton)
    {
        if (isTrack1Playing)
        {
            audio.pauseTrack1();
            isTrack1Playing = false;
            if (playIcon) track1PlayPauseButton.setImages(playIcon.get());
        }
        else
        {
            audio.playTrack1();
            isTrack1Playing = true;
            if (pauseIcon) track1PlayPauseButton.setImages(pauseIcon.get());
        }
    }
    else if (button == &track1MuteButton)
    {
        audio.toggleTrack1Mute();
        isTrack1Muted = !isTrack1Muted;
        if (isTrack1Muted)
            safeSetButtonImage(track1MuteButton, unmuteIcon, "Unmute T1");
        else
            safeSetButtonImage(track1MuteButton, muteIcon, "Mute T1");
    }
    else if (button == &track1ForwardButton)
    {
        audio.track1JumpForward(10.0);
    }
    else if (button == &track1BackwardButton)
    {
        audio.track1JumpBackward(10.0);
    }
    else if (button == &track2PlayPauseButton)
    {
        if (isTrack2Playing)
        {
            audio.pauseTrack2();
            isTrack2Playing = false;
            if (playIcon) track2PlayPauseButton.setImages(playIcon.get());
        }
        else
        {
            audio.playTrack2();
            isTrack2Playing = true;
            if (pauseIcon) track2PlayPauseButton.setImages(pauseIcon.get());
        }
    }
    else if (button == &track2MuteButton)
    {
        audio.toggleTrack2Mute();
        isTrack2Muted = !isTrack2Muted;
        if (isTrack2Muted)
            safeSetButtonImage(track2MuteButton, unmuteIcon, "Unmute T2");
        else
            safeSetButtonImage(track2MuteButton, muteIcon, "Mute T2");
    }
    else if (button == &track2ForwardButton)
    {
        audio.track2JumpForward(10.0);
    }
    else if (button == &track2BackwardButton)
    {
        audio.track2JumpBackward(10.0);
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
        audio.setGain(static_cast<float>(volumeSlider.getValue()));
    else if (slider == &mixerVolumeSlider)
        audio.setMixerGain(static_cast<float>(mixerVolumeSlider.getValue()));
    else if (slider == &positionSlider)
    {
        if (isDraggingPosition)
        {
            double target = positionSlider.getValue();
            audio.setPosition(target);
            currentTimeLabel.setText(formatTime(audio.transportSource.getCurrentPosition()), juce::dontSendNotification);
        }
    }
    else if (slider == &mixerPositionSlider)
    {
        if (isDraggingMixerPosition)
        {
            double target = mixerPositionSlider.getValue();
            audio.setMixerPosition(target);
            mixerCurrentTimeLabel.setText(formatTime(audio.mixerTransportSource.getCurrentPosition()), juce::dontSendNotification);
        }
    }
    else if (slider == &speedSlider)
    {
        audio.setSpeed(speedSlider.getValue());
    }
    else if (slider == &mixerSpeedSlider)
    {
        audio.setMixerSpeed(mixerSpeedSlider.getValue());
    }
}

void PlayerGUI::updateMetadata(const juce::String& title, const juce::String& artist, const juce::String& album, double duration)
{
    juce::String safeTitle = title.isNotEmpty() ? title : "Unknown";
    juce::String safeArtist = artist.isNotEmpty() ? artist : "Unknown";
    juce::String formattedDuration = (duration > 0) ? juce::String((int)(duration / 60)) + ":" + juce::String((int)duration % 60).paddedLeft('0', 2) : "0:00";
    juce::String info = safeTitle + " - " + safeArtist + " [" + formattedDuration + "]";
    metadataLabel.setText(info, juce::dontSendNotification);
}

void PlayerGUI::updateMixerMetadata(const juce::String& title, const juce::String& artist, const juce::String& album, double duration)
{
    juce::String safeTitle = title.isNotEmpty() ? title : "Unknown";
    juce::String safeArtist = artist.isNotEmpty() ? artist : "Unknown";
    juce::String formattedDuration = (duration > 0) ? juce::String((int)(duration / 60)) + ":" + juce::String((int)duration % 60).paddedLeft('0', 2) : "0:00";
    juce::String info = safeTitle + " - " + safeArtist + " [" + formattedDuration + "]";
    mixerMetadataLabel.setText(info, juce::dontSendNotification);
}

void PlayerGUI::refreshPlaylist()
{
    playlistBox.updateContent();
    playlistBox.repaint();
}

int PlayerGUI::getNumRows()
{
    if (playlistVisible)
        return playlistFiles.size();
    else
        return 0;
}

void PlayerGUI::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    g.fillAll(rowIsSelected ? juce::Colour::fromString("#FF2E3648") : juce::Colour::fromString("#FF1A1F2B"));
    g.setColour(juce::Colour::fromString("#FFFEE715"));
    if (rowNumber >= 0 && rowNumber < playlistFiles.size())
        g.drawText(playlistFiles[rowNumber].getFileNameWithoutExtension(), 4, 0, width - 8, height, juce::Justification::centredLeft);
}

void PlayerGUI::listBoxItemDoubleClicked(int rowNumber, const juce::MouseEvent&)
{
    if (rowNumber >= 0 && rowNumber < playlistFiles.size())
    {
        auto file = playlistFiles[rowNumber];
        currentPlaylistIndex = rowNumber;
        if (file.existsAsFile())
        {
            audio.loadFile(file);
            updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
            double totalLength = audio.getLengthInSeconds();
            positionSlider.setRange(0.0, totalLength, 0.001);
            totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
            waveformDisplay.loadWaveform(file);
            audio.play();
            isPlaying = true;
            if (pauseIcon) playPauseButton.setImages(pauseIcon.get()); else playPauseButton.setButtonText("Pause");
        }
    }
}

void PlayerGUI::saveSession()
{
    juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AudioPlayer").getChildFile("session.xml");
    
    sessionFile.getParentDirectory().createDirectory();
    
    juce::XmlElement root("Session");
    
    if (audio.getCurrentFile().existsAsFile())
    {
        root.setAttribute("lastFile", audio.getCurrentFile().getFullPathName());
        root.setAttribute("lastPosition", audio.getCurrentPosition());
        root.setAttribute("volume", volumeSlider.getValue());
        root.setAttribute("speed", speedSlider.getValue());
    }
    
    juce::XmlElement* markersElement = root.createNewChildElement("Markers");
    for (const auto& marker : markers)
    {
        juce::XmlElement* markerElement = markersElement->createNewChildElement("Marker");
        markerElement->setAttribute("timestamp", marker.timestamp);
        markerElement->setAttribute("label", marker.label);
    }
    
    root.writeTo(sessionFile);
}

void PlayerGUI::loadSession()
{
    juce::File sessionFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("AudioPlayer").getChildFile("session.xml");
    
    if (!sessionFile.existsAsFile())
        return;
    
    std::unique_ptr<juce::XmlElement> root = juce::parseXML(sessionFile);
    if (!root || root->getTagName() != "Session")
        return;
    
    juce::String lastFilePath = root->getStringAttribute("lastFile");
    if (lastFilePath.isNotEmpty())
    {
        juce::File lastFile(lastFilePath);
        if (lastFile.existsAsFile())
        {
            audio.loadFile(lastFile);
            updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
            double totalLength = audio.getLengthInSeconds();
            positionSlider.setRange(0.0, totalLength, 0.001);
            totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
            waveformDisplay.loadWaveform(lastFile);
            
            double lastPosition = root->getDoubleAttribute("lastPosition", 0.0);
            audio.setPosition(lastPosition);
            positionSlider.setValue(lastPosition, juce::dontSendNotification);
            
            volumeSlider.setValue(root->getDoubleAttribute("volume", 0.8), juce::dontSendNotification);
            speedSlider.setValue(root->getDoubleAttribute("speed", 1.0), juce::dontSendNotification);
        }
    }
    
    markers.clear();
    juce::XmlElement* markersElement = root->getChildByName("Markers");
    if (markersElement)
    {
        for (auto* markerElement : markersElement->getChildIterator())
        {
            if (markerElement->getTagName() == "Marker")
            {
                Marker marker;
                marker.timestamp = markerElement->getDoubleAttribute("timestamp");
                marker.label = markerElement->getStringAttribute("label");
                markers.add(marker);
            }
        }
    }
    waveformDisplay.setMarkers(markers);
}

void PlayerGUI::addMarker()
{
    double currentTime = audio.getCurrentPosition();
    int markerNumber = markers.size() + 1;
    
    Marker marker;
    marker.timestamp = currentTime;
    marker.label = "Marker " + juce::String(markerNumber) + " (" + formatTime(currentTime) + ")";
    
    markers.add(marker);
    
    waveformDisplay.setMarkers(markers);
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon, "Marker Added", 
        "Added: " + marker.label);
}

void PlayerGUI::showMarkersDialog()
{
    class MarkersListDialog : public juce::Component
    {
    public:
        MarkersListDialog(juce::Array<Marker>& markersRef, PlayerAudio& audioRef, PlayerGUI& guiRef)
            : markers(markersRef), audio(audioRef), gui(guiRef)
        {
            setSize(400, 350);
            
            for (int i = 0; i < markers.size(); i++)
            {
                auto* btn = new juce::TextButton(markers[i].label);
                btn->onClick = [this, i]()
                {
                    audio.setPosition(markers[i].timestamp);
                    if (!audio.isPlaying())
                        audio.play();
                };
                addAndMakeVisible(btn);
                markerButtons.add(btn);
                
                auto* delBtn = new juce::TextButton("X");
                delBtn->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
                delBtn->onClick = [this, i]()
                {
                    markers.remove(i);
                    if (auto* parent = getParentComponent())
                        parent->exitModalState(0);
                    gui.waveformDisplay.setMarkers(markers);
                    gui.showMarkersDialog();
                };
                addAndMakeVisible(delBtn);
                deleteButtons.add(delBtn);
            }
            
            addMarkerBtn.setButtonText("Add Marker");
            addMarkerBtn.setColour(juce::TextButton::buttonColourId, juce::Colour::fromString("#FFFEE715"));
            addMarkerBtn.setColour(juce::TextButton::textColourOffId, juce::Colour::fromString("#FF101820"));
            addMarkerBtn.onClick = [this]()
            {
                gui.addMarker();
                if (auto* parent = getParentComponent())
                    parent->exitModalState(0);
                gui.showMarkersDialog();
            };
            addAndMakeVisible(addMarkerBtn);
            
            closeButton.setButtonText("Close");
            closeButton.onClick = [this]()
            {
                if (auto* parent = getParentComponent())
                    parent->exitModalState(0);
            };
            addAndMakeVisible(closeButton);
        }
        
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colour::fromString("#FF101820"));
            g.setColour(juce::Colour::fromString("#FFFEE715"));
            g.drawRect(getLocalBounds(), 2);
        }
        
        void resized() override
        {
            auto area = getLocalBounds().reduced(10);
            
            for (int i = 0; i < markerButtons.size(); i++)
            {
                auto row = area.removeFromTop(40);
                deleteButtons[i]->setBounds(row.removeFromRight(40));
                row.removeFromRight(5);
                markerButtons[i]->setBounds(row);
                area.removeFromTop(5);
            }
            
            area.removeFromTop(10);
            auto buttonRow = area.removeFromTop(40);
            addMarkerBtn.setBounds(buttonRow.removeFromLeft(150));
            buttonRow.removeFromLeft(10);
            closeButton.setBounds(buttonRow);
        }
        
    private:
        juce::Array<Marker>& markers;
        PlayerAudio& audio;
        PlayerGUI& gui;
        juce::OwnedArray<juce::TextButton> markerButtons;
        juce::OwnedArray<juce::TextButton> deleteButtons;
        juce::TextButton addMarkerBtn;
        juce::TextButton closeButton;
    };
    
    auto* dialog = new MarkersListDialog(markers, audio, *this);
    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(dialog);
    options.dialogTitle = "Markers";
    options.dialogBackgroundColour = juce::Colour::fromString("#FF101820");
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    options.launchAsync();
}

bool PlayerGUI::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        if (audio.isPlaying())
        {
            audio.pause();
            isPlaying = false;
            if (playIcon) playPauseButton.setImages(playIcon.get()); 
            else playPauseButton.setButtonText("Play");
        }
        else
        {
            audio.play();
            isPlaying = true;
            if (pauseIcon) playPauseButton.setImages(pauseIcon.get()); 
            else playPauseButton.setButtonText("Pause");
        }
        return true;
    }
    else if (key == juce::KeyPress::leftKey)
    {
        audio.jumpBackward(5.0);
        return true;
    }
    else if (key == juce::KeyPress::rightKey)
    {
        audio.jumpForward(5.0);
        return true;
    }
    else if (key == juce::KeyPress::upKey)
    {
        float newVolume = juce::jmin(1.0f, (float)volumeSlider.getValue() + 0.05f);
        volumeSlider.setValue(newVolume);
        audio.setGain(newVolume);
        return true;
    }
    else if (key == juce::KeyPress::downKey)
    {
        float newVolume = juce::jmax(0.0f, (float)volumeSlider.getValue() - 0.05f);
        volumeSlider.setValue(newVolume);
        audio.setGain(newVolume);
        return true;
    }
    else if (key.getTextCharacter() == 'm' || key.getTextCharacter() == 'M')
    {
        audio.toggleMute();
        if (audio.isMuted)
            safeSetButtonImage(muteButton, unmuteIcon, "Unmute");
        else
            safeSetButtonImage(muteButton, muteIcon, "Mute");
        return true;
    }
    else if (key.getTextCharacter() == 'l' || key.getTextCharacter() == 'L')
    {
        bool newLoopState = !audio.isLooping();
        audio.setLooping(newLoopState);
        loopButton.setAlpha(newLoopState ? 1.0f : 0.5f);
        return true;
    }
    else if (key.getTextCharacter() == 'r' || key.getTextCharacter() == 'R')
    {
        audio.setPosition(0.0);
        positionSlider.setValue(0.0, juce::dontSendNotification);
        currentTimeLabel.setText("0:00", juce::dontSendNotification);
        return true;
    }
    
    return false;
}

void PlayerGUI::playNextInPlaylist()
{
    if (playlistFiles.isEmpty())
        return;
    
    currentPlaylistIndex++;
    if (currentPlaylistIndex >= playlistFiles.size())
        currentPlaylistIndex = 0;
    
    auto file = playlistFiles[currentPlaylistIndex];
    if (file.existsAsFile())
    {
        audio.loadFile(file);
        updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
        double totalLength = audio.getLengthInSeconds();
        positionSlider.setRange(0.0, totalLength, 0.001);
        totalTimeLabel.setText(formatTime(totalLength), juce::dontSendNotification);
        waveformDisplay.loadWaveform(file);
        audio.play();
        isPlaying = true;
        if (pauseIcon) playPauseButton.setImages(pauseIcon.get());
        else playPauseButton.setButtonText("Pause");
        
        playlistBox.selectRow(currentPlaylistIndex);
    }
}
