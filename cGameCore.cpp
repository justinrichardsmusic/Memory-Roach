#include "cGameCore.h"

void cGameCore::Init(olc::Decal* title, olc::Decal* info)
{
    srand((unsigned)time(NULL));

    decTitle = title;
    decInfo = info;
}

bool cGameCore::SetDebugMode(const bool debugOn)
{
    bDebugOn = debugOn;
    return bDebugOn;
}

olc::vf2d cGameCore::PointOfFocus(const olc::vf2d newPointOfFocus)
{
    if (newPointOfFocus != vfOutOfBounds)
        vfPointOfFocus = newPointOfFocus;

    return vfPointOfFocus;
}

float cGameCore::SecondsUntilNextMemBlock(const float secondsToSet)
{
    if (secondsToSet > -1.0f)
        fSecondsUntilNextMemBlock = secondsToSet;

    return fSecondsUntilNextMemBlock;
}

olc::vf2d cGameCore::LocationOfLastMemBlock(const olc::vf2d newLocationOfMemBlock)
{
    if (newLocationOfMemBlock != vfOutOfBounds)
        vfLocationOfLastMemBlock = newLocationOfMemBlock;

    return vfLocationOfLastMemBlock;
}

float cGameCore::MouseButtonHeldFor(const int button, const float modifyHeldForBy)
{
    if (button == 0)
    {
        if (modifyHeldForBy != 0.0f)
            fMouseZeroHeldFor += modifyHeldForBy;

        return fMouseZeroHeldFor;
    }
    else if (button == 1)
    {
        if (modifyHeldForBy != 0.0f)
            fMouseOneHeldFor += modifyHeldForBy;
        
        return fMouseOneHeldFor;
    }

    // Failsafe - should never reach here
    return 0.0f;
}

void cGameCore::ResetMouseButtonHeldState(const int button)
{
    if (button == 0)
        fMouseZeroHeldFor = 0.0f;
    else if (button == 1)
        fMouseOneHeldFor = 0.0f;

}

olc::vf2d cGameCore::PointerPos(const std::string pointerAorB, const olc::vf2d newPosToSet)
{
    if (pointerAorB ==      "A")
    {
        if (newPosToSet !=  vfOutOfBounds)
            vfPointerAPos = newPosToSet;

        return vfPointerAPos;
    }
    else if (pointerAorB == "B")
    {
        if (newPosToSet !=  vfOutOfBounds)
            vfPointerBPos = newPosToSet;

        return vfPointerBPos;
    }

    // Failsafe - should never reach here
    return olc::vf2d{};
}

void cGameCore::ResetPointerPos(const std::string pointerAorB)
{
    if (pointerAorB ==      "A")
        vfPointerAPos = vfOutOfBounds;
    else if (pointerAorB == "B")
        vfPointerBPos = vfOutOfBounds;

}

void cGameCore::AddSocket(const olc::vf2d tilePos, const int socketType, olc::Decal* decal)
{
    const olc::vf2d vfDefaultDirection =    { 1.0f,1.0f };
    const float fSpawnDelay =               0.0f;

    units.emplace_back                      (cUnit(tilePos, vfDefaultDirection, socketType, fSpawnDelay, decal));
    nCurrentAmountOfSockets++;
}

void cGameCore::PlacePointer(const olc::vf2d tilePos)
{
    if (bPlacePointerANext)
        PointerPos("A", tilePos);
    else
        PointerPos("B", tilePos);

    bPlacePointerANext = !bPlacePointerANext;
}

void cGameCore::Reset(olc::Decal* decTwoK)
{
    PointOfFocus                (vfCenterOfMap);
    fUITick =                   0.0f;
    nCurrentAmountOfSockets =   0;

    memBlocks.clear             ();
    units.clear                 ();

    ResetPointerPos             ("A");
    ResetPointerPos             ("B");
    bPlacePointerANext =        false;

    fSecondsUntilNextMemBlock = (float)(rand() % 4) + 0.25f;
    if (nSystemMem == 8)
        fSecondsUntilNextMemBlock = 6.0f;

    // Place dummy memory block to return when no memory exists
    memBlocks.emplace_back(cMemBlock(vfOutOfBounds, decTwoK));
    LocationOfLastMemBlock(GenerateInitialMemBlock(decTwoK));
}

void cGameCore::DrawGradient(olc::PixelGameEngine* pge)
{
    const olc::vf2d vfTL =                  { 0.0f, 0.0f };
    const olc::vf2d vfBR =                  { 720.0f, 360.0f };
    const olc::Pixel pTransparentBlack =    { 0, 0, 0, 0 };

    pge->GradientFillRectDecal              (vfTL, vfBR, olc::BLACK, pTransparentBlack, pTransparentBlack, olc::BLACK);
}

void cGameCore::DrawTitle(olc::PixelGameEngine* pge)
{
    const olc::vf2d	vfTitlePosition =       { 0.0f, 32.0f };
    pge->DrawDecal                          (vfTitlePosition, decTitle);
}

void cGameCore::DrawInstructions(olc::PixelGameEngine* pge)
{
    const olc::Pixel pTop =                 { 0, 0, 0, 0 };
    const olc::Pixel pBottom =              { 0, 0, 0, 255 };

    pge->GradientFillRectDecal              ({ 0.0f, 600.0f }, { 720.0f, 120.0f }, pTop, pBottom, pBottom, pTop);

    const olc::vf2d vfTextPos =             { 32.0f, 680.0f };
    const olc::vf2d vfScale =               { 2.0f, 2.0f };

    if (fUITick < 4.0f)
    {
        pge->DrawStringPropDecal(vfTextPos, "   Hold the LEFT-MOUSE button to construct sockets", olc::WHITE, vfScale);
        pge->DrawPartialDecal({ 232.0f, 512.0f }, { 256.0f, 128.0f }, decInfo, { 0.0f, 0.0f }, { 256.0f, 128.0f });
    }
    else if (fUITick < 8.0f)
    {
        pge->DrawStringPropDecal(vfTextPos, "         Hold the RIGHT-MOUSE button to set pointers", olc::WHITE, vfScale);
        pge->DrawPartialDecal({ 296.f, 512.0f }, { 128.0f, 128.0f }, decInfo, { 256.0f, 0.0f }, { 128.0f, 128.0f });
    }
    else if (fUITick < 12.0f)
    {
        pge->DrawStringPropDecal(vfTextPos, "LEFT-CLICK sockets to manufacture memory-roaches", olc::WHITE, vfScale);
        pge->DrawPartialDecal({ 232.0f, 512.0f }, { 256.0f, 128.0f }, decInfo, { 384.0f, 0.0f }, { 256.0f, 128.0f });
    }
    else if (fUITick < 16.0f)
    {
        pge->DrawStringPropDecal(vfTextPos, "         LEFT-CLICK used memory blocks to free them", olc::WHITE, vfScale);
        pge->DrawPartialDecal({ 296.0f, 512.0f }, { 128.0f, 128.0f }, decInfo, { 640.0f, 0.0f }, { 128.0f, 128.0f });
    }
    else
    {
        pge->DrawStringPropDecal(vfTextPos, "                              AVOID A SYSTEM CRASH!", olc::RED, vfScale);
    }
}

void cGameCore::DrawBoundingBox(olc::PixelGameEngine* pge, olc::vf2d camPos)
{
    const olc::vf2d vfTL =      { -616.0f, -616.0f };
    const olc::vf2d vfSize =    { 256.0f, 256.0f };
    const float fMapSize =      1664.0f;
    const float fThickness =    256.0f;
    const float fFactor =       12.0f;
    olc::Pixel pBorderColour =  { 0, 0, 0, 127 };

    pge->GradientFillRectDecal({ vfTL -     camPos                                               }, { vfSize.x * fFactor,   vfSize.y            }, olc::BLACK, pBorderColour, pBorderColour, olc::BLACK);
    pge->GradientFillRectDecal({ vfTL.x -   camPos.x,               vfTL.y - camPos.y + fMapSize }, { vfSize.x * fFactor,   vfSize.y            }, pBorderColour, olc::BLACK, olc::BLACK, pBorderColour);
    pge->GradientFillRectDecal({ vfTL -     camPos                                               }, { vfSize.x ,            vfSize.y * fFactor  }, olc::BLACK, olc::BLACK, pBorderColour, pBorderColour);
    pge->GradientFillRectDecal({ vfTL.x -   camPos.x + fMapSize,    vfTL.y - camPos.y            }, { vfSize.x ,            vfSize.y * fFactor  }, pBorderColour, pBorderColour, olc::BLACK, olc::BLACK);
}

void cGameCore::ProcessScreenScroll(const olc::vi2d mousePos, const float elapsedTime)
{
    const int nTL =         64;
    const int nBR =         656;

    const float fMapTL =    -232.0f;
    const float fMapBR =    928.0f;

    if (mousePos.y < nTL && PointOfFocus().y > fMapTL)
        vfPointOfFocus.y -= elapsedTime * fCursorSpeed;

    if (mousePos.y > nBR && vfPointOfFocus.y < fMapBR)
        vfPointOfFocus.y += elapsedTime * fCursorSpeed;

    if (mousePos.x < nTL && vfPointOfFocus.x > fMapTL)
        vfPointOfFocus.x -= elapsedTime * fCursorSpeed;

    if (mousePos.x > nBR && vfPointOfFocus.x < fMapBR)
        vfPointOfFocus.x += elapsedTime * fCursorSpeed;

}

void cGameCore::ClampTilePos(olc::vf2d& tilePos)
{
    const float fTL = -360.0f;
    const float fBR = 920.0f;

    if (tilePos.x < fTL)
        tilePos.x = fTL;

    if (tilePos.y < fTL)
        tilePos.y = fTL;

    if (tilePos.x > fBR)
        tilePos.x = fBR;

    if (tilePos.y > fBR)
        tilePos.y = fBR;

}

void cGameCore::UpdateAndClampUITick(const float elapsedTime, const float limit)
{
    fUITick += elapsedTime;

    if (fUITick > limit)
        fUITick -= limit;

}

bool cGameCore::TileOccupied(olc::vf2d worldPos, int& type)
{
    bool bOccupied = false;

    for (auto& u : units)
        if (u.vfPos == worldPos)
        {
            bOccupied = true;
            type = u.nType;
        }

    for (auto& m : memBlocks)
        if (m.vfPos == worldPos)
        {
            bOccupied = true;
            type = -1;
        }

    return bOccupied;
}

olc::vf2d cGameCore::GetMapPosFromMousePos(olc::vf2d pos)
{
    const float fSize =     128.0f;

    pos.x +=                360.0f;
    pos.y +=                360.0f;

    pos.x /=                fSize;
    pos.y /=                fSize;
    pos.x =                 (int)pos.x;
    pos.y =                 (int)pos.y;
    pos.x *=                fSize;
    pos.y *=                fSize;

    pos.x -=                360.0f;
    pos.y -=                360.0f;

    return pos;
}

cUnit* cGameCore::GetUnit(const olc::vf2d worldPos)
{
    for (auto& u : units)
        if (u.vfPos == worldPos)
            return &u;

    // Failed to get unit
    return &units[0];
}

cMemBlock* cGameCore::GetMemBlock(olc::vf2d worldPos)
{
    for (auto& m : memBlocks)
        if (m.vfPos == worldPos)
            return &m;

    // Failed to get memBlock
    return &memBlocks[0];
}

int cGameCore::GetFreeMemory()
{
    int nFreeMem = 0;
    for (auto& m : memBlocks)
        if (m.bUsed && !m.bFreed)
            nFreeMem += 2;

    return nFreeMem;
}

int cGameCore::GetUsedMemory()
{
    int count = 0;
    int mem = 0;

    for (auto& m : memBlocks)
    {
        if (!count == 0)
            mem += m.nMem;

        count++;
    }

    return mem;
}

bool cGameCore::GenerateMemBlock(const olc::vf2d worldPos, olc::Decal* twoK)
{
    int nDiscardType =              -1;
    fSecondsUntilNextMemBlock =     0.0f;

    if (!TileOccupied(worldPos, nDiscardType))
    {
        memBlocks.emplace_back(cMemBlock(worldPos, twoK));
        fSecondsUntilNextMemBlock = (float)(rand() % (128 / nSystemMem) + 2);

        return true;
    }

    return false;
}

olc::vf2d cGameCore::GenerateInitialMemBlock(olc::Decal* twoK)
{
    const int nRange =          5;
    const int nRangeOffset =    3;
    const int nFactor =         128;
    const int nMapOffset =      -360;

    int x = rand() % nRange + nRangeOffset + 1;
    int y = rand() % nRange + nRangeOffset;

    x *= nFactor;
    y *= nFactor;

    x += nMapOffset;
    y += nMapOffset;

    olc::vf2d newBlockPos = { float(x), float(y) };
    GenerateMemBlock(newBlockPos, twoK);

    return newBlockPos;
}

olc::vf2d cGameCore::ChooseNextMemBlockPos(const olc::vf2d lastBlockPos)
{
    // Generate a position in a 3x3 square around the lastBlockPos
    int x = rand() % 3 - 1;
    int y = 0;

    if (x == 0)
    {
        y = rand() % 2;
        if (y == 0)
            y = -1;

    }
    else
    {
        y = rand() % 3 - 1;
    }

    x *= 128;
    y *= 128;

    const olc::vf2d nextBlockPos{ lastBlockPos.x + float(x), lastBlockPos.y + float(y) };

    return nextBlockPos;
}

int cGameCore::CalculateAvailableMemory(const bool crashOccured)
{
    if (crashOccured)
        return -1;

    return nSystemMem - GetUsedMemory() - GetFreeMemory();
}

olc::vf2d cGameCore::GenerateVFAxis()
{
    int x = rand() % 2;
    int y = rand() % 2;

    if (x == 0)
        x = -1;

    if (y == 0)
        y = -1;

    return olc::vf2d((float)x, (float)y);
}

bool cGameCore::TilePosWithinMapBounds(olc::vf2d tilePos)
{
    const float fMapTLBounds = -360.0f;
    const float fMapBRBounds = 920.0f;

    return (tilePos.x >= fMapTLBounds || tilePos.x < fMapBRBounds || tilePos.y >= fMapTLBounds || tilePos.y < fMapBRBounds);
}
