/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
    setSize(800, 600);
    setFramesPerSecond(60); // This sets the frequency of the update calls.

    listBox.setModel(&model);
    listBox.getVerticalScrollBar().moveScrollbarInPages(1, dontSendNotification);
    listBox.getViewport()->setScrollBarsShown(false,false);

    addAndMakeVisible(listBox);
    addAndMakeVisible(addNew);
    
    
    testComp.setSize(getWidth()*2, getHeight()*.9);
    viewport.setViewedComponent(&testComp);
    addAndMakeVisible(viewport);


    model.addStringItem("LAw");
    model.addStringItem("LAw");
    model.addStringItem("LAw");
    model.addStringItem("LAw");
    model.addStringItem("LAw");
    listBox.updateContent();

    addNew.onClick = [this] {
        model.addStringItem("LAw");
        listBox.updateContent();
    };
}

MainComponent::~MainComponent() {
}

//==============================================================================
void MainComponent::update() {
    // This function is called at the frequency specified by the setFramesPerSecond() call
    // in the constructor. You can use it to update counters, animate values, etc.
}

//==============================================================================
void MainComponent::paint(Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized() {
    //listBox.setRowHeight(300);
    
    auto r = getLocalBounds();
    listBox.setColWidth(300);
    listBox.setBounds(r.removeFromTop(getWidth()/5));
    viewport.setBounds(r);
    addNew.setBounds(100, 100, 100, 50);
}
