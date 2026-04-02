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

// Disable symbol overrides so that std::mutex/std::condition_variable can use standard C headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#ifndef USE_SCUMM_API
#define USE_SCUMM_API
#endif

#ifdef USE_SCUMM_API

#include "backends/api/apicommandqueue.h"
#include <chrono>

namespace ScummApi {

void ApiCommandQueue::push(const ApiCommand &cmd) {
	std::lock_guard<std::mutex> lock(_mutex);
	_commands.push_back(cmd);
}

void ApiCommandQueue::drain(std::vector<ApiCommand> &out) {
	std::lock_guard<std::mutex> lock(_mutex);
	out.swap(_commands);
	_commands.clear();
}

std::vector<unsigned char> ApiCommandQueue::requestScreenshot(int timeoutMs) {
	ApiCommand cmd;
	cmd.type = ApiCommandType::SCREENSHOT_REQUEST;
	cmd.screenshotReady = false;
	cmd.screenshotFailed = false;

	{
		std::lock_guard<std::mutex> lock(_mutex);
		_screenshotRequests.push_back(std::move(cmd));
	}

	// Wait for the game thread to fulfill the request
	std::unique_lock<std::mutex> lock(_mutex);
	bool fulfilled = _screenshotCv.wait_for(lock,
		std::chrono::milliseconds(timeoutMs),
		[this]() {
			if (_screenshotRequests.empty())
				return true;
			return _screenshotRequests.back().screenshotReady ||
			       _screenshotRequests.back().screenshotFailed;
		});

	if (!fulfilled || _screenshotRequests.empty()) {
		return {};
	}

	auto result = std::move(_screenshotRequests.back().screenshotPngData);
	_screenshotRequests.pop_back();
	return result;
}

void ApiCommandQueue::drainScreenshotRequests(std::vector<ApiCommand *> &out) {
	std::lock_guard<std::mutex> lock(_mutex);
	for (auto &req : _screenshotRequests) {
		if (!req.screenshotReady && !req.screenshotFailed) {
			out.push_back(&req);
		}
	}
}

void ApiCommandQueue::notifyScreenshotReady() {
	_screenshotCv.notify_all();
}

} // End of namespace ScummApi

#endif // USE_SCUMM_API
