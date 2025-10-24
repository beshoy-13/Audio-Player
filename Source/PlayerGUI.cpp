#include "PlayerGUI.h"

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

    for (auto* b : { &loadButton, &restartButton, &stopButton, &playPauseButton, &startButton, &endButton, &loopButton, &muteButton, &backwardButton, &forwardButton })
    {
        addAndMakeVisible(b);
        b->addListener(this);
    }

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8, juce::dontSendNotification);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    metadataLabel.setJustificationType(juce::Justification::centredLeft);
    metadataLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(metadataLabel);
}

PlayerGUI::~PlayerGUI() {}

void PlayerGUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
}

void PlayerGUI::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto top = area.removeFromTop(70);
    int btnW = 80, gap = 8;

    loadButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    restartButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    stopButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    playPauseButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    startButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    endButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    loopButton.setBounds(top.removeFromLeft(btnW));
    muteButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    backwardButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    forwardButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);


    area.removeFromTop(8);
    volumeSlider.setBounds(area.removeFromTop(28));
    metadataLabel.setBounds(area.reduced(2));
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
    {
        audio.jumpBackward(5.0);
    }
    else if (button == &forwardButton)
    {
        audio.jumpForward(5.0);
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider) audio.setGain((float)volumeSlider.getValue());
}

void PlayerGUI::safeSetButtonImage(juce::DrawableButton& btn, std::unique_ptr<juce::Drawable>& drawable, const juce::String& fallbackText)
{
    btn.setButtonStyle(juce::DrawableButton::ImageFitted);
    if (drawable) { btn.setImages(drawable.get()); btn.setButtonText(""); }
    else btn.setButtonText(fallbackText);
}

