/*
  ==============================================================================

    CustomViewport.h
    Created: 12 Jun 2019 10:39:02am
    Author:  Dmytro Kiro

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

struct CustomViewport : Viewport {

    int numOfItems = 2;

    void visibleAreaChanged(const Rectangle<int> &newVisibleArea) override {
        
        if (isCurrentlyScrollingOnDrag())
            return;

        Component *c = getViewedComponent();

        int width = c->getLocalBounds().getWidth();

        int scrollX = newVisibleArea.getX();
        int featureWidth = width / numOfItems;

        int activeFeature = ((scrollX + featureWidth / 2) / featureWidth);
        int scrollTo = activeFeature * featureWidth;

        DBG("New Area X:" << scrollTo);

        setViewPosition(scrollTo, 0);
        //scrollRectIntoView(newVisibleArea.withX(scrollTo), Point<int>());
    }

    void scrollRectIntoView(const Rectangle<int>& rect, const Point<int>& margin )
    {
            Rectangle<int> viewRect(getViewPositionX(), getViewPositionY(), getViewWidth(), getViewHeight());
            Rectangle<int>rectWithMargins = rect.expanded(margin.x, margin.y);

            Point<int> movedBy = rectWithMargins.constrainedWithin( viewRect ).getPosition() - rectWithMargins.getPosition();

            setViewPosition(getViewPosition() - movedBy);
    }
};
