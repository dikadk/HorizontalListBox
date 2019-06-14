/*
  ==============================================================================

    HorizontalHorizontalListBox.h
    Created: 13 Jun 2019 11:46:38am
    Author:  Dmytro Kiro

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"


class HorizontalListBox : public Component,
                                    public SettableTooltipClient {
public:

    HorizontalListBox(const String &componentName = String(),
                      ListBoxModel *model = nullptr);

    ~HorizontalListBox() override;


//==============================================================================
/** Changes the current data model to display. */
    void setModel(ListBoxModel *newModel);

/** Returns the current list model. */
    ListBoxModel *getModel() const noexcept { return model; }


//==============================================================================
/** Causes the list to refresh its content.

    Call this when the number of columns in the list changes, or if you want it
    to call refreshComponentForColumn() on all the column components.

    This must only be called from the main message thread.
*/
    void updateContent();

//==============================================================================
/** Turns on multiple-selection of columns.

    By default this is disabled.

    When your column component gets clicked you'll need to call the
    selectColumnsBasedOnModifierKeys() method to tell the list that it's been
    clicked and to get it to do the appropriate selection based on whether
    the ctrl/shift keys are held down.
*/
    void setMultipleSelectionEnabled(bool shouldBeEnabled) noexcept;

/** If enabled, this makes the HorizontalListBox flip the selection status of
    each column that the user clicks, without affecting other selected columns.

    (This only has an effect if multiple selection is also enabled).
    If not enabled, you can still get the same column-flipping behaviour by holding
    down CMD or CTRL when clicking.
*/
    void setClickingTogglesColumnSelection(bool flipColumnSelection) noexcept;

/** Sets whether a column should be selected when the mouse is pressed or released.
    By default this is true, but you may want to turn it off.
*/
    void setColumnSelectedOnMouseDown(bool isSelectedOnMouseDown) noexcept;

/** Makes the list react to mouse moves by selecting the column that the mouse if over.

    This function is here primarily for the ComboBox class to use, but might be
    useful for some other purpose too.
*/
    void setMouseMoveSelectsColumns(bool shouldSelect);

//==============================================================================
/** Selects a column.

    If the column is already selected, this won't do anything.

    @param colNumber                the column to select
    @param dontScrollToShowThisCol  if true, the list's position won't change; if false and
                                    the selected column is off-screen, it'll scroll to make
                                    sure that column is on-screen
    @param deselectOthersFirst      if true and there are multiple selections, these will
                                    first be deselected before this item is selected
    @see isColumnSelected, selectColumnsBasedOnModifierKeys, flipColumnSelection, deselectColumn,
         deselectAllColumns, selectRangeOfColumns
*/
    void selectColumn(int colNumber,
                      bool dontScrollToShowThisCol = false,
                      bool deselectOthersFirst = true);

/** Selects a set of columns.

    This will add these columns to the current selection, so you might need to
    clear the current selection first with deselectAllColumns()

    @param firstCol                       the first column to select (inclusive)
    @param lastCol                        the last column to select (inclusive)
    @param dontScrollToShowThisRange      if true, the list's position won't change; if false and
                                          the selected range is off-screen, it'll scroll to make
                                          sure that the range of columns is on-screen
*/
    void selectRangeOfCols(int firstCol,
                           int lastCol,
                           bool dontScrollToShowThisRange = false);

/** Deselects a column.
    If it's not currently selected, this will do nothing.
    @see selectColumn, deselectAllColumns
*/
    void deselectCol(const int col);

/** Deselects any currently selected columns.
    @see deselectColumn
*/
    void deselectAllCols();

/** Selects or deselects a column.
    If the column's currently selected, this deselects it, and vice-versa.
*/
    void flipColSelection(int columnNumber);

/** Returns a sparse set indicating the columns that are currently selected.
    @see setSelectedColumns
*/
    SparseSet<int> getSelectedCols() const;

/** Sets the columns that should be selected, based on an explicit set of ranges.

    If sendNotificationEventToModel is true, the HorizontalListBoxModel::selectedColumnsChanged()
    method will be called. If it's false, no notification will be sent to the model.

    @see getSelectedColumns
*/
    void setSelectedCols(const SparseSet<int> &setOfColumnsToBeSelected,
                         const NotificationType sendNotificationEventToModel = sendNotification);

/** Checks whether a column is selected.
*/
    bool isColSelected(int columnNumber) const;

/** Returns the number of columns that are currently selected.
    @see getSelectedColumn, isColumnSelected, getLastColumnSelected
*/
    int getNumSelectedCols() const;

/** Returns the column number of a selected column.

    This will return the column number of the Nth selected column. The column numbers returned will
    be sorted in order from low to high.

    @param index    the index of the selected column to return, (from 0 to getNumSelectedColumns() - 1)
    @returns        the column number, or -1 if the index was out of range or if there aren't any columns
                    selected
    @see getNumSelectedColumns, isColumnSelected, getLastColumnSelected
*/
    int getSelectedCol(int index = 0) const;

/** Returns the last column that the user selected.

    This isn't the same as the highest column number that is currently selected - if the user
    had multiply-selected columns 10, 5 and then 6 in that order, this would return 6.

    If nothing is selected, it will return -1.
*/
    int getLastColumnSelected() const;

/** Multiply-selects columns based on the modifier keys.

    If no modifier keys are down, this will select the given column and
    deselect any others.

    If the ctrl (or command on the Mac) key is down, it'll flip the
    state of the selected column.

    If the shift key is down, it'll select up to the given column from the
    last column selected.

    @see selectColumn
*/
    void selectColsBasedOnModifierKeys(int colThatWasClickedOn,
                                       ModifierKeys modifiers,
                                       bool isMouseUpEvent);

//==============================================================================
/** Scrolls the list to a particular position.

    The proportion is between 0 and 1.0, so 0 scrolls to the top of the list,
    1.0 scrolls to the bottom.

    If the total number of columns all fit onto the screen at once, then this
    method won't do anything.

    @see getVerticalPosition
*/
    void setHorizontalPosition(double newProportion);

/** Returns the current vertical position as a proportion of the total.

    This can be used in conjunction with setVerticalPosition() to save and restore
    the list's position. It returns a value in the range 0 to 1.

    @see setVerticalPosition
*/
    double getHorizontalPosition() const;

/** Scrolls if necessary to make sure that a particular column is visible. */
    void scrollToEnsureColIsOnscreen(int col);

/** Returns a reference to the vertical scrollbar. */
    ScrollBar &getVerticalScrollBar() const noexcept;

/** Returns a reference to the horizontal scrollbar. */
    ScrollBar &getHorizontalScrollBar() const noexcept;

/** Finds the column index that contains a given x,y position.
    The position is relative to the HorizontalListBox's top-left.
    If no column exists at this position, the method will return -1.
    @see getComponentForColumnNumber
*/
    int getColContainingPosition(int x, int y) const noexcept;

/** Finds a column index that would be the most suitable place to insert a new
    item for a given position.

    This is useful when the user is e.g. dragging and dropping onto the HorizontalListBox,
    because it lets you easily choose the best position to insert the item that
    they drop, based on where they drop it.

    If the position is out of range, this will return -1. If the position is
    beyond the end of the list, it will return getNumCols() to indicate the end
    of the list.

    @see getComponentForColumnNumber
*/
    int getInsertionIndexForPosition(int x, int y) const noexcept;

/** Returns the position of one of the columns, relative to the top-left of
    the HorizontalListBox.

    This may be off-screen, and the range of the column number that is passed-in is
    not checked to see if it's a valid column.
*/
    Rectangle<int> getColPosition(int colNumber,
                                  bool relativeToComponentTopLeft) const noexcept;

/** Finds the column component for a given column in the list.

    The component returned will have been created using HorizontalListBoxModel::refreshComponentForColumn().

    If the component for this column is off-screen or if the column is out-of-range,
    this will return nullptr.

    @see getColumnContainingPosition
*/
    Component *getComponentForColNumber(int col) const noexcept;

/** Returns the column number that the given component represents.
    If the component isn't one of the list's columns, this will return -1.
*/
    int getColNumberOfComponent(Component *const colComponent) const noexcept;

/** Returns the width of a column (which may be less than the width of this component
    if there's a scrollbar).
*/
    int getVisibleColHeight() const noexcept;

//==============================================================================
/** Sets the width of each column in the list.
    The default width is 22 pixels.
    @see getColumnWidth
*/
    void setColWidth(int newWidth);

/** Returns the height of a column in the list.
    @see setColumnHeight
*/
    int getColWidth() const noexcept { return colWidth; }

/** Returns the number of columns actually visible.

    This is the number of whole columns which will fit on-screen, so the value might
    be more than the actual number of columns in the list.
*/
    int getNumColsOnScreen() const noexcept;

//==============================================================================
/** A set of colour IDs to use to change the colour of various aspects of the label.

    These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
    methods.

    @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
*/
    enum ColourIds {
        backgroundColourId = 0x1002800, /**< The background colour to fill the list with.
                                                  Make this transparent if you don't want the background to be filled. */
        outlineColourId = 0x1002810, /**< An optional colour to use to draw a border around the list.
                                                  Make this transparent to not have an outline. */
        textColourId = 0x1002820  /**< The preferred colour to use for drawing text in the HorizontalListBox. */
    };

/** Sets the thickness of a border that will be drawn around the box.

    To set the colour of the outline, use @code setColour (HorizontalListBox::outlineColourId, colourXYZ); @endcode
    @see outlineColourId
*/
    void setOutlineThickness(int outlineThickness);

/** Returns the thickness of outline that will be drawn around the HorizontalListBox.
    @see setOutlineColour
*/
    int getOutlineThickness() const noexcept { return outlineThickness; }

/** Sets a component that the list should use as a header.

    This will position the given component at the top of the list, maintaining the
    height of the component passed-in, but rescaling it horizontally to match the
    width of the items in the HorizontalListBox.

    The component will be deleted when setHeaderComponent() is called with a
    different component, or when the HorizontalListBox is deleted.
*/
    void setHeaderComponent(Component *newHeaderComponent);

/** Returns whatever header component was set with setHeaderComponent(). */
    Component *getHeaderComponent() const noexcept { return headerComponent.get(); }

/** Changes the width of the columns in the list.

    This can be used to make the list's column components wider than the list itself - the
    width of the columns will be either the width of the list or this value, whichever is
    greater, and if the columns become wider than the list, a horizontal scrollbar will
    appear.

    The default value for this is 0, which means that the columns will always
    be the same width as the list.
*/
    void setMinimumContentWidth(int newMinimumWidth);

/** Returns the space currently available for the column items, taking into account
    borders, scrollbars, etc.
*/
    int getVisibleContentWidth() const noexcept;

/** Repaints one of the columns.

    This does not invoke updateContent(), it just invokes a straightforward repaint
    for the area covered by this column.
*/
    void repaintCol(int colNumber) noexcept;

/** This fairly obscure method creates an image that shows the column components specified
    in columns (for example, these could be the currently selected column components).

    It's a handy method for doing drag-and-drop, as it can be passed to the
    DragAndDropContainer for use as the drag image.

    Note that it will make the column components temporarily invisible, so if you're
    using custom components this could affect them if they're sensitive to that
    sort of thing.

    @see Component::createComponentSnapshot
*/
    virtual Image createSnapshotOfCols(const SparseSet<int> &cols, int &imageX, int &imageY);

/** Returns the viewport that this HorizontalListBox uses.

    You may need to use this to change parameters such as whether scrollbars
    are shown, etc.
*/
    Viewport *getViewport() const noexcept;

//==============================================================================
/** @internal */
    bool keyPressed(const KeyPress &) override;

/** @internal */
    bool keyStateChanged(bool isKeyDown) override;

/** @internal */
    void paint(Graphics &) override;

/** @internal */
    void paintOverChildren(Graphics &) override;

/** @internal */
    void resized() override;

/** @internal */
    void visibilityChanged() override;

/** @internal */
    void mouseWheelMove(const MouseEvent &, const MouseWheelDetails &) override;

/** @internal */
    void mouseUp(const MouseEvent &) override;

/** @internal */
    void colourChanged() override;

/** @internal */
    void parentHierarchyChanged() override;

/** @internal */
    void startDragAndDrop(const MouseEvent &, const SparseSet<int> &columnsToDrag,
                          const var &dragDescription, bool allowDraggingToOtherWindows);

private:
//==============================================================================
    JUCE_PUBLIC_IN_DLL_BUILD (class ListViewport)

    JUCE_PUBLIC_IN_DLL_BUILD (class ColumnComponent)

    friend class ListViewport;

    friend class TableHorizontalListBox;

    ListBoxModel *model;
    std::unique_ptr<ListViewport> viewport;
    std::unique_ptr<Component> headerComponent;
    std::unique_ptr<MouseListener> mouseMoveSelector;
    SparseSet<int> selected;
    int totalItems = 0, colWidth = 22, minimumColWidth = 0;
    int outlineThickness = 0;
    int lastColSelected = -1;
    bool multipleSelection = false, alwaysFlipSelection = false, hasDoneInitialUpdate = false, selectOnMouseDown = true;

    void selectColInternal(int col, bool dontScrollToShowThisColumn,
                           bool deselectOthersFirst, bool isMouseClick);

#if JUCE_CATCH_DEPRECATED_CODE_MISUSE
// This method's bool parameter has changed: see the new method signature.
    JUCE_DEPRECATED (void setSelectedColumns(const SparseSet<int> &, bool));
    // This method has been replaced by the more flexible method createSnapshotOfCols.
    // Please call createSnapshotOfCols (getSelectedCols(), x, y) to get the same behaviour.
    JUCE_DEPRECATED (virtual void createSnapshotOfSelectedColumns(int &, int &)) {}

#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HorizontalListBox)
};