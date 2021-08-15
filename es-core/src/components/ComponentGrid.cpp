//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  ComponentGrid.cpp
//
//  Provides basic layout of components in an X*Y grid.
//

#include "components/ComponentGrid.h"

#include "Settings.h"

using namespace GridFlags;

ComponentGrid::ComponentGrid(Window* window, const Vector2i& gridDimensions)
    : GuiComponent(window)
    , mGridSize(gridDimensions)
    , mCursor(0, 0)
{
    assert(gridDimensions.x() > 0 && gridDimensions.y() > 0);

    mCells.reserve(gridDimensions.x() * gridDimensions.y());

    mColWidths = new float[gridDimensions.x()];
    mRowHeights = new float[gridDimensions.y()];

    for (int x = 0; x < gridDimensions.x(); x++)
        mColWidths[x] = 0;
    for (int y = 0; y < gridDimensions.y(); y++)
        mRowHeights[y] = 0;
}

ComponentGrid::~ComponentGrid()
{
    delete[] mRowHeights;
    delete[] mColWidths;
}

float ComponentGrid::getColWidth(int col)
{
    if (mColWidths[col] != 0)
        return mColWidths[col] * mSize.x();

    // Calculate automatic width.
    float freeWidthPerc = 1;
    int between = 0;
    for (int x = 0; x < mGridSize.x(); x++) {
        freeWidthPerc -= mColWidths[x]; // If it's 0 it won't do anything.
        if (mColWidths[x] == 0)
            between++;
    }

    return (freeWidthPerc * mSize.x()) / between;
}

float ComponentGrid::getRowHeight(int row)
{
    if (mRowHeights[row] != 0)
        return mRowHeights[row] * mSize.y();

    // Calculate automatic height.
    float freeHeightPerc = 1;
    int between = 0;
    for (int y = 0; y < mGridSize.y(); y++) {
        freeHeightPerc -= mRowHeights[y]; // If it's 0 it won't do anything.
        if (mRowHeights[y] == 0)
            between++;
    }

    return (freeHeightPerc * mSize.y()) / between;
}

void ComponentGrid::setColWidthPerc(int col, float width, bool update)
{
    assert(col >= 0 && col < mGridSize.x());
    mColWidths[col] = width;

    if (update)
        onSizeChanged();
}

void ComponentGrid::setRowHeightPerc(int row, float height, bool update)
{
    assert(height >= 0 && height <= 1);
    assert(row >= 0 && row < mGridSize.y());
    mRowHeights[row] = height;

    if (update)
        onSizeChanged();
}

void ComponentGrid::setEntry(const std::shared_ptr<GuiComponent>& comp,
                             const Vector2i& pos,
                             bool canFocus,
                             bool resize,
                             const Vector2i& size,
                             unsigned int border,
                             GridFlags::UpdateType updateType)
{
    assert(pos.x() >= 0 && pos.x() < mGridSize.x() && pos.y() >= 0 && pos.y() < mGridSize.y());
    assert(comp != nullptr);
    assert(comp->getParent() == nullptr);

    GridEntry entry(pos, size, comp, canFocus, resize, updateType, border);
    mCells.push_back(entry);

    addChild(comp.get());

    if (!cursorValid() && canFocus) {
        auto origCursor = mCursor;
        mCursor = pos;
        onCursorMoved(origCursor, mCursor);
    }

    updateCellComponent(mCells.back());
    updateSeparators();
}

bool ComponentGrid::removeEntry(const std::shared_ptr<GuiComponent>& comp)
{
    for (auto it = mCells.cbegin(); it != mCells.cend(); it++) {
        if (it->component == comp) {
            removeChild(comp.get());
            mCells.erase(it);
            return true;
        }
    }

    return false;
}

void ComponentGrid::updateCellComponent(const GridEntry& cell)
{
    // Size.
    Vector2f size(0, 0);
    for (int x = cell.pos.x(); x < cell.pos.x() + cell.dim.x(); x++)
        size[0] += getColWidth(x);
    for (int y = cell.pos.y(); y < cell.pos.y() + cell.dim.y(); y++)
        size[1] += getRowHeight(y);

    if (cell.resize)
        cell.component->setSize(size);

    // Find top left corner.
    glm::vec3 pos {};
    for (int x = 0; x < cell.pos.x(); x++)
        pos[0] += getColWidth(x);
    for (int y = 0; y < cell.pos.y(); y++)
        pos[1] += getRowHeight(y);

    // Center component.
    pos[0] = pos.x + (size.x() - cell.component->getSize().x()) / 2.0f;
    pos[1] = pos.y + (size.y() - cell.component->getSize().y()) / 2.0f;

    cell.component->setPosition(pos);
}

void ComponentGrid::updateSeparators()
{
    mSeparators.clear();

    bool drawAll = Settings::getInstance()->getBool("DebugGrid");

    Vector2f pos;
    Vector2f size;

    for (auto it = mCells.cbegin(); it != mCells.cend(); it++) {
        if (!it->border && !drawAll)
            continue;

        // Find component position + size.
        pos = Vector2f(0, 0);
        size = Vector2f(0, 0);
        for (int x = 0; x < it->pos.x(); x++)
            pos[0] += getColWidth(x);
        for (int y = 0; y < it->pos.y(); y++)
            pos[1] += getRowHeight(y);
        for (int x = it->pos.x(); x < it->pos.x() + it->dim.x(); x++)
            size[0] += getColWidth(x);
        for (int y = it->pos.y(); y < it->pos.y() + it->dim.y(); y++)
            size[1] += getRowHeight(y);

        if (size == 0)
            return;

        if (it->border & BORDER_TOP || drawAll) {
            std::vector<float> coordVector;
            coordVector.push_back(pos.x());
            coordVector.push_back(pos.y());
            coordVector.push_back(size.x());
            coordVector.push_back(1.0f * Renderer::getScreenHeightModifier());
            mSeparators.push_back(coordVector);
        }
        if (it->border & BORDER_BOTTOM || drawAll) {
            std::vector<float> coordVector;
            coordVector.push_back(pos.x());
            coordVector.push_back(pos.y() + size.y());
            coordVector.push_back(size.x());
            coordVector.push_back(1.0f * Renderer::getScreenHeightModifier());
            mSeparators.push_back(coordVector);
        }
        if (it->border & BORDER_LEFT || drawAll) {
            std::vector<float> coordVector;
            coordVector.push_back(pos.x());
            coordVector.push_back(pos.y());
            coordVector.push_back(1.0f * Renderer::getScreenWidthModifier());
            coordVector.push_back(size.y());
            mSeparators.push_back(coordVector);
        }
        if (it->border & BORDER_RIGHT || drawAll) {
            std::vector<float> coordVector;
            coordVector.push_back(pos.x() + size.x());
            coordVector.push_back(pos.y());
            coordVector.push_back(1.0f * Renderer::getScreenWidthModifier());
            coordVector.push_back(size.y());
            mSeparators.push_back(coordVector);
        }
    }
}

void ComponentGrid::onSizeChanged()
{
    for (auto it = mCells.cbegin(); it != mCells.cend(); it++)
        updateCellComponent(*it);

    updateSeparators();
}

const ComponentGrid::GridEntry* ComponentGrid::getCellAt(int x, int y) const
{
    assert(x >= 0 && x < mGridSize.x() && y >= 0 && y < mGridSize.y());

    for (auto it = mCells.cbegin(); it != mCells.cend(); it++) {
        int xmin = it->pos.x();
        int xmax = xmin + it->dim.x();
        int ymin = it->pos.y();
        int ymax = ymin + it->dim.y();

        if (x >= xmin && y >= ymin && x < xmax && y < ymax)
            return &(*it);
    }

    return nullptr;
}

bool ComponentGrid::input(InputConfig* config, Input input)
{
    const GridEntry* cursorEntry = getCellAt(mCursor);
    if (cursorEntry && cursorEntry->component->input(config, input))
        return true;

    if (!input.value)
        return false;

    if (config->isMappedLike("down", input))
        return moveCursor(Vector2i(0, 1));

    if (config->isMappedLike("up", input))
        return moveCursor(Vector2i(0, -1));

    if (config->isMappedLike("left", input))
        return moveCursor(Vector2i(-1, 0));

    if (config->isMappedLike("right", input))
        return moveCursor(Vector2i(1, 0));

    return false;
}

void ComponentGrid::resetCursor()
{
    if (!mCells.size())
        return;

    for (auto it = mCells.cbegin(); it != mCells.cend(); it++) {
        if (it->canFocus) {
            Vector2i origCursor = mCursor;
            mCursor = it->pos;
            onCursorMoved(origCursor, mCursor);
            break;
        }
    }
}

bool ComponentGrid::moveCursor(Vector2i dir)
{
    assert(dir.x() || dir.y());

    const Vector2i origCursor = mCursor;
    const GridEntry* currentCursorEntry = getCellAt(mCursor);
    Vector2i searchAxis(dir.x() == 0, dir.y() == 0);

    while (mCursor.x() >= 0 && mCursor.y() >= 0 && mCursor.x() < mGridSize.x() &&
           mCursor.y() < mGridSize.y()) {
        mCursor = mCursor + dir;
        Vector2i curDirPos = mCursor;
        const GridEntry* cursorEntry;

        // Spread out on search axis+
        while (mCursor.x() < mGridSize.x() && mCursor.y() < mGridSize.y() && mCursor.x() >= 0 &&
               mCursor.y() >= 0) {
            cursorEntry = getCellAt(mCursor);
            if (cursorEntry && cursorEntry->canFocus && cursorEntry != currentCursorEntry) {
                onCursorMoved(origCursor, mCursor);
                return true;
            }
            mCursor += searchAxis;
        }

        // Now again on search axis-
        mCursor = curDirPos;
        while (mCursor.x() >= 0 && mCursor.y() >= 0 && mCursor.x() < mGridSize.x() &&
               mCursor.y() < mGridSize.y()) {
            cursorEntry = getCellAt(mCursor);

            if (cursorEntry && cursorEntry->canFocus && cursorEntry != currentCursorEntry) {
                onCursorMoved(origCursor, mCursor);
                return true;
            }
            mCursor -= searchAxis;
        }
        mCursor = curDirPos;
    }

    // Failed to find another focusable element in this direction.
    mCursor = origCursor;
    return false;
}

void ComponentGrid::onFocusLost()
{
    const GridEntry* cursorEntry = getCellAt(mCursor);
    if (cursorEntry)
        cursorEntry->component->onFocusLost();
}

void ComponentGrid::onFocusGained()
{
    const GridEntry* cursorEntry = getCellAt(mCursor);
    if (cursorEntry)
        cursorEntry->component->onFocusGained();
}

bool ComponentGrid::cursorValid()
{
    const GridEntry* e = getCellAt(mCursor);
    return (e != nullptr && e->canFocus);
}

void ComponentGrid::update(int deltaTime)
{
    // Update everything.
    const GridEntry* cursorEntry = getCellAt(mCursor);
    for (auto it = mCells.cbegin(); it != mCells.cend(); it++) {
        if (it->updateType == UPDATE_ALWAYS ||
            (it->updateType == UPDATE_WHEN_SELECTED && cursorEntry == &(*it))) {
            it->component->update(deltaTime);
        }
    }
}

void ComponentGrid::render(const glm::mat4& parentTrans)
{
    glm::mat4 trans = parentTrans * getTransform();

    renderChildren(trans);

    // Draw cell separators.
    for (size_t i = 0; i < mSeparators.size(); i++) {
        Renderer::setMatrix(trans);
        Renderer::drawRect(mSeparators[i][0], mSeparators[i][1], mSeparators[i][2],
                           mSeparators[i][3], 0xC6C7C6FF, 0xC6C7C6FF);
    }
}

void ComponentGrid::textInput(const std::string& text)
{
    const GridEntry* selectedEntry = getCellAt(mCursor);
    if (selectedEntry != nullptr && selectedEntry->canFocus)
        selectedEntry->component->textInput(text);
}

void ComponentGrid::onCursorMoved(Vector2i from, Vector2i to)
{
    const GridEntry* cell = getCellAt(from);
    if (cell)
        cell->component->onFocusLost();

    cell = getCellAt(to);
    if (cell)
        cell->component->onFocusGained();

    updateHelpPrompts();
}

void ComponentGrid::setCursorTo(const std::shared_ptr<GuiComponent>& comp)
{
    for (auto it = mCells.cbegin(); it != mCells.cend(); it++) {
        if (it->component == comp) {
            Vector2i oldCursor = mCursor;
            mCursor = it->pos;
            onCursorMoved(oldCursor, mCursor);
            return;
        }
    }

    // Component not found!
    assert(false);
}

std::vector<HelpPrompt> ComponentGrid::getHelpPrompts()
{
    std::vector<HelpPrompt> prompts;
    const GridEntry* e = getCellAt(mCursor);
    if (e)
        prompts = e->component->getHelpPrompts();

    bool canScrollVert = mGridSize.y() > 1;
    bool canScrollHoriz = mGridSize.x() > 1;
    for (auto it = prompts.cbegin(); it != prompts.cend(); it++) {
        if (it->first == "up/down/left/right") {
            canScrollHoriz = false;
            canScrollVert = false;
            break;
        }
        else if (it->first == "up/down") {
            canScrollVert = false;
        }
        else if (it->first == "left/right") {
            canScrollHoriz = false;
        }
    }

    if (canScrollHoriz && canScrollVert)
        prompts.push_back(HelpPrompt("up/down/left/right", "choose"));
    else if (canScrollHoriz)
        prompts.push_back(HelpPrompt("left/right", "choose"));
    else if (canScrollVert)
        prompts.push_back(HelpPrompt("up/down", "choose"));

    return prompts;
}
