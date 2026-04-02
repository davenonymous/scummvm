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

#include "scumm/api/eventinstrumentation.h"
#include "scumm/api/statebuilder.h"
#include "scumm/scumm.h"
#include "scumm/verbs.h"
#include "backends/api/apieventqueue.h"

namespace ScummApi {

static ApiEventQueue *g_eventQueue = nullptr;

void setEventQueue(ApiEventQueue *queue) {
	g_eventQueue = queue;
}

void onSceneChanged(int roomId, const char *roomName) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"room_id\": " + std::to_string(roomId)
		+ ", \"room_name\": \"" + jsonEscape(roomName ? roomName : "") + "\"}";

	g_eventQueue->push({ApiEventType::SCENE_CHANGED, payload});
}

/** Clean a SCUMM text string for API output:
 *  - 0x10 word-break markers → space
 *  - 0xFF escape sequences → stripped (newlines become space)
 *  - Sentence-end (.!?) followed by uppercase → insert space
 *  - Non-printable characters → stripped */
static std::string cleanText(const char *raw) {
	if (!raw)
		return "";
	std::string result;
	for (int i = 0; raw[i] != 0; ++i) {
		unsigned char ch = static_cast<unsigned char>(raw[i]);
		if (ch == 0xFF) {
			++i; // skip command byte
			unsigned char cmd = static_cast<unsigned char>(raw[i]);
			if (cmd == 4 || cmd == 5 || cmd == 6 || cmd == 7 ||
			    cmd == 9 || cmd == 10 || cmd == 12 || cmd == 13 || cmd == 14)
				i += 2;
			if (cmd == 1 || cmd == 8)
				result += ' ';
		} else if (ch == 0x10) {
			result += ' ';
		} else if (ch >= 0x20 && ch != 0x7F) {
			// Insert a space when a sentence-ending punctuation is
			// immediately followed by an uppercase letter (the original
			// text had a newline here that convertMessageToString dropped).
			if (!result.empty() && (ch >= 'A' && ch <= 'Z')) {
				char prev = result.back();
				if (prev == '.' || prev == '!' || prev == '?')
					result += ' ';
			}
			result += static_cast<char>(ch);
		}
	}
	return result;
}

void onDialogLine(int speakerId, const char *speakerName, const char *text) {
	if (!g_eventQueue)
		return;

	std::string cleanedText = cleanText(text);
	std::string payload = "{\"speaker_id\": " + std::to_string(speakerId)
		+ ", \"speaker_name\": \"" + jsonEscape(speakerName ? speakerName : "")
		+ "\", \"text\": \"" + jsonEscape(cleanedText) + "\"}";

	g_eventQueue->push({ApiEventType::DIALOG_LINE, payload});
}

/** Decode a SCUMM verb resource into readable text, stripping control codes. */
static std::string decodeVerbText(Scumm::ScummEngine *vm, const byte *data) {
	if (!data || !vm)
		return "";

	byte buf[270];
	vm->apiConvertMessage(data, buf, sizeof(buf));

	std::string result;
	for (int i = 0; buf[i] != 0; ++i) {
		if (buf[i] == 0xFF) {
			++i; // advance to command byte
			byte cmd = buf[i];
			if (cmd == 4 || cmd == 5 || cmd == 6 || cmd == 7 ||
			    cmd == 9 || cmd == 10 || cmd == 12 || cmd == 13 || cmd == 14)
				i += 2;
			// i now points at the last byte of the escape; for-loop ++i moves past it
		} else if (buf[i] == 0x10) {
			result += ' ';
		} else if (buf[i] >= 0x20 && buf[i] != 0x7F) {
			result += static_cast<char>(buf[i]);
		}
	}
	return result;
}

/** Check if a verb slot is a dialog choice (encoded text, not a plain label). */
static bool isChoiceVerb(Scumm::ScummEngine *vm, int slotIndex) {
	const Scumm::VerbSlot &vs = vm->_verbs[slotIndex];
	if (vs.verbid == 0 || vs.curmode != 1)
		return false;
	if (vs.type == Scumm::kImageVerbType)
		return false;
	const byte *data = vm->getResourceAddress(Scumm::rtVerb, slotIndex);
	if (!data)
		return false;
	return (data[0] < 0x20 || data[0] >= 0x7F);
}

void onDialogChoicesFromVerbs(Scumm::ScummEngine *vm) {
	if (!g_eventQueue || !vm)
		return;

	std::string payload = "{\"choices\": [";
	int choiceIndex = 0;
	for (int i = 1; i < vm->apiGetNumVerbs(); ++i) {
		if (!isChoiceVerb(vm, i))
			continue;

		const byte *labelData = vm->getResourceAddress(Scumm::rtVerb, i);
		std::string text = decodeVerbText(vm, labelData);

		if (choiceIndex > 0)
			payload += ", ";
		payload += "{\"index\": " + std::to_string(choiceIndex)
			+ ", \"text\": \"" + jsonEscape(text) + "\"}";
		++choiceIndex;
	}
	payload += "]}";

	if (choiceIndex > 0)
		g_eventQueue->push({ApiEventType::DIALOG_CHOICES, payload});
}

void onDialogEnded() {
	if (!g_eventQueue)
		return;

	g_eventQueue->push({ApiEventType::DIALOG_ENDED, "{}"});
}

void onActorMoved(int actorId, const char *name, int x, int y, int room) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"actor_id\": " + std::to_string(actorId)
		+ ", \"name\": \"" + jsonEscape(name ? name : "")
		+ "\", \"x\": " + std::to_string(x)
		+ ", \"y\": " + std::to_string(y)
		+ ", \"room\": " + std::to_string(room) + "}";

	g_eventQueue->push({ApiEventType::ACTOR_MOVED, payload});
}

void onActorTalking(int actorId, const char *name, const char *text) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"actor_id\": " + std::to_string(actorId)
		+ ", \"name\": \"" + jsonEscape(name ? name : "")
		+ "\", \"text\": \"" + jsonEscape(text ? text : "") + "\"}";

	g_eventQueue->push({ApiEventType::ACTOR_TALKING, payload});
}

void onInventoryChanged() {
	if (!g_eventQueue)
		return;

	g_eventQueue->push({ApiEventType::INVENTORY_CHANGED, "{}"});
}

void onVerbExecuted(int verbId, int objectId, int targetId) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"verb_id\": " + std::to_string(verbId)
		+ ", \"object_id\": " + std::to_string(objectId)
		+ ", \"target_id\": " + std::to_string(targetId) + "}";

	g_eventQueue->push({ApiEventType::VERB_EXECUTED, payload});
}

void onGameStateChanged(const char *state) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"state\": \"" + jsonEscape(state ? state : "") + "\"}";

	g_eventQueue->push({ApiEventType::GAME_STATE_CHANGED, payload});
}

void onWalkComplete(int actorId, int x, int y) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"actor_id\": " + std::to_string(actorId)
		+ ", \"x\": " + std::to_string(x)
		+ ", \"y\": " + std::to_string(y) + "}";

	g_eventQueue->push({ApiEventType::WALK_COMPLETE, payload});
}

void onPlayerStopped(int actorId, int x, int y) {
	if (!g_eventQueue)
		return;

	std::string payload = "{\"actor_id\": " + std::to_string(actorId)
		+ ", \"x\": " + std::to_string(x)
		+ ", \"y\": " + std::to_string(y) + "}";

	g_eventQueue->push({ApiEventType::PLAYER_STOPPED, payload});
}

} // End of namespace ScummApi

#endif // USE_SCUMM_API
