#include "Source.h"

using namespace GAME2D;
class CodeJam2023 : public olc::PixelGameEngine, public GAME2D::PGE_GAME_2D_BACKEND
{
public:
	CodeJam2023() { sAppName = "Memory-Roach"; }

private:
	cGameCore	game		{};
	cMenu		menu		{};
	cHud		hud			{};
	cUI			ui			{};

	void AddAudioSources()
	{
		AddAudioSource(MEM_LOCK,		"./assets/snd/mem_lock.wav");
		AddAudioSource(FREE_MEM_AVAIL,	"./assets/snd/free_mem_avail.wav");
		AddAudioSource(MEM_FREED,		"./assets/snd/mem_freed.wav");
		AddAudioSource(NEW_UNIT,		"./assets/snd/new_unit.wav");
		AddAudioSource(PLACE_SOCKET,	"./assets/snd/place_socket.wav");
		AddAudioSource(CRASH,			"./assets/snd/crash.wav");
		AddAudioSource(WIN,				"./assets/snd/win.wav");
	
		AddAudioSource(BG_MUS,			"./assets/snd/bg_mus.mp3");
	}

public:
	bool OnUserCreate() override
	{
		// Start GAME2D backend and set options
		PGE_GAME_2D_BACKEND::StartGAME2D		(ScreenWidth(), ScreenHeight(), SPLASH_SCREEN_ON, "./assets/snd/default.wav");
		splashScreen.SetOptions					(2, 1, 3.0f, 0.5f, olc::BLACK, olc::Pixel{ 255, 191, 0 }, olc::DARK_GREY, olc::GREY, PGE_GAME_2D_ENABLED);

		// Scrolling Tile
		PGE_GAME_2D_BACKEND::SetScrollingTile	({ 512, 512 }, rm.RM_Sprite("./assets/img/circuit.png"));

		// Add menus
		menu.InitMenus();

		// Add Audio
		AddAudioSources();

		if (AL.bMusicOn)
			AS[BG_MUS].Play(1.0f, 0.75f, true);

		// Game Core and UI
		game.Init	(rm.RM_Sprite("./assets/img/title.png"), rm.RM_Sprite("./assets/img/info.png"));
		ui.Init		(rm.RM_Sprite("./assets/img/ui.png"));
		camera.vecCamPos = fVecZero;

		// Game State
		nGameState = MAIN_MENU;
		nNextGameState = MAIN_MENU;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Exit
		if (bReturnFalseNextFrame)
			return false;

		// First Update Frame
		if (bFirstUpdateFrame)
		{
			StartScreenTransition(BLACK, TO_TRANSPARENT, 2.0f);
			nNextGameState = MAIN_MENU;
		}

		// Splash Screen
		if (bSplashScreenOn && splashScreen.AnimateSplashScreen(fElapsedTime))
		{
			UpdateGAME2D(fVecZero);
			return true;
		}

		// Update GAME2D
		UpdateGAME2D(game.PointOfFocus());

		// Game States
		switch (nGameState)
		{
		case MAIN_MENU:
		{
			// Draw menu graphics
			game.DrawGradient(this);
			game.DrawTitle(this);

			// User Initiated Exit
			if (GetKey(olc::Key::ESCAPE).bReleased)
				bReturnFalseNextFrame = true;

			// Menu logic
			int nMenuID = -1;
			bool bPlaySound = false;

			if (!menu.Update(this, nMenuID, bPlaySound))
				bReturnFalseNextFrame = true;

			// Process menu interactions
			if (menu.nCurrentMenu == cMenu::START_MENU)
			{
				// Update instructions - every 4.0 seconds the information changes... (5 information scenes in total)
				const float fTickLimit = 20.0f;
				game.UpdateAndClampUITick(fElapsedTime, fTickLimit);
				game.DrawInstructions(this);
			}

			if (nMenuID == cMenu::MENU_ID::NEW_GAME)
			{
				const olc::vf2d vfCenterOfMap = { 360.0f, 360.0f };
				game.PointOfFocus(vfCenterOfMap);
			}

			if (nMenuID == cMenu::MENU_ID::START)
			{
				game.nSystemMem = 8;
				nNextGameState = RESET;
			}

			if (nMenuID == cMenu::MENU_ID::SOUND)
			{
				AL.bSoundOn = !AL.bSoundOn;
				AL.bMusicOn = !AL.bMusicOn;

				if (AL.bMusicOn)
					AS[BG_MUS].Play(1.0f, 0.5f, true);
				else
					(AS[BG_MUS].Stop());

			}

			if (bPlaySound && AL.bSoundOn)
				AS[DEFAULT_MENU_SND].Play();

		}
		break;

		case RESET:
		{
			game.Reset(rm.RM_Sprite("./assets/img/twok.png"));
			hud.Reset(game.nSystemMem);
			
			ui.nUIState = cUI::UI_STATE::EMPTY;
			nNextGameState = GAME_PLAY;
		}
		break;

		case GAME_PLAY:
		{
			// Forfeit Game
			if (GetKey(olc::Key::ESCAPE).bReleased)
				hud.bCrashOccured = true;

			// Cache and Clamp
			const olc::vf2d vfMouseWorldPos = GetMousePos() + camera.vecCamPos;
			olc::vf2d tilePos = game.GetMapPosFromMousePos(vfMouseWorldPos);
			game.ClampTilePos(tilePos);

			// Get info about tile currently under mouse position
			int nSelectedTileType = 0;
			bool bTileOccupied = game.TileOccupied(tilePos, nSelectedTileType);

			// Cache for selected memBlocks/units
			cMemBlock* mSelectedBlock = nullptr;
			cUnit* uSelectedUnit = nullptr;

			// Update memBlocks
			game.SecondsUntilNextMemBlock(game.SecondsUntilNextMemBlock() - fElapsedTime);

			// Set UI state;
			ui.nUIState = cUI::UI_STATE::EMPTY;

			if (bTileOccupied)
			{
				if (nSelectedTileType == cUnit::MEMORY)
				{
					mSelectedBlock = game.GetMemBlock(tilePos);

					if (mSelectedBlock->bUsed && !mSelectedBlock->bFreed)
						ui.nUIState = cUI::UI_STATE::FREE_MEM;
					else
						ui.nUIState = cUI::UI_STATE::INVALID;

				}
				else if (nSelectedTileType == cUnit::SOCKET_1K)
				{
					uSelectedUnit = game.GetUnit(tilePos);

					if (!uSelectedUnit->bInUse && !uSelectedUnit->bUsed)
						ui.nUIState = cUI::UI_STATE::ROACH_1K;
					else if (uSelectedUnit->bUsed)
						ui.nUIState = cUI::UI_STATE::INVALID;

				}
				else if (nSelectedTileType == cUnit::SOCKET_2K)
				{
					uSelectedUnit = game.GetUnit(tilePos);

					if (!uSelectedUnit->bInUse && !uSelectedUnit->bUsed)
						ui.nUIState = cUI::UI_STATE::ROACH_2K;
					else if (uSelectedUnit->bUsed)
						ui.nUIState = cUI::UI_STATE::INVALID;

				}
			}

			// Input
			if (GetKey(olc::D).bPressed)
			{
				const bool bToggleDebugMode = true;
				game.DebugOn(bToggleDebugMode);
			}

			// Mouse Control
			const olc::vi2d viMousePos = GetMousePos();
			game.ProcessScreenScroll(viMousePos, fElapsedTime);

			if (hud.bCrashOccured || hud.bGameWon)
				game.PointOfFocus(game.vfCenterOfMap);

			if (GetMouse(0).bPressed)
			{
				game.ResetMouseButtonHeldState(0);
				ui.nUIState = cUI::UI_STATE::EMPTY;
			}

			if (GetMouse(1).bPressed)
			{
				game.ResetMouseButtonHeldState(1);
				ui.nUIState = cUI::UI_STATE::EMPTY;
			}

			if (!GetMouse(1).bHeld && GetMouse(0).bHeld && ui.nUIState != cUI::UI_STATE::INVALID)
			{
				game.MouseButtonHeldFor(0, fElapsedTime);

				if (game.MouseButtonHeldFor(0) > 0.5f)
				{
					float fAlpha = 255.0f * game.MouseButtonHeldFor(0) * 0.5f;
					if (fAlpha > 255.0f)
						fAlpha = 255.0f;

					olc::Pixel pHighlight = olc::WHITE;

					if (ui.nUIState == cUI::UI_STATE::ROACH_1K || ui.nUIState == cUI::UI_STATE::ROACH_2K)
					{
						ui.nUIState = cUI::UI_STATE::DECONSTRUCT;
						pHighlight = { 255, 144, 0, (uint8_t)fAlpha };
					
						FillRectDecal(tilePos - camera.vecCamPos, { 128.0f * (fAlpha / 255.0f) , 128.0f }, pHighlight);
					}
					else if (ui.nUIState == cUI::UI_STATE::EMPTY && game.nCurrentAmountOfSockets < hud.nMaxSockets)
					{
						ui.nUIState = cUI::UI_STATE::CONSTRUCT_1K;
						pHighlight = { 47, 114, 143, (uint8_t)fAlpha };
						if (game.MouseButtonHeldFor(0) > 1.5f)
						{
							pHighlight = { 132, 47, 144, (uint8_t)fAlpha };
							ui.nUIState = cUI::UI_STATE::CONSTRUCT_2K;
						}

						FillRectDecal(tilePos - camera.vecCamPos, { 128.0f * (fAlpha / 255.0f) , 128.0f }, pHighlight);
					}
				}
			}

			if (!GetMouse(0).bHeld && GetMouse(1).bHeld)
			{
				game.MouseButtonHeldFor(1, fElapsedTime);

				if (game.MouseButtonHeldFor(1) > 0.25f)
				{
					ui.nUIState = cUI::UI_STATE::SET_POINTER;

					float fAlpha = 255.0f * game.MouseButtonHeldFor(1);
					if (fAlpha > 255.0f)
						fAlpha = 255.0f;

					olc::Pixel pHighlight = olc::Pixel{ 0, 191, 127, (uint8_t)fAlpha };
					FillRectDecal(tilePos - camera.vecCamPos, { 128.0f * (fAlpha / 255.0f) , 128.0f }, pHighlight);
				}
			}

			if (GetMouse(0).bReleased && game.MouseButtonHeldFor(0) < 0.5f && !hud.bGameWon && !hud.bCrashOccured && ui.nUIState != cUI::UI_STATE::INVALID)
			{
				game.ResetMouseButtonHeldState(0);

				if (bTileOccupied)
				{
					const olc::vf2d vfRandomAxis = game.GenerateVFAxis();

					if (ui.nUIState == cUI::UI_STATE::FREE_MEM)
					{
						mSelectedBlock->bFreed = true;
						uSelectedUnit = game.GetUnit(tilePos);

						if (AL.bSoundOn)
							AS[MEM_FREED].Play();

					}
					if (nSelectedTileType == cUnit::SOCKET_1K)
					{
						uSelectedUnit->Interact();
						game.units.emplace_back(cUnit(tilePos, vfRandomAxis, 0, 1.2f, rm.RM_Sprite("./assets/img/eprom.png")));
						uSelectedUnit = game.GetUnit(tilePos);

						if (AL.bSoundOn)
							AS[NEW_UNIT].Play(1.0f, 0.5f);

					}
					else if (nSelectedTileType == cUnit::SOCKET_2K)
					{
						uSelectedUnit->Interact();
						game.units.emplace_back(cUnit(tilePos, vfRandomAxis, 1, 2.0f, rm.RM_Sprite("./assets/img/eprom.png")));
						uSelectedUnit = game.GetUnit(tilePos);

						if (AL.bSoundOn)
							AS[NEW_UNIT].Play(1.0f, 0.5f);

					}
				}
			}

			if (GetMouse(0).bReleased && game.MouseButtonHeldFor(0) >= 0.5f && game.MouseButtonHeldFor(0) < 1.5f && !hud.bGameWon && !hud.bCrashOccured && ui.nUIState != cUI::UI_STATE::INVALID)
			{
				game.ResetMouseButtonHeldState(0);

				if (!bTileOccupied)
				{
					if (game.nCurrentAmountOfSockets < hud.nMaxSockets)
					{
						game.AddSocket(tilePos, cUnit::SOCKET_1K, rm.RM_Sprite("./assets/img/sockets.png"));

						if (AL.bSoundOn)
						AS[PLACE_SOCKET].Play();

					}
				}
			}

			if (GetMouse(0).bReleased && game.MouseButtonHeldFor(0) >= 1.5f && !hud.bGameWon && !hud.bCrashOccured && ui.nUIState != cUI::UI_STATE::INVALID)
			{
				game.ResetMouseButtonHeldState(0);

				if (!bTileOccupied)
				{
					if (game.nCurrentAmountOfSockets < hud.nMaxSockets)
					{
						game.AddSocket(tilePos, cUnit::SOCKET_2K, rm.RM_Sprite("./assets/img/sockets.png"));

						if (AL.bSoundOn)
							AS[PLACE_SOCKET].Play();

					}
				}
				else if (uSelectedUnit != nullptr)
				{
					if (uSelectedUnit->nType >= cUnit::SOCKET_1K && !uSelectedUnit->bUsed)
					{
						const bool bDeconstructUnit = true;
						uSelectedUnit->Interact(bDeconstructUnit);
						game.nCurrentAmountOfSockets--;
						uSelectedUnit = game.GetUnit(tilePos);

						if (AL.bSoundOn)
							AS[MEM_FREED].Play();

					}
				}
			}

			if (GetMouse(1).bReleased && game.MouseButtonHeldFor(1) > 0.25f && !hud.bGameWon && !hud.bCrashOccured)
			{
				game.ResetMouseButtonHeldState(1);
				game.PlacePointer(tilePos);

				if (AL.bSoundOn)
					AS[NEW_UNIT].Play(1.0f, 0.5f);

			}

			// Generate Mem Blocks
			if (game.SecondsUntilNextMemBlock() <= 0.0f && !hud.bGameWon && !hud.bCrashOccured)
			{
				const olc::vf2d vfNextBlockPos = game.ChooseNextMemBlockPos(game.LocationOfLastMemBlock());

				if (!game.TilePosWithinMapBounds(vfNextBlockPos))
				{
					// Can't generate block, use mouse cursor as a quick hack
					if (game.GenerateMemBlock(tilePos, rm.RM_Sprite("./assets/img/twok.png")))
					{
						if (AL.bSoundOn)
							AS[DEFAULT_MENU_SND].Play();
					
					}

					game.LocationOfLastMemBlock(tilePos);
				}
				else
				{
					if (game.GenerateMemBlock(vfNextBlockPos, rm.RM_Sprite("./assets/img/twok.png")))
					{
						if (AL.bSoundOn)
							AS[DEFAULT_MENU_SND].Play();

					}

					game.LocationOfLastMemBlock(vfNextBlockPos);
				}
			}

			// Units
			const bool bUnitsOnly = true;

			for (auto& u : game.units)
			{
				u.Update(game.PointerPos("A"), game.PointerPos("B"), fElapsedTime);

				if (u.bPlaySpawnSound)
				{
					u.bPlaySpawnSound = false;

					if (AL.bSoundOn)
						AS[DEFAULT_MENU_SND].Play();

				}

				u.Draw(camera.vecCamPos, !bUnitsOnly);
			}

			for (auto& u : game.units)
			{
				u.Draw(camera.vecCamPos, bUnitsOnly);

				// Check if any units are newly inside a memory block
				if (u.nType == cUnit::EPROM_1K || u.nType == cUnit::EPROM_2K)
				{
					if (!u.bInUse)
					{
						olc::vf2d vfUnitPos = game.GetMapPosFromMousePos(u.vfPos);
						cMemBlock* mBlock = game.GetMemBlock(vfUnitPos);
						const olc::vf2d vfInvalidPos = { -999.0f, -999.0f };

						if (mBlock->vfPos != vfInvalidPos && !mBlock->bInUse && !mBlock->bUsed)
						{
							u.Interact();
							u.vfPos = mBlock->vfPos;
							mBlock->CombineWithSocket(u.fSecondsUntilUsed);

							if (AL.bSoundOn)
								AS[MEM_LOCK].Play();

						}
					}
				}
			}

			// MemBlocks
			for (auto& m : game.memBlocks)
			{
				if (m.Update(fElapsedTime))
				{
					if (AL.bSoundOn)
						AS[FREE_MEM_AVAIL].Play();

				}

				m.Draw(this, camera.vecCamPos);
			}

			// Pointers
			ui.DrawPointers(this, game.PointerPos("A"), game.PointerPos("B"), camera.vecCamPos);

			// Gradient
			game.DrawGradient(this);

			// UI
			if (!hud.bCrashOccured && !hud.bGameWon)
				ui.Draw(this, tilePos - camera.vecCamPos, fElapsedTime);

			// Bounding Box
			game.DrawBoundingBox(this, camera.vecCamPos);

			// HUD
			const int nUsedMem =	game.GetUsedMemory				();
			const int nFreeMem =	game.GetFreeMemory				();
			int nAvaibleMem =		game.CalculateAvailableMemory	(hud.bCrashOccured);

			// Lose condition
			if (nAvaibleMem < 0 && !hud.bGameWon)
			{
				hud.bCrashOccured = true;

				if (AL.bSoundOn)
					AS[CRASH].Play();

			}

			// Win condition
			if (nUsedMem == 0 && nFreeMem == 0 && !hud.bGameWon && !hud.bCrashOccured)
			{
				hud.bGameWon = true;

				if (AL.bSoundOn)
					AS[WIN].Play();

			}

			// Render HUD
			hud.Draw(this, game.nSystemMem, nUsedMem, nAvaibleMem, nFreeMem, game.nCurrentAmountOfSockets, fElapsedTime);

			// Process end game HUD logic
			if (hud.bCrashOccured || hud.bGameWon)
			{
				hud.fCountDownToSummary -= fElapsedTime;

				if (hud.fCountDownToSummary <= 0.0f)
				{
					game.PointOfFocus({ 0.0f, 0.0f });

					hud.fCountDownToSummary = 4.0f;
					hud.bMenuMode = true;

					// Check to see whether the game is complete
					if (game.nSystemMem == 128 && hud.bGameWon)
					{
						menu.nCurrentMenu = 0; // Main Menu
						menu.mainMenu.StartTransition(1.0f, 1.2f);
						menu.startMenu.bVisible = false;

						hud.bGameComplete = true;

						nNextGameState = GAME_OVER;
					}
					else
					{
						menu.nCurrentMenu = 2; // In-Game Menu
						menu.inGameMenu.StartTransition(1.0f, 1.2f);
						menu.startMenu.bVisible = false;

						if (hud.bCrashOccured)
							menu.inGameMenu.GetMenuItemByID(cMenu::CONTINUE)->bToggledOff = true;

						if (hud.bGameWon)
							menu.inGameMenu.GetMenuItemByID(cMenu::CONTINUE)->bToggledOff = false;

						nNextGameState = SUMMARY;
					}
				}
			}

			// Debug
			if (game.DebugOn())
			{
				// Add debug content here
				//
			}
		}
		break;

		case SUMMARY:
		{
			game.DrawGradient(this);
			hud.Draw(this, game.nSystemMem, game.GetUsedMemory(), game.CalculateAvailableMemory(hud.bCrashOccured), game.GetFreeMemory(), game.nCurrentAmountOfSockets, fElapsedTime);

			int nMenuID = -1;
			bool bPlaySound = false;

			if (!menu.Update(this, nMenuID, bPlaySound))
				bReturnFalseNextFrame = true;

			if (nMenuID == 7) // Continue
			{
				if (game.nSystemMem < 128)
					game.nSystemMem *= 2;

				nNextGameState = RESET;
			}

			if (nMenuID == 8) // Retry
			{
				nNextGameState = RESET;
			}

			if (nMenuID == 9) // Menu
			{
				nNextGameState = MAIN_MENU;
			}

			if (bPlaySound && AL.bSoundOn)
				AS[DEFAULT_MENU_SND].Play();

		}
		break;

		case GAME_OVER:
		{
			game.DrawGradient(this);
			hud.Draw(this, game.nSystemMem, game.GetUsedMemory(), game.CalculateAvailableMemory(hud.bCrashOccured), game.GetFreeMemory(), game.nCurrentAmountOfSockets, fElapsedTime);

			game.fUITick += fElapsedTime;
			if (game.fUITick >= 8.0f)
			{
				game.fUITick -= 8.0f;
				nNextGameState = MAIN_MENU;
			}
		}
		break;

		default:
			break;

		}

		// Late Update GAME2D
		LateUpdateGAME2D();

		return true;
	}
};

using namespace GAME2D;
int main()
{
	CodeJam2023 jam;
	if (jam.Construct(720, 720, 1, 1, WINDOWED, VSYNC_ON))
		jam.Start();

	return 0;
}
