/*
  ==============================================================================

    HorizonalListBox.cpp
    Created: 13 Jun 2019 3:27:34pm
    Author:  Dmytro Kiro

  ==============================================================================
*/

#include "HorizontalListBox.h"


class HorizontalListBox::ColumnComponent : public Component,
                                           public TooltipClient {
public:
    ColumnComponent(HorizontalListBox &lb) : owner(lb) {}

    void paint(Graphics &g) override {
        if (auto *m = owner.getModel())
            m->paintListBoxItem(col, g, getWidth(), getHeight(), selected);
    }

    void update(const int newCol, const bool nowSelected) {
        if (col != newCol || selected != nowSelected) {
            repaint();
            col = newCol;
            selected = nowSelected;
        }

        if (auto *m = owner.getModel()) {
            setMouseCursor(m->getMouseCursorForRow(col));

            customComponent.reset(m->refreshComponentForRow(newCol, nowSelected, customComponent.release()));

            if (customComponent != nullptr) {
                addAndMakeVisible(customComponent.get());
                customComponent->setBounds(getLocalBounds());
            }
        }
    }

    void performSelection(const MouseEvent &e, bool isMouseUp) {
        owner.selectColsBasedOnModifierKeys(col, e.mods, isMouseUp);

        if (auto *m = owner.getModel())
            m->listBoxItemClicked(col, e);
    }

    bool isInDragToScrollViewport() const noexcept {
        if (auto *vp = owner.getViewport())
            return vp->isScrollOnDragEnabled() && (vp->canScrollVertically() || vp->canScrollHorizontally());

        return false;
    }

    void mouseDown(const MouseEvent &e) override {
        isDragging = false;
        isDraggingToScroll = false;
        selectColOnMouseUp = false;

        if (isEnabled()) {
            if (owner.selectOnMouseDown && !(selected || isInDragToScrollViewport()))
                performSelection(e, false);
            else
                selectColOnMouseUp = true;
        }
    }

    void mouseUp(const MouseEvent &e) override {
        if (isEnabled() && selectColOnMouseUp && !(isDragging || isDraggingToScroll))
            performSelection(e, true);
    }

    void mouseDoubleClick(const MouseEvent &e) override {
        if (isEnabled())
            if (auto *m = owner.getModel())
                m->listBoxItemDoubleClicked(col, e);
    }

    void mouseDrag(const MouseEvent &e) override {
        if (auto *m = owner.getModel()) {
            if (isEnabled() && e.mouseWasDraggedSinceMouseDown() && !isDragging) {
                SparseSet<int> colsToDrag;

                if (owner.selectOnMouseDown || owner.isColSelected(col))
                    colsToDrag = owner.getSelectedCols();
                else
                    colsToDrag.addRange(Range<int>::withStartAndLength(col, 1));

                if (colsToDrag.size() > 0) {
                    auto dragDescription = m->getDragSourceDescription(colsToDrag);

                    if (!(dragDescription.isVoid() ||
                          (dragDescription.isString() && dragDescription.toString().isEmpty()))) {
                        isDragging = true;
                        owner.startDragAndDrop(e, colsToDrag, dragDescription, true);
                    }
                }
            }
        }

        if (!isDraggingToScroll)
            if (auto *vp = owner.getViewport())
                isDraggingToScroll = vp->isCurrentlyScrollingOnDrag();
    }

    void resized() override {
        if (customComponent != nullptr)
            customComponent->setBounds(getLocalBounds());
    }

    String getTooltip() override {
        if (auto *m = owner.getModel())
            return m->getTooltipForRow(col);

        return {};
    }

    HorizontalListBox &owner;
    std::unique_ptr<Component> customComponent;
    int col = -1;
    bool selected = false, isDragging = false, isDraggingToScroll = false, selectColOnMouseUp = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColumnComponent)
};


//==============================================================================
class HorizontalListBox::ListViewport : public Viewport {
public:
    ListViewport(HorizontalListBox &lb) : owner(lb) {
        setWantsKeyboardFocus(false);

        auto content = new Component();
        setViewedComponent(content);
        content->setWantsKeyboardFocus(false);
    }

    ColumnComponent *getComponentForCol(const int col) const noexcept {
        return cols[col % jmax(1, cols.size())];
    }

    ColumnComponent *getComponentForColIfOnscreen(const int col) const noexcept {
        return (col >= firstIndex && col < firstIndex + cols.size())
               ? getComponentForCol(col) : nullptr;
    }

    int getColNumberOfComponent(Component *const colComponent) const noexcept {
        const int index = getViewedComponent()->getIndexOfChildComponent(colComponent);
        const int num = cols.size();

        for (int i = num; --i >= 0;)
            if (((firstIndex + i) % jmax(1, num)) == index)
                return firstIndex + i;

        return -1;
    }

    void visibleAreaChanged(const Rectangle<int> &) override {
        updateVisibleArea(true);

        if (auto *m = owner.getModel())
            m->listWasScrolled();
    }

    void updateVisibleArea(const bool makeSureItUpdatesContent) {
        hasUpdated = false;

        auto &content = *getViewedComponent();
        auto newX = content.getX();
        auto newY = content.getY();
        auto newW = owner.totalItems * owner.getColWidth();
        auto newH = jmax(owner.minimumColWidth, getMaximumVisibleHeight());

        if (newX + newW < getMaximumVisibleWidth() && newW > getMaximumVisibleWidth())
            newX = getMaximumVisibleWidth() - newW;

        content.setBounds(newX, newY, newW, newH);

        if (makeSureItUpdatesContent && !hasUpdated)
            updateContents();
    }

    void updateContents() {
        hasUpdated = true;
        auto colW = owner.getColWidth();
        auto &content = *getViewedComponent();

        if (colW > 0) {
            auto x = getViewPositionX();
            auto h = content.getHeight();

            const int numNeeded = 2 + getMaximumVisibleWidth() / colW;
            cols.removeRange(numNeeded, cols.size());

            while (numNeeded > cols.size()) {
                auto newColumn = new ColumnComponent(owner);
                cols.add(newColumn);
                content.addAndMakeVisible(newColumn);
            }

            firstIndex = x / colW;
            firstWholeIndex = (x + colW - 1) / colW;
            lastWholeIndex = (x + getMaximumVisibleWidth() - 1) / colW;

            for (int i = 0; i < numNeeded; ++i) {
                const int col = i + firstIndex;

                if (auto *colComp = getComponentForCol(col)) {
                    colComp->setBounds(col * colW, 0, colW, h);
                    colComp->update(col, owner.isColSelected(col));
                }
            }
        }

        if (owner.headerComponent != nullptr)
            owner.headerComponent->setBounds(owner.outlineThickness,
                                             owner.outlineThickness + content.getY(),
                                             owner.headerComponent->getWidth(),
                                             jmax(owner.getHeight() - owner.outlineThickness * 2,
                                                  content.getHeight()));
    }

    void selectCol(const int col, const int colW, const bool dontScroll,
                   const int lastSelectedCol, const int totalCols, const bool isMouseClick) {
        hasUpdated = false;

        if (col < firstWholeIndex && !dontScroll) {
            setViewPosition(col * colW, getViewPositionY());
        } else if (col >= lastWholeIndex && !dontScroll) {
            const int colsOnScreen = lastWholeIndex - firstWholeIndex;

            if (col >= lastSelectedCol + colsOnScreen
                && colsOnScreen < totalCols - 1
                && !isMouseClick) {
                setViewPosition(jlimit(0, jmax(0, totalCols - colsOnScreen), col) * colW,
                                getViewPositionY());
            } else {
                setViewPosition(jmax(0, (col + 1) * colW - getMaximumVisibleWidth()),
                                getViewPositionY());
            }
        }

        if (!hasUpdated)
            updateContents();
    }

    void scrollToEnsureColIsOnscreen(const int col, const int colw) {
        if (col < firstWholeIndex) {
            setViewPosition(col * colw, getViewPositionY());
        } else if (col >= lastWholeIndex) {
            setViewPosition(
                    jmax(0, (col + 1) * colw - getMaximumVisibleWidth()), getViewPositionY());
        }
    }

    void paint(Graphics &g) override {
        if (isOpaque())
            g.fillAll(owner.findColour(HorizontalListBox::backgroundColourId));
    }

    bool keyPressed(const KeyPress &key) override {
        if (Viewport::respondsToKey(key)) {
            const int allowableMods = owner.multipleSelection ? ModifierKeys::shiftModifier : 0;

            if ((key.getModifiers().getRawFlags() & ~allowableMods) == 0) {
                // we want to avoid these keypresses going to the viewport, and instead allow
                // them to pass up to our listbox..
                return false;
            }
        }

        return Viewport::keyPressed(key);
    }

private:
    HorizontalListBox &owner;
    OwnedArray<ColumnComponent> cols;
    int firstIndex = 0, firstWholeIndex = 0, lastWholeIndex = 0;
    bool hasUpdated = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListViewport)
};

//==============================================================================
struct ListBoxMouseMoveSelector : public MouseListener {
    ListBoxMouseMoveSelector(HorizontalListBox &lb) : owner(lb) {
        owner.addMouseListener(this, true);
    }

    ~ListBoxMouseMoveSelector() override {
        owner.removeMouseListener(this);
    }

    void mouseMove(const MouseEvent &e) override {
        auto pos = e.getEventRelativeTo(&owner).position.toInt();
        owner.selectColumn(owner.getColContainingPosition(pos.x, pos.y), true);
    }

    void mouseExit(const MouseEvent &e) override {
        mouseMove(e);
    }

    HorizontalListBox &owner;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListBoxMouseMoveSelector)
};


//==============================================================================
HorizontalListBox::HorizontalListBox(const String &name, ListBoxModel *const m)
        : Component(name), model(m) {
    viewport.reset(new ListViewport(*this));
    addAndMakeVisible(viewport.get());

    HorizontalListBox::setWantsKeyboardFocus(true);
    HorizontalListBox::colourChanged();
}

HorizontalListBox::~HorizontalListBox() {
    headerComponent.reset();
    viewport.reset();
}

void HorizontalListBox::setModel(ListBoxModel *const newModel) {
    if (model != newModel) {
        model = newModel;
        repaint();
        updateContent();
    }
}

void HorizontalListBox::setMultipleSelectionEnabled(bool b) noexcept { multipleSelection = b; }

void HorizontalListBox::setClickingTogglesColumnSelection(bool b) noexcept { alwaysFlipSelection = b; }

void HorizontalListBox::setColumnSelectedOnMouseDown(bool b) noexcept { selectOnMouseDown = b; }

void HorizontalListBox::setMouseMoveSelectsColumns(bool b) {
    if (b) {
        if (mouseMoveSelector == nullptr)
            mouseMoveSelector.reset(new ListBoxMouseMoveSelector(*this));
    } else {
        mouseMoveSelector.reset();
    }
}

//==============================================================================
void HorizontalListBox::paint(Graphics &g) {
    if (!hasDoneInitialUpdate)
        updateContent();

    g.fillAll(findColour(backgroundColourId));
}

void HorizontalListBox::paintOverChildren(Graphics &g) {
    if (outlineThickness > 0) {
        g.setColour(findColour(outlineColourId));
        g.drawRect(getLocalBounds(), outlineThickness);
    }
}

void HorizontalListBox::resized() {
    viewport->setBoundsInset(
            BorderSize<int>(outlineThickness,
                            outlineThickness + (headerComponent != nullptr ? headerComponent->getWidth() : 0),
                            outlineThickness, outlineThickness));

    viewport->setSingleStepSizes(getColWidth(), 20);

    viewport->updateVisibleArea(false);
}

void HorizontalListBox::visibilityChanged() {
    viewport->updateVisibleArea(true);
}

Viewport *HorizontalListBox::getViewport() const noexcept {
    return viewport.get();
}

//==============================================================================
void HorizontalListBox::updateContent() {
    hasDoneInitialUpdate = true;
    totalItems = (model != nullptr) ? model->getNumRows() : 0;

    bool selectionChanged = false;

    if (selected.size() > 0 && selected[selected.size() - 1] >= totalItems) {
        selected.removeRange({totalItems, std::numeric_limits<int>::max()});
        lastColSelected = getSelectedCol(0);
        selectionChanged = true;
    }

    viewport->updateVisibleArea(isVisible());
    viewport->resized();

    if (selectionChanged && model != nullptr)
        model->selectedRowsChanged(lastColSelected);
}

//==============================================================================
void HorizontalListBox::selectColumn(int colNumber, bool dontScrollToShowThisCol, bool deselectOthersFirst) {
    selectColInternal(colNumber, dontScrollToShowThisCol, deselectOthersFirst, false);
}

void HorizontalListBox::selectColInternal(const int col,
                                          bool dontScroll,
                                          bool deselectOthersFirst,
                                          bool isMouseClick) {
    if (!multipleSelection)
        deselectOthersFirst = true;

    if ((!isColSelected(col))
        || (deselectOthersFirst && getNumSelectedCols() > 1)) {
        if (isPositiveAndBelow(col, totalItems)) {
            if (deselectOthersFirst)
                selected.clear();

            selected.addRange({col, col + 1});

            if (getHeight() == 0 || getWidth() == 0)
                dontScroll = true;

            viewport->selectCol(col, getColWidth(), dontScroll,
                                lastColSelected, totalItems, isMouseClick);

            lastColSelected = col;
            model->selectedRowsChanged(col);
        } else {
            if (deselectOthersFirst)
                deselectAllCols();
        }
    }
}

void HorizontalListBox::deselectCol(const int col) {
    if (selected.contains(col)) {
        selected.removeRange({col, col + 1});

        if (col == lastColSelected)
            lastColSelected = getSelectedCol(0);

        viewport->updateContents();
        model->selectedRowsChanged(lastColSelected);
    }
}

void HorizontalListBox::setSelectedCols(const SparseSet<int> &setOfColumnsToBeSelected,
                                        const NotificationType sendNotificationEventToModel) {
    selected = setOfColumnsToBeSelected;
    selected.removeRange({totalItems, std::numeric_limits<int>::max()});

    if (!isColSelected(lastColSelected))
        lastColSelected = getSelectedCol(0);

    viewport->updateContents();

    if (model != nullptr && sendNotificationEventToModel == sendNotification)
        model->selectedRowsChanged(lastColSelected);
}

SparseSet<int> HorizontalListBox::getSelectedCols() const {
    return selected;
}

void HorizontalListBox::selectRangeOfCols(int firstCol, int lastCol, bool dontScrollToShowThisRange) {
    if (multipleSelection && (firstCol != lastCol)) {
        const int numCols = totalItems - 1;
        firstCol = jlimit(0, jmax(0, numCols), firstCol);
        lastCol = jlimit(0, jmax(0, numCols), lastCol);

        selected.addRange({jmin(firstCol, lastCol),
                           jmax(firstCol, lastCol) + 1});

        selected.removeRange({lastCol, lastCol + 1});
    }

    selectColInternal(lastCol, dontScrollToShowThisRange, false, true);
}

void HorizontalListBox::flipColSelection(const int colNumber) {
    if (isColSelected(colNumber))
        deselectCol(colNumber);
    else
        selectColInternal(colNumber, false, false, true);
}

void HorizontalListBox::deselectAllCols() {
    if (!selected.isEmpty()) {
        selected.clear();
        lastColSelected = -1;

        viewport->updateContents();

        if (model != nullptr)
            model->selectedRowsChanged(lastColSelected);
    }
}

void HorizontalListBox::selectColsBasedOnModifierKeys(int colThatWasClickedOn,
                                                      ModifierKeys modifiers,
                                                      bool isMouseUpEvent) {
    if (multipleSelection && (modifiers.isCommandDown() || alwaysFlipSelection)) {
        flipColSelection(colThatWasClickedOn);
    } else if (multipleSelection && modifiers.isShiftDown() && lastColSelected >= 0) {
        selectRangeOfCols(lastColSelected, colThatWasClickedOn);
    } else if ((!modifiers.isPopupMenu()) || !isColSelected(colThatWasClickedOn)) {
        selectColInternal(colThatWasClickedOn, false, !(multipleSelection && (!isMouseUpEvent) &&
                                                        isColSelected(colThatWasClickedOn)), true);
    }
}

int HorizontalListBox::getNumSelectedCols() const {
    return selected.size();
}

int HorizontalListBox::getSelectedCol(const int index) const {
    return (isPositiveAndBelow(index, selected.size()))
           ? selected[index] : -1;
}

bool HorizontalListBox::isColSelected(const int col) const {
    return selected.contains(col);
}

int HorizontalListBox::getLastColumnSelected() const {
    return isColSelected(lastColSelected) ? lastColSelected : -1;
}

//==============================================================================
int HorizontalListBox::getColContainingPosition(const int x, const int y) const noexcept {
    if (isPositiveAndBelow(getHeight(), y)) {
        const int col = (viewport->getViewPositionX() + x - viewport->getX()) / colWidth;

        if (isPositiveAndBelow(col, totalItems))
            return col;
    }

    return -1;
}

int HorizontalListBox::getInsertionIndexForPosition(const int x, const int y) const noexcept {
    if (isPositiveAndBelow(x, getWidth()))
        return jlimit(0, totalItems, (viewport->getViewPositionY() + y + colWidth / 2 - viewport->getY()) / colWidth);

    return -1;
}

Component *HorizontalListBox::getComponentForColNumber(const int col) const noexcept {
    if (auto *listColComp = viewport->getComponentForColIfOnscreen(col))
        return listColComp->customComponent.get();

    return nullptr;
}

int HorizontalListBox::getColNumberOfComponent(Component *const colComponent) const noexcept {
    return viewport->getColNumberOfComponent(colComponent);
}

Rectangle<int> HorizontalListBox::getColPosition(int colNumber, bool relativeToComponentTopLeft) const noexcept {
    auto x = viewport->getX() + colWidth * colNumber;

    if (relativeToComponentTopLeft)
        x -= viewport->getViewPositionX();

    return {x, viewport->getY(),
            colWidth, viewport->getViewedComponent()->getHeight()};
}

void HorizontalListBox::setHorizontalPosition(const double proportion) {
    auto offscreen = viewport->getViewedComponent()->getWidth() - viewport->getWidth();

    viewport->setViewPosition(jmax(0, roundToInt(proportion * offscreen)), viewport->getViewPositionY());
}

double HorizontalListBox::getHorizontalPosition() const {
    auto offscreen = viewport->getViewedComponent()->getWidth() - viewport->getWidth();

    return offscreen > 0 ? viewport->getViewPositionX() / (double) offscreen
                         : 0;
}

int HorizontalListBox::getVisibleColHeight() const noexcept {
    return viewport->getViewHeight();
}

void HorizontalListBox::scrollToEnsureColIsOnscreen(const int col) {
    viewport->scrollToEnsureColIsOnscreen(col, getColWidth());
}

//==============================================================================
bool HorizontalListBox::keyPressed(const KeyPress &key) {
    const int numVisibleColumns = viewport->getWidth() / getColWidth();

    const bool multiple = multipleSelection
                          && lastColSelected >= 0
                          && key.getModifiers().isShiftDown();

    if (key.isKeyCode(KeyPress::upKey)) {
        if (multiple)
            selectRangeOfCols(lastColSelected, lastColSelected - 1);
        else
            selectColumn(jmax(0, lastColSelected - 1));
    } else if (key.isKeyCode(KeyPress::downKey)) {
        if (multiple)
            selectRangeOfCols(lastColSelected, lastColSelected + 1);
        else
            selectColumn(jmin(totalItems - 1, jmax(0, lastColSelected) + 1));
    } else if (key.isKeyCode(KeyPress::pageUpKey)) {
        if (multiple)
            selectRangeOfCols(lastColSelected, lastColSelected - numVisibleColumns);
        else
            selectColumn(jmax(0, jmax(0, lastColSelected) - numVisibleColumns));
    } else if (key.isKeyCode(KeyPress::pageDownKey)) {
        if (multiple)
            selectRangeOfCols(lastColSelected, lastColSelected + numVisibleColumns);
        else
            selectColumn(jmin(totalItems - 1, jmax(0, lastColSelected) + numVisibleColumns));
    } else if (key.isKeyCode(KeyPress::homeKey)) {
        if (multiple)
            selectRangeOfCols(lastColSelected, 0);
        else
            selectColumn(0);
    } else if (key.isKeyCode(KeyPress::endKey)) {
        if (multiple)
            selectRangeOfCols(lastColSelected, totalItems - 1);
        else
            selectColumn(totalItems - 1);
    } else if (key.isKeyCode(KeyPress::returnKey) && isColSelected(lastColSelected)) {
        if (model != nullptr)
            model->returnKeyPressed(lastColSelected);
    } else if ((key.isKeyCode(KeyPress::deleteKey) || key.isKeyCode(KeyPress::backspaceKey))
               && isColSelected(lastColSelected)) {
        if (model != nullptr)
            model->deleteKeyPressed(lastColSelected);
    } else if (multipleSelection && key == KeyPress('a', ModifierKeys::commandModifier, 0)) {
        selectRangeOfCols(0, std::numeric_limits<int>::max());
    } else {
        return false;
    }

    return true;
}

bool HorizontalListBox::keyStateChanged(const bool isKeyDown) {
    return isKeyDown
           && (KeyPress::isKeyCurrentlyDown(KeyPress::upKey)
               || KeyPress::isKeyCurrentlyDown(KeyPress::pageUpKey)
               || KeyPress::isKeyCurrentlyDown(KeyPress::downKey)
               || KeyPress::isKeyCurrentlyDown(KeyPress::pageDownKey)
               || KeyPress::isKeyCurrentlyDown(KeyPress::homeKey)
               || KeyPress::isKeyCurrentlyDown(KeyPress::endKey)
               || KeyPress::isKeyCurrentlyDown(KeyPress::returnKey));
}

void HorizontalListBox::mouseWheelMove(const MouseEvent &e, const MouseWheelDetails &wheel) {
    bool eventWasUsed = false;

    if (wheel.deltaX != 0.0f && getHorizontalScrollBar().isVisible()) {
        eventWasUsed = true;
        getHorizontalScrollBar().mouseWheelMove(e, wheel);
    }

    if (wheel.deltaY != 0.0f && getVerticalScrollBar().isVisible()) {
        eventWasUsed = true;
        getVerticalScrollBar().mouseWheelMove(e, wheel);
    }

    if (!eventWasUsed)
        Component::mouseWheelMove(e, wheel);
}

void HorizontalListBox::mouseUp(const MouseEvent &e) {
    if (e.mouseWasClicked() && model != nullptr)
        model->backgroundClicked(e);
}

//==============================================================================
void HorizontalListBox::setColWidth(const int newWidth) {
    colWidth = jmax(1, newWidth);
    viewport->setSingleStepSizes(colWidth, 20);
    updateContent();
}

int HorizontalListBox::getNumColsOnScreen() const noexcept {
    return viewport->getMaximumVisibleWidth() / colWidth;
}

void HorizontalListBox::setMinimumContentWidth(const int newMinimumWidth) {
    minimumColWidth = newMinimumWidth;
    updateContent();
}

int HorizontalListBox::getVisibleContentWidth() const noexcept { return viewport->getMaximumVisibleWidth(); }

ScrollBar &HorizontalListBox::getVerticalScrollBar() const noexcept { return viewport->getVerticalScrollBar(); }

ScrollBar &HorizontalListBox::getHorizontalScrollBar() const noexcept { return viewport->getHorizontalScrollBar(); }

void HorizontalListBox::colourChanged() {
    setOpaque(findColour(backgroundColourId).isOpaque());
    viewport->setOpaque(isOpaque());
    repaint();
}

void HorizontalListBox::parentHierarchyChanged() {
    colourChanged();
}

void HorizontalListBox::setOutlineThickness(int newThickness) {
    outlineThickness = newThickness;
    resized();
}

void HorizontalListBox::setHeaderComponent(Component *newHeaderComponent) {
    if (headerComponent.get() != newHeaderComponent) {
        headerComponent.reset(newHeaderComponent);
        addAndMakeVisible(newHeaderComponent);
        HorizontalListBox::resized();
    }
}

void HorizontalListBox::repaintCol(const int colNumber) noexcept {
    repaint(getColPosition(colNumber, true));
}

Image HorizontalListBox::createSnapshotOfCols(const SparseSet<int> &cols, int &imageX, int &imageY) {
    Rectangle<int> imageArea;
    auto firstCol = getColContainingPosition(viewport->getX(), 0);

    for (int i = getNumColsOnScreen() + 2; --i >= 0;) {
        if (cols.contains(firstCol + i)) {
            if (auto *colComp = viewport->getComponentForColIfOnscreen(firstCol + i)) {
                auto pos = getLocalPoint(colComp, Point < int > ());

                imageArea = imageArea.getUnion({pos.x, pos.y, colComp->getWidth(), colComp->getHeight()});
            }
        }
    }

    imageArea = imageArea.getIntersection(getLocalBounds());
    imageX = imageArea.getX();
    imageY = imageArea.getY();

    Image snapshot(Image::ARGB, imageArea.getWidth(), imageArea.getHeight(), true);

    for (int i = getNumColsOnScreen() + 2; --i >= 0;) {
        if (cols.contains(firstCol + i)) {
            if (auto *colComp = viewport->getComponentForColIfOnscreen(firstCol + i)) {
                Graphics g(snapshot);
                g.setOrigin(getLocalPoint(colComp, Point < int > ()) - imageArea.getPosition());

                if (g.reduceClipRegion(colComp->getLocalBounds())) {
                    g.beginTransparencyLayer(0.6f);
                    colComp->paintEntireComponent(g, false);
                    g.endTransparencyLayer();
                }
            }
        }
    }

    return snapshot;
}

void
HorizontalListBox::startDragAndDrop(const MouseEvent &e, const SparseSet<int> &columnsToDrag,
                                    const var &dragDescription,
                                    bool allowDraggingToOtherWindows) {
    if (auto *dragContainer = DragAndDropContainer::findParentDragContainerFor(this)) {
        int x, y;
        auto dragImage = createSnapshotOfCols(columnsToDrag, x, y);

        auto p = Point < int > (x, y) - e.getEventRelativeTo(this).position.toInt();
        dragContainer->startDragging(dragDescription, this, dragImage, allowDraggingToOtherWindows, &p, &e.source);
    } else {
        // to be able to do a drag-and-drop operation, the listbox needs to
        // be inside a component which is also a DragAndDropContainer.
        jassertfalse;
    }
}