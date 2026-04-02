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

#ifndef BACKENDS_API_APIEVENTQUEUE_H
#define BACKENDS_API_APIEVENTQUEUE_H

#ifdef USE_SCUMM_API

#include <string>
#include <vector>
#include <mutex>

namespace ScummApi {

/**
 * Typed SSE event types emitted by the SCUMM engine instrumentation.
 */
enum class ApiEventType {
	SCENE_CHANGED,
	DIALOG_LINE,
	DIALOG_CHOICES,
	DIALOG_ENDED,
	ACTOR_MOVED,
	ACTOR_TALKING,
	INVENTORY_CHANGED,
	VERB_EXECUTED,
	GAME_STATE_CHANGED,
	WALK_COMPLETE,
	PLAYER_STOPPED
};

inline const char *eventTypeToString(ApiEventType type) {
	switch (type) {
	case ApiEventType::SCENE_CHANGED:       return "scene_changed";
	case ApiEventType::DIALOG_LINE:         return "dialog_line";
	case ApiEventType::DIALOG_CHOICES:      return "dialog_choices";
	case ApiEventType::DIALOG_ENDED:        return "dialog_ended";
	case ApiEventType::ACTOR_MOVED:         return "actor_moved";
	case ApiEventType::ACTOR_TALKING:       return "actor_talking";
	case ApiEventType::INVENTORY_CHANGED:   return "inventory_changed";
	case ApiEventType::VERB_EXECUTED:       return "verb_executed";
	case ApiEventType::GAME_STATE_CHANGED:  return "game_state_changed";
	case ApiEventType::WALK_COMPLETE:       return "walk_complete";
	case ApiEventType::PLAYER_STOPPED:      return "player_stopped";
	}
	return "unknown";
}

/**
 * A single SSE event with type and JSON payload.
 * Payload is pre-serialized JSON so the HTTP thread can stream it directly.
 */
struct ApiEvent {
	ApiEventType type;
	std::string jsonPayload;  // Pre-serialized JSON object
};

/**
 * ApiEventQueue — thread-safe bounded ring buffer for SSE events.
 *
 * The game thread pushes events; the HTTP SSE handler drains them.
 * When the buffer is full, the oldest events are silently dropped
 * (backpressure: the game must never block waiting for slow clients).
 *
 * Thread safety: all methods are mutex-protected.
 */
class ApiEventQueue {
public:
	static const int DEFAULT_CAPACITY = 512;

	explicit ApiEventQueue(int capacity = DEFAULT_CAPACITY);

	/**
	 * Push an event into the queue. If full, the oldest event is overwritten.
	 * Called from the game thread.
	 */
	void push(const ApiEvent &event);

	/**
	 * Drain all queued events into the output vector and clear the queue.
	 * Called from the HTTP SSE thread.
	 * Returns the number of events drained.
	 */
	int drain(std::vector<ApiEvent> &out);

	/**
	 * Check whether the queue has any events waiting.
	 */
	bool hasEvents() const;

	/**
	 * Clear all events.
	 */
	void clear();

private:
	std::vector<ApiEvent> _buffer;
	int _capacity;
	int _head;    // Next write position
	int _count;   // Current number of valid events
	mutable std::mutex _mutex;
};

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // BACKENDS_API_APIEVENTQUEUE_H
