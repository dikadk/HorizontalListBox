#include <utility>

/*
  ==============================================================================

    TestListBoxModel.h
    Created: 11 Jun 2019 7:48:24pm
    Author:  home

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

struct TestListBoxModel: public ListBoxModel{

    TestListBoxModel(){
        data = StringArray("One", "Two");
    }


    int getNumRows() override {
        return data.size();
    }

    void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override {
        if(rowIsSelected)
            g.fillAll(Colours::aliceblue);

        g.setColour (LookAndFeel::getDefaultLookAndFeel().findColour (Label::textColourId));
        g.setFont (height * 0.2f);

        g.drawText (data[rowNumber] + String (rowNumber + 1),
                    5, 0, width, height,
                    Justification::centredLeft, true);
        g.drawRect(0,0,width, height);

    }

    void addStringItem(String item){
        data.add(std::move(item));
    }

    StringArray data;
};
