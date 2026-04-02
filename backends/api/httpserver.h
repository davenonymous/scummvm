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

#ifndef BACKENDS_API_HTTPSERVER_H
#define BACKENDS_API_HTTPSERVER_H

#ifdef USE_SCUMM_API

// NOTE: This header must NOT include any standard C++ headers (<thread>,
// <mutex>, <atomic>, etc.) because it gets included by engine code that
// has ScummVM's forbidden.h active. All std:: types are hidden behind
// the Pimpl in httpserver.cpp.

namespace httplib {
class Server;
}

namespace ScummApi {

/**
 * HttpServer — wraps cpp-httplib's Server and runs it on a background thread.
 *
 * All standard library types (std::thread, std::atomic) are hidden in the
 * .cpp file to avoid leaking forbidden symbols into engine headers.
 */
class HttpServer {
public:
	HttpServer();
	~HttpServer();

	// Non-copyable
	HttpServer(const HttpServer &) = delete;
	HttpServer &operator=(const HttpServer &) = delete;

	/** Start listening on the given port. Returns true on success. */
	bool start(int port);

	/** Stop the server and join the background thread. */
	void stop();

	/** Whether the server is currently running. */
	bool isRunning() const;

	/** Access the underlying httplib::Server to register routes. */
	httplib::Server &server();

private:
	struct Impl;
	Impl *_impl;
};

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // BACKENDS_API_HTTPSERVER_H
