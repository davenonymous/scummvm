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

#ifndef SCUMM_API_EVENTINSTRUMENTATION_H
#define SCUMM_API_EVENTINSTRUMENTATION_H

#ifdef USE_SCUMM_API

namespace Scumm {
class ScummEngine;
}

namespace ScummApi {

class ApiEventQueue;

/** Set the global event queue pointer (called during ScummApi initialization). */
void setEventQueue(ApiEventQueue *queue);

/** Emitted when the player enters a new room. */
void onSceneChanged(int roomId, const char *roomName);

/** Emitted when an actor speaks a dialog line. */
void onDialogLine(int speakerId, const char *speakerName, const char *text);

/**
 * Emitted when the player is presented with dialog choices.
 * Scans the engine's verb slots for active choices and fires
 * a dialog_choices SSE event.
 */
void onDialogChoicesFromVerbs(Scumm::ScummEngine *vm);

/** Emitted when a dialog sequence ends. */
void onDialogEnded();

/** Emitted when an actor changes position. */
void onActorMoved(int actorId, const char *name, int x, int y, int room);

/** Emitted when an actor speaks (text + actor info). */
void onActorTalking(int actorId, const char *name, const char *text);

/** Emitted when the player's inventory changes. */
void onInventoryChanged();

/** Emitted when a verb action is executed. */
void onVerbExecuted(int verbId, int objectId, int targetId);

/** Emitted when the game state changes (e.g. cutscene/gameplay/dialog/frozen). */
void onGameStateChanged(const char *state);

/** Emitted when a walk command completes. */
void onWalkComplete(int actorId, int x, int y);

/** Emitted when the player actor stops moving. */
void onPlayerStopped(int actorId, int x, int y);

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // SCUMM_API_EVENTINSTRUMENTATION_H
