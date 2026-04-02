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

// Disable symbol overrides so that cpp-httplib can use standard C/C++ headers
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#ifndef USE_SCUMM_API
#define USE_SCUMM_API
#endif

#ifdef USE_SCUMM_API

// cpp-httplib implementation — only compiled once in this translation unit
#define CPPHTTPLIB_NO_EXCEPTIONS
#include "backends/api/httplib.h"
#include "backends/api/httpserver.h"
#include "common/textconsole.h"

#include <thread>
#include <atomic>

namespace ScummApi {

struct HttpServer::Impl {
	httplib::Server server;
	std::thread thread;
	std::atomic<bool> running;
	int port;

	Impl() : running(false), port(0) {}
};

HttpServer::HttpServer()
	: _impl(new Impl()) {
}

HttpServer::~HttpServer() {
	stop();
	delete _impl;
}

bool HttpServer::start(int port) {
	if (_impl->running.load())
		return false;

	_impl->port = port;

	// Configure CORS headers for all responses
	_impl->server.set_default_headers({
		{"Access-Control-Allow-Origin", "*"},
		{"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
		{"Access-Control-Allow-Headers", "Content-Type"}
	});

	// Handle CORS preflight for all paths
	_impl->server.Options(".*", [](const httplib::Request &, httplib::Response &res) {
		res.status = 204;
	});

	// Log every completed request: method + path + status code
	_impl->server.set_logger([](const httplib::Request &req, const httplib::Response &res) {
		warning("SCUMM API: %s %s -> %d", req.method.c_str(), req.path.c_str(), res.status);
	});

	_impl->running.store(true);

	_impl->thread = std::thread([this]() {
		warning("SCUMM API: HTTP server thread started, binding to 0.0.0.0:%d", _impl->port);
		_impl->server.listen("0.0.0.0", _impl->port);
		_impl->running.store(false);
		warning("SCUMM API: HTTP server thread exited");
	});

	// Wait for the server to actually bind and start accepting connections
	_impl->server.wait_until_ready();

	if (_impl->running.load()) {
		warning("SCUMM API: Server is ready at http://localhost:%d", _impl->port);
	} else {
		warning("SCUMM API: ERROR - Server failed to bind on port %d (port already in use?)", _impl->port);
	}

	return _impl->running.load();
}

void HttpServer::stop() {
	if (!_impl->running.load())
		return;

	_impl->server.stop();

	if (_impl->thread.joinable()) {
		_impl->thread.join();
	}

	_impl->running.store(false);
}

bool HttpServer::isRunning() const {
	return _impl->running.load();
}

httplib::Server &HttpServer::server() {
	return _impl->server;
}

} // End of namespace ScummApi

#endif // USE_SCUMM_API
