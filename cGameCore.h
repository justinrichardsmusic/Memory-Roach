#pragma once
#include "olcPixelGameEngine.h"
#include "cUnit.h"
#include "cMemBlock.h"

class cGameCore
{
public:
	cGameCore() {}

private:
	// Variables
	olc::vf2d				vfPointOfFocus					{};
	
	float					fSecondsUntilNextMemBlock =		2.0f;
	olc::vf2d				vfLocationOfLastMemBlock		{};

	float					fMouseZeroHeldFor =				0.0f;
	float					fMouseOneHeldFor =				0.0f;
	
	const olc::vf2d			vfOutOfBounds =					{ -999.0f, -999.0f };
	olc::vf2d				vfPointerAPos =					vfOutOfBounds;
	olc::vf2d				vfPointerBPos =					vfOutOfBounds;

public:
	// Variables
	bool					bDebugOn =						false;
	const olc::vf2d			vfCenterOfMap =					{ 360.0f, 360.0f };
	const float				fCursorSpeed =					512.0f;
	float					fUITick =						0.0f;
	int						nSystemMem =					8;
	int						nCurrentAmountOfSockets =		0;
	bool					bPlacePointerANext =			false;

	std::vector<cUnit>		units							{};
	std::vector<cMemBlock>	memBlocks						{};

	olc::Decal*				decTitle =						nullptr;
	olc::Decal*				decInfo =						nullptr;

	// Functions
	void					Init							(olc::Decal* title, olc::Decal* info);
	bool					SetDebugMode					(const bool debugOn);
	olc::vf2d				PointOfFocus					(const olc::vf2d newPointOfFocus = { -999.0f, -999.0f });
	float					SecondsUntilNextMemBlock		(const float secondsToSet = -1.0f);
	olc::vf2d				LocationOfLastMemBlock			(const olc::vf2d newLocationOfMemBlock = { -999.0f, -999.0f });
	float					MouseButtonHeldFor				(const int button, const float modifyHeldForBy = 0.0f);
	void					ResetMouseButtonHeldState		(const int button);
	olc::vf2d				PointerPos						(const std::string pointerAorB, const olc::vf2d newPosToSet = { -999.0f, -999.0f });
	void					ResetPointerPos					(const std::string pointerAorB);
	void					AddSocket						(const olc::vf2d tilePos, const int socketType, olc::Decal* decal);
	void					PlacePointer					(const olc::vf2d tilePos);

	void					Reset							(olc::Decal* decTwoK);

	void					DrawGradient					(olc::PixelGameEngine* pge);
	void					DrawTitle						(olc::PixelGameEngine* pge);
	void					DrawInstructions				(olc::PixelGameEngine* pge);
	void					DrawBoundingBox					(olc::PixelGameEngine* pge, olc::vf2d camPos);

	void					ProcessScreenScroll				(const olc::vi2d mousePos, const float elapsedTime);
	void					ClampTilePos					(olc::vf2d& tilePos);
	void					UpdateAndClampUITick			(const float elapsedTime, const float limit);

	bool					TileOccupied					(olc::vf2d worldPos, int& type);
	olc::vf2d				GetMapPosFromMousePos			(olc::vf2d pos);
	cUnit*					GetUnit							(const olc::vf2d worldPos);
	cMemBlock*				GetMemBlock						(olc::vf2d worldPos);
	int						GetFreeMemory					();
	int						GetUsedMemory					();
	bool					GenerateMemBlock				(const olc::vf2d worldPos, olc::Decal* twoK);
	olc::vf2d				GenerateInitialMemBlock			(olc::Decal* twoK);
	olc::vf2d				ChooseNextMemBlockPos			(const olc::vf2d lastBlockPos);
	int						CalculateAvailableMemory		(const bool crashOccured = false);
	olc::vf2d				GenerateVFAxis					();
	bool					TilePosWithinMapBounds			(olc::vf2d tilePos);
};
