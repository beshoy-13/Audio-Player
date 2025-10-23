#include "PlayerGUI.h"
#include "BinaryData.h"

PlayerGUI::PlayerGUI(PlayerAudio& audioRef) : audio(audioRef)
{
    std::unique_ptr<juce::XmlElement> xml;
    int svgDataSize = 0;
    const char* svgData = nullptr;

    svgData = BinaryData::getNamedResource("upload_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) loadIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load upload from Binary Data!");

    svgData = BinaryData::getNamedResource("restart_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) restartIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load restart from Binary Data!");

    svgData = BinaryData::getNamedResource("stop_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) stopIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load stop from Binary Data!");

    svgData = BinaryData::getNamedResource("start_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) playIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load start from Binary Data!");

    svgData = BinaryData::getNamedResource("pause_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) pauseIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load pause from Binary Data!");

    svgData = BinaryData::getNamedResource("startsong_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) startIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load startsong from Binary Data!");

    svgData = BinaryData::getNamedResource("endsong_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) endIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load endsong from Binary Data!");

    svgData = BinaryData::getNamedResource("mute_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) muteIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load mute from Binary Data!");

    svgData = BinaryData::getNamedResource("unmute_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) unmuteIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load unmute from Binary Data!");

    svgData = BinaryData::getNamedResource("replay_10_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) backwardIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load backward icon!");

    svgData = BinaryData::getNamedResource("forward_10_svg", svgDataSize);
    xml = juce::parseXML(svgData);
    if (xml != nullptr) forwardIcon = juce::Drawable::createFromSVG(*xml);
    else juce::Logger::writeToLog("ERROR: Failed to load forward icon!");

    safeSetButtonImage(loadButton, loadIcon, "Load");
    safeSetButtonImage(restartButton, restartIcon, "Restart");
    safeSetButtonImage(stopButton, stopIcon, "Stop");
    safeSetButtonImage(playPauseButton, playIcon, "Play");
    safeSetButtonImage(startButton, startIcon, "Start");
    safeSetButtonImage(endButton, endIcon, "End");
    safeSetButtonImage(muteButton, muteIcon, "Mute");
    safeSetButtonImage(backwardButton, backwardIcon, "⏪ 10s");
    safeSetButtonImage(forwardButton, forwardIcon, "⏩ 10s");

    juce::Button* allButtons[] =
    {
        &loadButton,
        &restartButton,
        &stopButton,
        &playPauseButton,
        &startButton,
        &endButton,
        &muteButton,
        &backwardButton,
        &forwardButton
    };

    for (auto* b : allButtons)
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
    auto top = area.removeFromTop(100);

    int btnW = 80, gap = 8;

    loadButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    restartButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    stopButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    playPauseButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    startButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
    endButton.setBounds(top.removeFromLeft(btnW)); top.removeFromLeft(gap);
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
            audio.stop();
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
    else if (button == &restartButton || button == &startButton) audio.setPosition(0.0);
    else if (button == &endButton)
    {
        double len = audio.getLengthInSeconds();
        if (len > 0.1) audio.setPosition(len - 0.1);
    }

    else if (button == &backwardButton)
    {
        audio.jumpBackward(10.0);
    }
    else if (button == &forwardButton)
    {
        audio.jumpForward(10.0);
    }

    else if (button == &muteButton)
    {
        audio.toggleMute();

        if (audio.isMuted)
        {
            safeSetButtonImage(muteButton, unmuteIcon, "Unmute");
        }
        else
        {
            safeSetButtonImage(muteButton, muteIcon, "Mute");
        }
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