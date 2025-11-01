#include "PlayerGUI.h"

static juce::String formatDuration(double seconds)
{
    int totalSecs = static_cast<int>(seconds);
    int mins = totalSecs / 60;
    int secs = totalSecs % 60;
    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
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

PlayerGUI::PlayerGUI(PlayerAudio& audioRef) : audio(audioRef)
{
    loadIcon     = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("upload")));
    restartIcon  = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("restart")));
    stopIcon     = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("stop")));
    playIcon     = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("start")));
    pauseIcon    = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("pause")));
    startIcon    = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("startsong")));
    endIcon      = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("endsong")));
    loopIcon     = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("loop")));
    muteIcon     = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("mute")));
    unmuteIcon   = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("unmute")));
    backwardIcon = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("backward")));
    forwardIcon  = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("forward")));
    playlistIcon = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("playlist")));
    mixerIcon    = juce::Drawable::createFromSVG(*juce::parseXML(getSVGFile("mixer")));
    
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
    
    for (auto* b : { &loadButton, &restartButton, &stopButton, &playPauseButton, &startButton, &endButton,
                     &loopButton, &muteButton, &backwardButton, &forwardButton, &playlistButton, &mixerButton })
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
    
    metadataLabel.setJustificationType(juce::Justification::centredLeft);
    metadataLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    addAndMakeVisible(metadataLabel);
    
    mixerMetadataLabel.setJustificationType(juce::Justification::centredLeft);
    mixerMetadataLabel.setColour(juce::Label::textColourId, juce::Colour::fromString("#FFFEE715"));
    mixerMetadataLabel.setVisible(false);
    addAndMakeVisible(mixerMetadataLabel);
    
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
}

PlayerGUI::~PlayerGUI() {}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("#FF101820"));
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(10);
    int btnH = 70;
    int btnW = 70;
    int gap = 10;
    
    auto top = area.removeFromTop(btnH);
    loadButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    playPauseButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    stopButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    restartButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    startButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    endButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    loopButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    muteButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    backwardButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    forwardButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    playlistButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    mixerButton.setBounds(top.removeFromLeft(btnW));
    
    area.removeFromTop(10);
    volumeSlider.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);
    
    auto metadataArea = area.removeFromTop(30);
    metadataLabel.setBounds(metadataArea.reduced(4));
    
    if (audio.hasMixerTrack())
    {
        area.removeFromTop(10);
        mixerVolumeSlider.setBounds(area.removeFromTop(40));
        area.removeFromTop(10);
        auto mixerMetadataArea = area.removeFromTop(30);
        mixerMetadataLabel.setBounds(mixerMetadataArea.reduced(4));
    }
    
    if (playlistVisible)
        playlistBox.setBounds(area.removeFromBottom(180));
    else
        playlistBox.setBounds(0, 0, 0, 0);
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select an audio file...", juce::File(), "*.wav;*.mp3;*.aiff");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                if (file.existsAsFile())
                {
                    audio.loadFile(file);
                    updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
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
        isPlaying = false;
        if (playIcon) playPauseButton.setImages(playIcon.get()); else playPauseButton.setButtonText("Play");
    }
    else if (button == &restartButton || button == &startButton)
        audio.setPosition(0.0);
    else if (button == &endButton)
    {
        double len = audio.getLengthInSeconds();
        if (len > 0.1) audio.setPosition(len - 0.1);
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
    else if (button == &playlistButton)
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
                playlistVisible = true;
                addAndMakeVisible(playlistBox);
                resized();
                repaint();
            });
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
                                
                                mixerVolumeSlider.setVisible(true);
                                mixerMetadataLabel.setVisible(true);
                                
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
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
        audio.setGain((float)volumeSlider.getValue());
    else if (slider == &mixerVolumeSlider)
        audio.setMixerGain((float)mixerVolumeSlider.getValue());
}

void PlayerGUI::updateMetadata(const juce::String& title,
                               const juce::String& artist,
                               const juce::String& album,
                               double duration)
{
    juce::String safeTitle  = title.isNotEmpty()  ? title  : "Unknown";
    juce::String safeArtist = artist.isNotEmpty() ? artist : "Unknown";
    juce::String formattedDuration = (duration > 0) ? juce::String((int)(duration / 60)) + ":" + juce::String((int)duration % 60).paddedLeft('0', 2) : "0:00";
    juce::String info = safeTitle + " - " + safeArtist + " [" + formattedDuration + "]";
    metadataLabel.setText(info, juce::dontSendNotification);
}

void PlayerGUI::updateMixerMetadata(const juce::String& title,
                                    const juce::String& artist,
                                    const juce::String& album,
                                    double duration)
{
    juce::String safeTitle  = title.isNotEmpty()  ? title  : "Unknown";
    juce::String safeArtist = artist.isNotEmpty() ? artist : "Unknown";
    juce::String formattedDuration = (duration > 0) ? juce::String((int)(duration / 60)) + ":" + juce::String((int)duration % 60).paddedLeft('0', 2) : "0:00";
    juce::String info = safeTitle + " - " + safeArtist + " [" + formattedDuration + "]";
    mixerMetadataLabel.setText(info, juce::dontSendNotification);
}

void PlayerGUI::safeSetButtonImage(juce::DrawableButton& btn, std::unique_ptr<juce::Drawable>& drawable, const juce::String& fallbackText)
{
    btn.setButtonStyle(juce::DrawableButton::ImageFitted);
    if (drawable) { btn.setImages(drawable.get()); btn.setButtonText(""); }
    else btn.setButtonText(fallbackText);
}

void PlayerGUI::refreshPlaylist()
{
    playlistBox.updateContent();
    playlistBox.repaint();
}

int PlayerGUI::getNumRows()
{
    return playlistFiles.size();
}

void PlayerGUI::paintListBoxItem(int rowNumber, juce::Graphics& g,
                                 int width, int height, bool rowIsSelected)
{
    g.fillAll(rowIsSelected
        ? juce::Colour::fromString("#FF2E3648")
        : juce::Colour::fromString("#FF1A1F2B"));
    g.setColour(juce::Colour::fromString("#FFFEE715"));
    if (rowNumber >= 0 && rowNumber < playlistFiles.size())
        g.drawText(playlistFiles[rowNumber].getFileNameWithoutExtension(),
                   4, 0, width - 8, height, juce::Justification::centredLeft);
}

void PlayerGUI::listBoxItemDoubleClicked(int rowNumber, const juce::MouseEvent&)
{
    if (rowNumber >= 0 && rowNumber < playlistFiles.size())
    {
        auto file = playlistFiles[rowNumber];
        if (file.existsAsFile())
        {
            audio.loadFile(file);
            audio.play();
            isPlaying = true;
            if (pauseIcon) playPauseButton.setImages(pauseIcon.get());
            else playPauseButton.setButtonText("Pause");
            updateMetadata(audio.getTitle(), audio.getArtist(), audio.getAlbum(), audio.getDuration());
        }
    }
}
