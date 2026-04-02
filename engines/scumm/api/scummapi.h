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

#ifndef SCUMM_API_SCUMMAPI_H
#define SCUMM_API_SCUMMAPI_H

#ifdef USE_SCUMM_API

// This header is included from engine files (scumm.cpp) that must NOT
// pull in standard C++ headers (<thread>, <mutex>, etc.) because ScummVM's
// forbidden.h would break them. We use the Pimpl pattern to hide all
// std:: types and backend headers behind an opaque pointer.

namespace Scumm {
class ScummEngine;
}

namespace ScummApi {

/**
 * ScummApiController — main API controller for the SCUMM HTTP REST/SSE interface.
 *
 * Owns the HTTP server, event queue, command queue, and a mutex-protected
 * snapshot of the engine state. The game loop calls tick() each frame to
 * drain commands, fulfill screenshot requests, and publish a fresh snapshot.
 *
 * All implementation details (httplib, std::mutex, etc.) are hidden behind
 * the Pimpl (_impl) to avoid leaking standard C++ headers into engine code.
 */
class ScummApiController {
public:
	explicit ScummApiController(int port);
	~ScummApiController();

	// Non-copyable
	ScummApiController(const ScummApiController &) = delete;
	ScummApiController &operator=(const ScummApiController &) = delete;

	/** Start the HTTP server on the configured port. */
	void start();

	/** Stop the HTTP server and clean up. */
	void stop();

	/** Called from the game loop each frame. */
	void tick(Scumm::ScummEngine *vm);

private:
	struct Impl;
	Impl *_impl;
};

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // SCUMM_API_SCUMMAPI_H
