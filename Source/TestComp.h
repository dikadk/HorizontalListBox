/*
  ==============================================================================

    TestComp.h
    Created: 12 Jun 2019 10:32:23am
    Author:  Dmytro Kiro

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct TestComp: Component{
    
    TestComp(){

    };
    
    void paint(Graphics& g) override{
        g.fillAll(Colours::aqua);

        auto r = getLocalBounds();
        g.drawFittedText("Little", r.removeFromLeft(getWidth()/2), Justification::Flags::centred, 1);
        g.drawRect(r);

        g.setColour(Colours::green);
        g.fillRect(r);
        g.setColour(Colours::black);
        g.drawFittedText("Dicky", r, Justification::Flags::centred, 1);
        g.drawRect(r);
    };
    
    void resized() override{
        //constrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(),getHeight(), getWidth());
    };

    void mouseDown (const MouseEvent& e) override
    {
        // Prepares our dragger to drag this Component
        //dragger.startDraggingComponent (this, e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        // Moves this Component according to the mouse drag event and applies our constraints to it
        //dragger.dragComponent (this, e, &constrainer);
        //getParentComponent()->mouseDrag(e);
    }

    ComponentBoundsConstrainer constrainer;
    ComponentDragger dragger;

};
