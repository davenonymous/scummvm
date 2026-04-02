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

// Disable symbol overrides so that std::string/std::vector work correctly
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#ifndef USE_SCUMM_API
#define USE_SCUMM_API
#endif

#ifdef USE_SCUMM_API

#include "scumm/api/statebuilder.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/verbs.h"
#include "scumm/object.h"
#include "scumm/boxes.h"
#include "backends/api/apistate.h"

namespace ScummApi {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string bytesToString(const byte *data) {
	if (!data)
		return "";
	return std::string(reinterpret_cast<const char *>(data));
}

/** Convert raw SCUMM name bytes to a clean string, stripping '@' padding. */
static std::string cleanName(const byte *data) {
	if (!data)
		return "";
	std::string result;
	for (int i = 0; data[i] != 0; ++i) {
		if (data[i] != '@')
			result += static_cast<char>(data[i]);
	}
	return result;
}

/** Decode a SCUMM message resource (with embedded control codes) into
 *  readable text. Falls back to raw bytesToString if vm is null. */
static std::string decodeVerbLabel(Scumm::ScummEngine *vm, const byte *data) {
	if (!data)
		return "";
	if (!vm)
		return bytesToString(data);

	byte buf[270];
	vm->apiConvertMessage(data, buf, sizeof(buf));

	// Strip any remaining 0xFF-prefixed control codes from the output
	// (e.g. color/animation escapes that aren't printable text).
	std::string result;
	for (int i = 0; buf[i] != 0; ++i) {
		if (buf[i] == 0xFF) {
			// Skip past the entire escape sequence:
			// 0xFF + cmd byte + optional parameter bytes.
			++i; // advance to the command byte
			byte cmd = buf[i];
			// Commands 4-7, 9, 10, 12-14 have 2 parameter bytes;
			// commands 1, 2, 3, 8 have none.
			if (cmd == 4 || cmd == 5 || cmd == 6 || cmd == 7 ||
			    cmd == 9 || cmd == 10 || cmd == 12 || cmd == 13 || cmd == 14) {
				i += 2;
			}
			// i now points at the last byte of the escape sequence;
			// the for-loop's ++i will move past it.
		} else if (buf[i] == 0x10) {
			// SCUMM word-break marker — rendered as a space on screen.
			result += ' ';
		} else if (buf[i] >= 0x20 && buf[i] != 0x7F) {
			result += static_cast<char>(buf[i]);
		}
	}
	return result;
}

static std::string intToString(int value) {
	char buf[32];
	snprintf(buf, sizeof(buf), "%d", value);
	return std::string(buf);
}

std::string jsonEscape(const std::string &s) {
	std::string out;
	out.reserve(s.size() + 8);
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char ch = static_cast<unsigned char>(s[i]);
		switch (ch) {
		case '"':  out += "\\\""; break;
		case '\\': out += "\\\\"; break;
		case '\n': out += "\\n";  break;
		case '\r': out += "\\r";  break;
		case '\t': out += "\\t";  break;
		case '\b': out += "\\b";  break;
		case '\f': out += "\\f";  break;
		default:
			if (ch < 0x20) {
				// Escape other control characters as \u00XX
				char esc[8];
				snprintf(esc, sizeof(esc), "\\u%04x", ch);
				out += esc;
			} else {
				out += static_cast<char>(ch);
			}
			break;
		}
	}
	return out;
}

// ---------------------------------------------------------------------------
// Tiny JSON builder helpers — produce fragments, not full documents.
// ---------------------------------------------------------------------------

static std::string jsonString(const std::string &value) {
	return "\"" + jsonEscape(value) + "\"";
}

static std::string jsonInt(int value) {
	return intToString(value);
}

static std::string jsonBool(bool value) {
	return value ? "true" : "false";
}

// ---------------------------------------------------------------------------
// buildSnapshot
// ---------------------------------------------------------------------------

/** Check if a verb slot is a dialog choice rather than a regular UI verb.
 *  Dialog choice verbs are text-type slots whose raw resource data contains
 *  SCUMM control codes (starting with non-ASCII bytes like 0x7F or 0xFF)
 *  that reference string variables holding the actual choice text.
 *  Alternatively, in v6+ games, normal verbs are "saved" (saveid != 0)
 *  during a dialog, and the choice verbs have saveid == 0. */
static bool isDialogChoiceVerb(Scumm::ScummEngine *vm, int slotIndex) {
	const Scumm::VerbSlot &vs = vm->_verbs[slotIndex];
	if (vs.verbid == 0 || vs.curmode != 1)
		return false;
	if (vs.type == Scumm::kImageVerbType)
		return false;

	const byte *data = vm->getResourceAddress(Scumm::rtVerb, slotIndex);
	if (!data)
		return false;

	// If the raw resource starts with a non-printable byte or a SCUMM
	// escape sequence, it is a dialog choice (not a plain-text verb label).
	return (data[0] < 0x20 || data[0] >= 0x7F);
}

static bool hasActiveChoiceScreen(Scumm::ScummEngine *vm) {
	// Check method 1: any verb has been "saved away" (v6+ dialog pattern)
	for (int i = 1; i < vm->apiGetNumVerbs(); ++i) {
		if (vm->_verbs[i].saveid != 0)
			return true;
	}
	// Check method 2: any active verb slot contains encoded choice text
	// (v5 pattern — normal verbs stay visible alongside choices)
	for (int i = 1; i < vm->apiGetNumVerbs(); ++i) {
		if (isDialogChoiceVerb(vm, i))
			return true;
	}
	return false;
}

static GameState determineGameState(Scumm::ScummEngine *vm) {
	if (vm->_haveMsg != 0)
		return GameState::DIALOG;

	if (hasActiveChoiceScreen(vm))
		return GameState::DIALOG;

	// The engine does not expose a single "isCutscene" flag,
	// so we default to GAMEPLAY.
	return GameState::GAMEPLAY;
}

static void collectActors(Scumm::ScummEngine *vm, ScummApiSnapshot &snap) {
	int talkingActorId = vm->getTalkingActor();

	for (int i = 0; i < vm->apiGetNumActors(); ++i) {
		Scumm::Actor *actor = vm->apiGetActor(i);
		if (!actor)
			continue;
		if (!actor->_visible)
			continue;
		if (actor->_room != vm->_currentRoom)
			continue;

		ActorInfo info;
		info.id = actor->_number;
		info.name = bytesToString(actor->getActorName());

		Common::Point pos = actor->getPos();
		info.x = pos.x;
		info.y = pos.y;

		info.isMoving = (actor->_moving != 0);
		info.isTalking = (talkingActorId == actor->_number);
		info.isVisible = actor->_visible;
		info.facing = actor->getFacing();
		info.room = actor->_room;

		snap.actors.push_back(info);
	}
}

static void collectInventory(Scumm::ScummEngine *vm, ScummApiSnapshot &snap) {
	int playerId = vm->apiGetPlayerActor();

	for (int i = 1; i < vm->apiGetNumInventory(); ++i) {
		uint16 objId = vm->apiGetInventoryObj(i);
		if (objId == 0)
			continue;

		// Only include items owned by the current player actor.
		int owner = vm->apiGetOwner(objId);
		if (owner != playerId)
			continue;

		ItemInfo item;
		item.id = objId;
		item.name = cleanName(vm->apiGetObjOrActorName(objId));

		snap.inventory.push_back(item);
	}
}

static void collectVerbs(Scumm::ScummEngine *vm, ScummApiSnapshot &snap) {
	for (int i = 1; i < vm->apiGetNumVerbs(); ++i) {
		const Scumm::VerbSlot &vs = vm->_verbs[i];
		if (vs.verbid == 0)
			continue;
		if (vs.curmode != 1)
			continue;
		// Skip image-type verbs (inventory arrows, UI bitmaps) —
		// their resource data is an OBIM image, not text.
		if (vs.type == Scumm::kImageVerbType)
			continue;
		// Skip dialog choice verbs — they appear in dialog.choices instead.
		if (isDialogChoiceVerb(vm, i))
			continue;

		VerbInfo info;
		info.id = vs.verbid;
		info.key = vs.key;

		// Verb label text is stored as a resource at rtVerb slot i.
		// Use convertMessageToString to decode embedded SCUMM control
		// codes (e.g. string variable references) into readable text.
		const byte *labelData = vm->getResourceAddress(Scumm::rtVerb, i);
		info.label = decodeVerbLabel(vm, labelData);

		snap.verbs.push_back(info);
	}
}

static void collectHotspots(Scumm::ScummEngine *vm, ScummApiSnapshot &snap) {
	for (int i = 0; i < vm->apiGetNumLocalObjects(); ++i) {
		const Scumm::ObjectData &obj = vm->_objs[i];
		if (obj.obj_nr == 0)
			continue;

		HotspotInfo info;
		info.id = obj.obj_nr;
		info.name = cleanName(vm->apiGetObjOrActorName(obj.obj_nr));
		info.x = obj.x_pos;
		info.y = obj.y_pos;
		info.width = obj.width;
		info.height = obj.height;
		info.walkToX = obj.walk_x;
		info.walkToY = obj.walk_y;

		snap.hotspots.push_back(info);
	}
}

static void collectBoxes(Scumm::ScummEngine *vm, ScummApiSnapshot &snap) {
	int numBoxes = vm->getNumBoxes();

	for (int i = 0; i < numBoxes; ++i) {
		Scumm::BoxCoords coords = vm->getBoxCoordinates(i);
		byte flags = vm->getBoxFlags(i);

		BoxInfo info;
		info.id = i;
		info.coords.ulX = coords.ul.x;
		info.coords.ulY = coords.ul.y;
		info.coords.urX = coords.ur.x;
		info.coords.urY = coords.ur.y;
		info.coords.llX = coords.ll.x;
		info.coords.llY = coords.ll.y;
		info.coords.lrX = coords.lr.x;
		info.coords.lrY = coords.lr.y;
		info.flags = flags;
		info.isWalkable = !(flags & Scumm::kBoxLocked) && !(flags & Scumm::kBoxInvisible);

		snap.allBoxes.push_back(info);
		if (info.isWalkable)
			snap.walkableBoxes.push_back(info);
	}
}

static void collectDialog(Scumm::ScummEngine *vm, ScummApiSnapshot &snap) {
	bool choiceScreen = hasActiveChoiceScreen(vm);
	snap.dialog.isActive = (vm->_haveMsg != 0) || choiceScreen;
	if (!snap.dialog.isActive)
		return;

	// Speaker info (meaningful during speech lines, not choice menus)
	int talkingId = vm->getTalkingActor();
	snap.dialog.speakerId = talkingId;

	if (talkingId > 0 && talkingId < vm->apiGetNumActors()) {
		Scumm::Actor *talkActor = vm->apiGetActor(talkingId);
		if (talkActor)
			snap.dialog.speakerName = bytesToString(talkActor->getActorName());
	}

	// Collect dialog choices: verb slots identified as choices
	// (either saved-verb pattern in v6+ or encoded-text pattern in v5).
	if (!choiceScreen)
		return;

	int choiceIndex = 0;
	for (int i = 1; i < vm->apiGetNumVerbs(); ++i) {
		if (!isDialogChoiceVerb(vm, i))
			continue;

		const byte *labelData = vm->getResourceAddress(Scumm::rtVerb, i);

		DialogChoiceInfo choice;
		choice.index = choiceIndex++;
		choice.text = decodeVerbLabel(vm, labelData);
		snap.dialog.choices.push_back(choice);
	}

	// A real dialog choice screen always has at least 2 options.
	// A single entry is a false positive from a transient verb state.
	if (snap.dialog.choices.size() < 2)
		snap.dialog.choices.clear();
}

ScummApiSnapshot buildSnapshot(Scumm::ScummEngine *vm) {
	ScummApiSnapshot snap;

	if (!vm) {
		snap.isEngineReady = false;
		return snap;
	}

	snap.isEngineReady = true;

	// Room metadata
	snap.currentRoom = vm->_currentRoom;
	snap.roomWidth = vm->_roomWidth;
	snap.roomHeight = vm->_roomHeight;
	snap.screenWidth = vm->_screenWidth;
	snap.screenHeight = vm->_screenHeight;
	snap.cameraX = vm->camera._cur.x;

	// Game state flags
	snap.haveMessage = (vm->_haveMsg != 0);
	snap.talkingActorId = vm->getTalkingActor();
	snap.gameState = determineGameState(vm);

	// Collect all sub-sections
	collectActors(vm, snap);
	collectInventory(vm, snap);
	collectVerbs(vm, snap);
	collectHotspots(vm, snap);
	collectBoxes(vm, snap);
	collectDialog(vm, snap);

	return snap;
}

// ---------------------------------------------------------------------------
// JSON serialization — per-section
// ---------------------------------------------------------------------------

static std::string actorToJsonObject(const ActorInfo &a) {
	std::string json = "{";
	json += "\"id\":" + jsonInt(a.id);
	json += ",\"name\":" + jsonString(a.name);
	json += ",\"x\":" + jsonInt(a.x);
	json += ",\"y\":" + jsonInt(a.y);
	json += ",\"isMoving\":" + jsonBool(a.isMoving);
	json += ",\"isTalking\":" + jsonBool(a.isTalking);
	json += ",\"isVisible\":" + jsonBool(a.isVisible);
	json += ",\"facing\":" + jsonInt(a.facing);
	json += ",\"room\":" + jsonInt(a.room);
	json += "}";
	return json;
}

std::string actorsToJson(const ScummApiSnapshot &snap) {
	std::string json = "[";
	for (size_t i = 0; i < snap.actors.size(); ++i) {
		if (i > 0)
			json += ",";
		json += actorToJsonObject(snap.actors[i]);
	}
	json += "]";
	return json;
}

std::string inventoryToJson(const ScummApiSnapshot &snap) {
	std::string json = "[";
	for (size_t i = 0; i < snap.inventory.size(); ++i) {
		if (i > 0)
			json += ",";
		json += "{\"id\":" + jsonInt(snap.inventory[i].id);
		json += ",\"name\":" + jsonString(snap.inventory[i].name);
		json += "}";
	}
	json += "]";
	return json;
}

std::string verbsToJson(const ScummApiSnapshot &snap) {
	std::string json = "[";
	for (size_t i = 0; i < snap.verbs.size(); ++i) {
		if (i > 0)
			json += ",";
		json += "{\"id\":" + jsonInt(snap.verbs[i].id);
		json += ",\"label\":" + jsonString(snap.verbs[i].label);
		json += ",\"key\":" + jsonInt(snap.verbs[i].key);
		json += "}";
	}
	json += "]";
	return json;
}

std::string hotspotsToJson(const ScummApiSnapshot &snap) {
	std::string json = "[";
	for (size_t i = 0; i < snap.hotspots.size(); ++i) {
		if (i > 0)
			json += ",";
		const HotspotInfo &h = snap.hotspots[i];
		json += "{\"id\":" + jsonInt(h.id);
		json += ",\"name\":" + jsonString(h.name);
		json += ",\"x\":" + jsonInt(h.x);
		json += ",\"y\":" + jsonInt(h.y);
		json += ",\"width\":" + jsonInt(h.width);
		json += ",\"height\":" + jsonInt(h.height);
		json += ",\"walkToX\":" + jsonInt(h.walkToX);
		json += ",\"walkToY\":" + jsonInt(h.walkToY);
		json += "}";
	}
	json += "]";
	return json;
}

static std::string boxInfoToJsonObject(const BoxInfo &b) {
	std::string json = "{";
	json += "\"id\":" + jsonInt(b.id);
	json += ",\"coords\":{";
	json += "\"ulX\":" + jsonInt(b.coords.ulX);
	json += ",\"ulY\":" + jsonInt(b.coords.ulY);
	json += ",\"urX\":" + jsonInt(b.coords.urX);
	json += ",\"urY\":" + jsonInt(b.coords.urY);
	json += ",\"llX\":" + jsonInt(b.coords.llX);
	json += ",\"llY\":" + jsonInt(b.coords.llY);
	json += ",\"lrX\":" + jsonInt(b.coords.lrX);
	json += ",\"lrY\":" + jsonInt(b.coords.lrY);
	json += "}";
	json += ",\"flags\":" + jsonInt(b.flags);
	json += ",\"isWalkable\":" + jsonBool(b.isWalkable);
	json += "}";
	return json;
}

static std::string boxArrayToJson(const std::vector<BoxInfo> &boxes) {
	std::string json = "[";
	for (size_t i = 0; i < boxes.size(); ++i) {
		if (i > 0)
			json += ",";
		json += boxInfoToJsonObject(boxes[i]);
	}
	json += "]";
	return json;
}

std::string boxesToJson(const ScummApiSnapshot &snap) {
	std::string json = "{";
	json += "\"walkable\":" + boxArrayToJson(snap.walkableBoxes);
	json += ",\"all\":" + boxArrayToJson(snap.allBoxes);
	json += "}";
	return json;
}

std::string roomToJson(const ScummApiSnapshot &snap) {
	std::string json = "{";
	json += "\"currentRoom\":" + jsonInt(snap.currentRoom);
	json += ",\"roomName\":" + jsonString(snap.roomName);
	json += ",\"roomWidth\":" + jsonInt(snap.roomWidth);
	json += ",\"roomHeight\":" + jsonInt(snap.roomHeight);
	json += ",\"screenWidth\":" + jsonInt(snap.screenWidth);
	json += ",\"screenHeight\":" + jsonInt(snap.screenHeight);
	json += ",\"cameraX\":" + jsonInt(snap.cameraX);
	json += ",\"cameraY\":" + jsonInt(snap.cameraY);
	json += "}";
	return json;
}

std::string dialogToJson(const ScummApiSnapshot &snap) {
	std::string json = "{";
	json += "\"isActive\":" + jsonBool(snap.dialog.isActive);
	json += ",\"speakerId\":" + jsonInt(snap.dialog.speakerId);
	json += ",\"speakerName\":" + jsonString(snap.dialog.speakerName);
	json += ",\"currentLine\":" + jsonString(snap.dialog.currentLine);

	json += ",\"choices\":[";
	for (size_t i = 0; i < snap.dialog.choices.size(); ++i) {
		if (i > 0)
			json += ",";
		json += "{\"index\":" + jsonInt(snap.dialog.choices[i].index);
		json += ",\"text\":" + jsonString(snap.dialog.choices[i].text);
		json += "}";
	}
	json += "]";

	json += "}";
	return json;
}

// ---------------------------------------------------------------------------
// Full snapshot → JSON
// ---------------------------------------------------------------------------

std::string snapshotToJson(const ScummApiSnapshot &snap) {
	std::string json = "{";

	json += "\"isEngineReady\":" + jsonBool(snap.isEngineReady);
	json += ",\"gameState\":" + jsonString(gameStateToString(snap.gameState));
	json += ",\"haveMessage\":" + jsonBool(snap.haveMessage);
	json += ",\"talkingActorId\":" + jsonInt(snap.talkingActorId);

	// Room
	json += ",\"room\":" + roomToJson(snap);

	// Actors
	json += ",\"actors\":" + actorsToJson(snap);

	// Inventory
	json += ",\"inventory\":" + inventoryToJson(snap);

	// Verbs
	json += ",\"verbs\":" + verbsToJson(snap);

	// Hotspots
	json += ",\"hotspots\":" + hotspotsToJson(snap);

	// Boxes
	json += ",\"boxes\":" + boxesToJson(snap);

	// Dialog
	json += ",\"dialog\":" + dialogToJson(snap);

	json += "}";
	return json;
}

} // End of namespace ScummApi

#endif // USE_SCUMM_API
