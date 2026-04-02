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

// Disable symbol overrides so that std::mutex can use standard C headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#ifndef USE_SCUMM_API
#define USE_SCUMM_API
#endif

#ifdef USE_SCUMM_API

#include "backends/api/apieventqueue.h"

namespace ScummApi {

ApiEventQueue::ApiEventQueue(int capacity)
	: _capacity(capacity), _head(0), _count(0) {
	_buffer.resize(capacity);
}

void ApiEventQueue::push(const ApiEvent &event) {
	std::lock_guard<std::mutex> lock(_mutex);

	_buffer[_head] = event;
	_head = (_head + 1) % _capacity;

	if (_count < _capacity) {
		_count++;
	}
	// If _count == _capacity, we just overwrote the oldest event (backpressure)
}

int ApiEventQueue::drain(std::vector<ApiEvent> &out) {
	std::lock_guard<std::mutex> lock(_mutex);

	if (_count == 0)
		return 0;

	out.reserve(out.size() + _count);

	// Calculate the start position of valid events in the ring buffer
	int start = (_head - _count + _capacity) % _capacity;

	for (int i = 0; i < _count; i++) {
		int idx = (start + i) % _capacity;
		out.push_back(std::move(_buffer[idx]));
	}

	int drained = _count;
	_count = 0;
	_head = 0;

	return drained;
}

bool ApiEventQueue::hasEvents() const {
	std::lock_guard<std::mutex> lock(_mutex);
	return _count > 0;
}

void ApiEventQueue::clear() {
	std::lock_guard<std::mutex> lock(_mutex);
	_count = 0;
	_head = 0;
}

} // End of namespace ScummApi

#endif // USE_SCUMM_API
