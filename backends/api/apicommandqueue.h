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

#ifndef BACKENDS_API_APICOMMANDQUEUE_H
#define BACKENDS_API_APICOMMANDQUEUE_H

#ifdef USE_SCUMM_API

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace ScummApi {

/**
 * Command types posted by the HTTP thread for the game loop to execute.
 */
enum class ApiCommandType {
	VERB_SINGLE,       // Execute a single-object verb (e.g. "Look at X")
	VERB_MULTI,        // Execute a two-object verb (e.g. "Use X on Y")
	WALK_TO,           // Walk the player actor to screen coordinates
	CLICK,             // Inject a synthetic mouse click
	DIALOG_CHOOSE,     // Select a dialog option by index
	SAVE_GAME,         // Save the game to a slot
	LOAD_GAME,         // Load the game from a slot
	SCREENSHOT_REQUEST // Request a screenshot (fulfilled by game thread)
};

/**
 * ApiCommand — a command posted by the HTTP thread.
 *
 * Uses a flat struct with all possible fields rather than a union/variant
 * for simplicity. Only the fields relevant to the command type are read.
 */
struct ApiCommand {
	ApiCommandType type;

	// VERB_SINGLE / VERB_MULTI
	int verbId = 0;
	int objectId = 0;
	int targetId = 0;   // Only for VERB_MULTI

	// WALK_TO / CLICK
	int x = 0;
	int y = 0;

	// CLICK
	bool isRightButton = false;

	// DIALOG_CHOOSE
	int choiceIndex = 0;

	// SAVE_GAME / LOAD_GAME
	int saveSlot = 0;
	std::string saveDescription;

	// SCREENSHOT_REQUEST — response written here by game thread
	std::vector<unsigned char> screenshotPngData;
	bool screenshotReady = false;
	bool screenshotFailed = false;
};

/**
 * ApiCommandQueue — thread-safe queue for commands from HTTP to game thread.
 *
 * HTTP thread pushes commands; game loop drains them each tick.
 * For screenshot requests, the game thread fulfills them in-place and
 * signals the waiting HTTP thread via a condition variable.
 */
class ApiCommandQueue {
public:
	/**
	 * Push a command onto the queue.
	 * Called from the HTTP thread.
	 */
	void push(const ApiCommand &cmd);

	/**
	 * Drain all pending commands into the output vector and clear the queue.
	 * Called from the game thread at the top of each tick.
	 */
	void drain(std::vector<ApiCommand> &out);

	/**
	 * Push a screenshot request and wait for the game thread to fulfill it.
	 * Returns the PNG data, or empty vector on timeout/failure.
	 * Called from the HTTP thread (blocks until fulfilled or timeout).
	 */
	std::vector<unsigned char> requestScreenshot(int timeoutMs = 2000);

	/**
	 * Drain only screenshot requests (separate from normal commands).
	 * Game thread calls this to find and fulfill screenshot requests.
	 */
	void drainScreenshotRequests(std::vector<ApiCommand *> &out);

	/**
	 * Signal that a screenshot request has been fulfilled.
	 * Called by game thread after writing PNG data into the command.
	 */
	void notifyScreenshotReady();

private:
	std::vector<ApiCommand> _commands;
	std::vector<ApiCommand> _screenshotRequests;
	mutable std::mutex _mutex;
	std::condition_variable _screenshotCv;
};

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // BACKENDS_API_APICOMMANDQUEUE_H
