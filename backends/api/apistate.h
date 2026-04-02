/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BACKENDS_API_APISTATE_H
#define BACKENDS_API_APISTATE_H

#ifdef USE_SCUMM_API

#include <string>
#include <vector>

namespace ScummApi {

/**
 * Game state enum — indicates what the player is currently experiencing.
 * This determines what actions are valid (e.g. dialog choices during DIALOG).
 */
enum class GameState {
	GAMEPLAY,
	CUTSCENE,
	DIALOG,
	FROZEN
};

inline const char *gameStateToString(GameState state) {
	switch (state) {
	case GameState::GAMEPLAY:  return "gameplay";
	case GameState::CUTSCENE:  return "cutscene";
	case GameState::DIALOG:    return "dialog";
	case GameState::FROZEN:    return "frozen";
	}
	return "unknown";
}

/**
 * Information about a single actor visible in the current room.
 * All fields are plain values — safe to copy across thread boundaries.
 */
struct ActorInfo {
	int id = 0;
	std::string name;
	int x = 0;
	int y = 0;
	bool isMoving = false;
	bool isTalking = false;
	bool isVisible = false;
	int facing = 0;
	int room = 0;
};

/**
 * An inventory item belonging to the player.
 */
struct ItemInfo {
	int id = 0;
	std::string name;
};

/**
 * An active verb slot in the UI (e.g. "Open", "Pick up", "Use").
 */
struct VerbInfo {
	int id = 0;
	std::string label;
	int key = 0;
};

/**
 * An interactive object (hotspot) in the current room.
 */
struct HotspotInfo {
	int id = 0;
	std::string name;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int walkToX = 0;
	int walkToY = 0;
};

/**
 * Quad coordinates for a walk box — four corner points.
 */
struct BoxCoordsInfo {
	int ulX = 0, ulY = 0;  // upper-left
	int urX = 0, urY = 0;  // upper-right
	int llX = 0, llY = 0;  // lower-left
	int lrX = 0, lrY = 0;  // lower-right
};

/**
 * A walkable/trigger box in the current room.
 */
struct BoxInfo {
	int id = 0;
	BoxCoordsInfo coords;
	int flags = 0;
	bool isWalkable = false;
};

/**
 * Current state of the dialog system.
 */
struct DialogChoiceInfo {
	int index = 0;
	std::string text;
};

struct DialogState {
	bool isActive = false;
	int speakerId = 0;
	std::string speakerName;
	std::string currentLine;
	std::vector<DialogChoiceInfo> choices;
};

/**
 * ScummApiSnapshot — a complete, thread-safe snapshot of the SCUMM engine state.
 *
 * This struct is a pure value type: all std::string and std::vector fields
 * are owned copies. No raw pointers into engine memory cross the thread boundary.
 * The game loop publishes a new snapshot each tick under a mutex; the HTTP thread
 * reads the latest snapshot without touching engine internals.
 */
struct ScummApiSnapshot {
	// Location
	int currentRoom = 0;
	std::string roomName;
	int cameraX = 0;
	int cameraY = 0;
	int roomWidth = 0;
	int roomHeight = 0;
	int screenWidth = 0;
	int screenHeight = 0;

	// Game state
	GameState gameState = GameState::GAMEPLAY;
	bool haveMessage = false;
	int talkingActorId = 0;

	// Actors in current room
	std::vector<ActorInfo> actors;

	// Player inventory
	std::vector<ItemInfo> inventory;

	// Active verb slots
	std::vector<VerbInfo> verbs;

	// Interactive objects in current room
	std::vector<HotspotInfo> hotspots;

	// Walk boxes
	std::vector<BoxInfo> walkableBoxes;
	std::vector<BoxInfo> allBoxes;

	// Dialog state
	DialogState dialog;

	// Engine readiness
	bool isEngineReady = false;
};

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // BACKENDS_API_APISTATE_H
