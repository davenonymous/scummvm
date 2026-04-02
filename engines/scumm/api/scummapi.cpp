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

// Disable symbol overrides so that cpp-httplib and std headers work correctly
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#ifndef USE_SCUMM_API
#define USE_SCUMM_API
#endif

#ifdef USE_SCUMM_API

// On Windows/MinGW, include <windows.h> before httplib so that Win32 API
// symbols (MultiByteToWideChar, CP_UTF8, etc.) are declared before winsock2.h
// pulls in the partial Windows headers that httplib.h depends on.
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// Disable httplib exception handling — ScummVM is built without exceptions.
#define CPPHTTPLIB_NO_EXCEPTIONS

#include "scumm/api/scummapi.h"
#include "scumm/api/testpage.h"
#include "scumm/api/openapi_spec.h"
#include "scumm/api/eventinstrumentation.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/verbs.h"
#include "scumm/object.h"
#include "scumm/boxes.h"
#include "backends/api/httplib.h"
#include "backends/api/httpserver.h"
#include "backends/api/apieventqueue.h"
#include "backends/api/apicommandqueue.h"
#include "backends/api/apistate.h"
#include "graphics/thumbnail.h"
#include "image/png.h"
#include "common/memstream.h"

#include <string>
#include <vector>
#include <thread>
#include <chrono>

namespace ScummApi {

// Forward-declared statebuilder functions (implemented in scummapi_statebuilder.cpp)
ScummApiSnapshot buildSnapshot(Scumm::ScummEngine *vm);
std::string snapshotToJson(const ScummApiSnapshot &snap);
std::string actorsToJson(const ScummApiSnapshot &snap);
std::string inventoryToJson(const ScummApiSnapshot &snap);
std::string verbsToJson(const ScummApiSnapshot &snap);
std::string hotspotsToJson(const ScummApiSnapshot &snap);
std::string boxesToJson(const ScummApiSnapshot &snap);
std::string roomToJson(const ScummApiSnapshot &snap);
std::string dialogToJson(const ScummApiSnapshot &snap);

// ---------------------------------------------------------------------------
// JSON helpers — hand-built string concatenation, no JSON library
// ---------------------------------------------------------------------------

static std::string jsonEscape(const std::string &input) {
	std::string result;
	result.reserve(input.size() + 16);
	for (char ch : input) {
		switch (ch) {
		case '"':  result += "\\\""; break;
		case '\\': result += "\\\\"; break;
		case '\b': result += "\\b";  break;
		case '\f': result += "\\f";  break;
		case '\n': result += "\\n";  break;
		case '\r': result += "\\r";  break;
		case '\t': result += "\\t";  break;
		default:
			if (static_cast<unsigned char>(ch) < 0x20) {
				// Control characters: encode as \u00XX
				char buf[8];
				snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(ch));
				result += buf;
			} else {
				result += ch;
			}
			break;
		}
	}
	return result;
}

static std::string jsonOk(const std::string &message = "ok") {
	return "{\"status\":\"" + jsonEscape(message) + "\"}";
}

static std::string jsonError(const std::string &message) {
	return "{\"error\":\"" + jsonEscape(message) + "\"}";
}

static bool parseIntField(const std::string &body, const std::string &key, int &outValue) {
	std::string searchKey = "\"" + key + "\"";
	size_t keyPos = body.find(searchKey);
	if (keyPos == std::string::npos)
		return false;

	size_t colonPos = body.find(':', keyPos + searchKey.size());
	if (colonPos == std::string::npos)
		return false;

	// Skip whitespace after colon
	size_t valueStart = colonPos + 1;
	while (valueStart < body.size() && (body[valueStart] == ' ' || body[valueStart] == '\t'))
		valueStart++;

	if (valueStart >= body.size())
		return false;

	// Parse integer (may start with minus sign)
	std::string numStr;
	if (body[valueStart] == '-') {
		numStr += '-';
		valueStart++;
	}
	while (valueStart < body.size() && body[valueStart] >= '0' && body[valueStart] <= '9') {
		numStr += body[valueStart];
		valueStart++;
	}

	if (numStr.empty() || numStr == "-")
		return false;

	outValue = std::stoi(numStr);
	return true;
}

static bool parseBoolField(const std::string &body, const std::string &key, bool &outValue) {
	std::string searchKey = "\"" + key + "\"";
	size_t keyPos = body.find(searchKey);
	if (keyPos == std::string::npos)
		return false;

	size_t colonPos = body.find(':', keyPos + searchKey.size());
	if (colonPos == std::string::npos)
		return false;

	size_t valueStart = colonPos + 1;
	while (valueStart < body.size() && (body[valueStart] == ' ' || body[valueStart] == '\t'))
		valueStart++;

	if (body.compare(valueStart, 4, "true") == 0) {
		outValue = true;
		return true;
	}
	if (body.compare(valueStart, 5, "false") == 0) {
		outValue = false;
		return true;
	}
	return false;
}

static bool parseStringField(const std::string &body, const std::string &key, std::string &outValue) {
	std::string searchKey = "\"" + key + "\"";
	size_t keyPos = body.find(searchKey);
	if (keyPos == std::string::npos)
		return false;

	size_t colonPos = body.find(':', keyPos + searchKey.size());
	if (colonPos == std::string::npos)
		return false;

	size_t quoteStart = body.find('"', colonPos + 1);
	if (quoteStart == std::string::npos)
		return false;

	// Find closing quote (handle escaped quotes)
	size_t quoteEnd = quoteStart + 1;
	while (quoteEnd < body.size()) {
		if (body[quoteEnd] == '\\') {
			quoteEnd += 2;  // Skip escaped character
			continue;
		}
		if (body[quoteEnd] == '"')
			break;
		quoteEnd++;
	}

	if (quoteEnd >= body.size())
		return false;

	outValue = body.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
	return true;
}

// ---------------------------------------------------------------------------
// ScummApiController::Impl — Pimpl holding all std:: and httplib types
// ---------------------------------------------------------------------------

struct ScummApiController::Impl {
	int port;
	HttpServer httpServer;
	ApiEventQueue eventQueue;
	ApiCommandQueue commandQueue;

	mutable std::mutex snapshotMutex;
	ScummApiSnapshot snapshot;

	explicit Impl(int p) : port(p) {}

	void processCommands(Scumm::ScummEngine *vm);
	void fulfillScreenshotRequests();
	void updateSnapshot(Scumm::ScummEngine *vm);
	void registerEndpoints();
};

// ---------------------------------------------------------------------------
// ScummApiController public methods — delegate to Impl
// ---------------------------------------------------------------------------

ScummApiController::ScummApiController(int port)
	: _impl(new Impl(port)) {
}

ScummApiController::~ScummApiController() {
	stop();
	delete _impl;
}

void ScummApiController::start() {
	setEventQueue(&_impl->eventQueue);
	_impl->registerEndpoints();

	debug("SCUMM API: Starting HTTP server on port %d...", _impl->port);

	if (_impl->httpServer.start(_impl->port)) {
		debug("SCUMM API: Server is listening at http://localhost:%d", _impl->port);
		debug("SCUMM API: Test page available at http://localhost:%d/", _impl->port);
		debug("SCUMM API: OpenAPI spec at http://localhost:%d/openapi.json", _impl->port);
		debug("SCUMM API: SSE event stream at http://localhost:%d/events", _impl->port);
	} else {
		debug("SCUMM API: ERROR - Failed to start HTTP server on port %d", _impl->port);
	}
}

void ScummApiController::stop() {
	if (!_impl)
		return;
	debug("SCUMM API: Stopping HTTP server...");
	_impl->httpServer.stop();
	setEventQueue(nullptr);
	debug("SCUMM API: Server stopped.");
}

void ScummApiController::tick(Scumm::ScummEngine *vm) {
	if (!vm)
		return;

	_impl->processCommands(vm);
	_impl->fulfillScreenshotRequests();
	_impl->updateSnapshot(vm);
}

// ---------------------------------------------------------------------------
// Command processing
// ---------------------------------------------------------------------------

void ScummApiController::Impl::processCommands(Scumm::ScummEngine *vm) {
	std::vector<ApiCommand> commands;
	commandQueue.drain(commands);

	for (const ApiCommand &cmd : commands) {
		switch (cmd.type) {

		case ApiCommandType::VERB_SINGLE:
			if (vm->_sentenceNum < Scumm::NUM_SENTENCE) {
				Scumm::SentenceTab &st = vm->_sentence[vm->_sentenceNum++];
				st.verb = cmd.verbId;
				st.objectA = cmd.objectId;
				st.objectB = 0;
				st.preposition = 0;
				st.freezeCount = 0;
			}
			break;

		case ApiCommandType::VERB_MULTI:
			if (vm->_sentenceNum < Scumm::NUM_SENTENCE) {
				Scumm::SentenceTab &st = vm->_sentence[vm->_sentenceNum++];
				st.verb = cmd.verbId;
				st.objectA = cmd.objectId;
				st.objectB = cmd.targetId;
				st.preposition = (cmd.targetId != 0) ? 1 : 0;
				st.freezeCount = 0;
			}
			break;

		case ApiCommandType::WALK_TO:
			vm->_mouse = Common::Point(cmd.x, cmd.y);
			vm->_leftBtnPressed = 1;
			break;

		case ApiCommandType::CLICK:
			vm->_mouse = Common::Point(cmd.x, cmd.y);
			if (cmd.isRightButton) {
				vm->_rightBtnPressed = 1;
			} else {
				vm->_leftBtnPressed = 1;
			}
			break;

		case ApiCommandType::DIALOG_CHOOSE: {
			// Find the n-th dialog choice verb slot and trigger it
			// directly via the input script system.
			// Dialog choices are verb slots whose raw resource data
			// starts with non-ASCII bytes (SCUMM control codes).
			int choiceCount = 0;
			bool found = false;
			for (int i = 1; i < vm->apiGetNumVerbs(); ++i) {
				const Scumm::VerbSlot &vs = vm->_verbs[i];
				if (vs.verbid == 0 || vs.curmode != 1)
					continue;
				if (vs.type == Scumm::kImageVerbType)
					continue;
				const byte *data = vm->getResourceAddress(Scumm::rtVerb, i);
				if (!data || (data[0] >= 0x20 && data[0] < 0x7F))
					continue; // Plain-text label = regular verb, not a choice

				if (choiceCount == cmd.choiceIndex) {
					vm->apiRunInputScript(Scumm::kVerbClickArea, vs.verbid, 1);
					found = true;
					break;
				}
				++choiceCount;
			}
			if (!found) {
				debug("SCUMM API: DIALOG_CHOOSE index %d out of range (%d choices available)",
				      cmd.choiceIndex, choiceCount);
			}
			break;
		}

		case ApiCommandType::SAVE_GAME:
			vm->requestSave(cmd.saveSlot, Common::String(cmd.saveDescription.c_str()));
			break;

		case ApiCommandType::LOAD_GAME:
			vm->requestLoad(cmd.saveSlot);
			break;

		case ApiCommandType::SCREENSHOT_REQUEST:
			break;
		}
	}
}

// ---------------------------------------------------------------------------
// Screenshot fulfillment
// ---------------------------------------------------------------------------

void ScummApiController::Impl::fulfillScreenshotRequests() {
	std::vector<ApiCommand *> requests;
	commandQueue.drainScreenshotRequests(requests);

	if (requests.empty())
		return;

	Graphics::Surface screenSurface;
	bool captured = Graphics::createScreenShot(screenSurface);

	for (ApiCommand *req : requests) {
		if (!captured) {
			req->screenshotFailed = true;
			req->screenshotReady = true;
			continue;
		}

		Common::MemoryWriteStreamDynamic pngStream(DisposeAfterUse::YES);
		bool written = Image::writePNG(pngStream, screenSurface);

		if (written && pngStream.size() > 0) {
			const byte *data = pngStream.getData();
			req->screenshotPngData.assign(data, data + pngStream.size());
		} else {
			req->screenshotFailed = true;
		}
		req->screenshotReady = true;
	}

	if (captured) {
		screenSurface.free();
	}

	commandQueue.notifyScreenshotReady();
}

// ---------------------------------------------------------------------------
// Snapshot update
// ---------------------------------------------------------------------------

void ScummApiController::Impl::updateSnapshot(Scumm::ScummEngine *vm) {
	ScummApiSnapshot newSnapshot = buildSnapshot(vm);

	// Fire dialog_choices SSE event when choices first appear.
	// Use the snapshot data (already validated) rather than re-scanning verbs.
	{
		std::lock_guard<std::mutex> lock(snapshotMutex);
		bool hadChoices = !snapshot.dialog.choices.empty();
		bool hasChoices = !newSnapshot.dialog.choices.empty();

		if (hasChoices && !hadChoices) {
			std::string payload = "{\"choices\": [";
			for (size_t i = 0; i < newSnapshot.dialog.choices.size(); ++i) {
				if (i > 0)
					payload += ", ";
				payload += "{\"index\": " + std::to_string(newSnapshot.dialog.choices[i].index)
					+ ", \"text\": \"" + jsonEscape(newSnapshot.dialog.choices[i].text) + "\"}";
			}
			payload += "]}";
			eventQueue.push({ApiEventType::DIALOG_CHOICES, payload});
		}

		snapshot = std::move(newSnapshot);
	}
}

// ---------------------------------------------------------------------------
// REST endpoint registration
// ---------------------------------------------------------------------------

void ScummApiController::Impl::registerEndpoints() {
	httplib::Server &srv = httpServer.server();

	// -- Test page --
	srv.Get("/", [](const httplib::Request & /*req*/, httplib::Response &res) {
		res.set_content(SCUMM_API_TEST_PAGE, "text/html");
	});

	// -- OpenAPI spec --
	srv.Get("/openapi.json", [](const httplib::Request & /*req*/, httplib::Response &res) {
		res.set_content(SCUMM_API_OPENAPI_SPEC, "application/json");
	});

	// -- Full state snapshot --
	srv.Get("/api/state", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(snapshotToJson(snapshot), "application/json");
	});

	// -- Actors --
	srv.Get("/api/actors", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(actorsToJson(snapshot), "application/json");
	});

	// -- Inventory --
	srv.Get("/api/inventory", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(inventoryToJson(snapshot), "application/json");
	});

	// -- Verbs --
	srv.Get("/api/verbs", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(verbsToJson(snapshot), "application/json");
	});

	// -- Hotspots (interactive objects) --
	srv.Get("/api/hotspots", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(hotspotsToJson(snapshot), "application/json");
	});

	// -- Walk boxes --
	srv.Get("/api/boxes", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(boxesToJson(snapshot), "application/json");
	});

	// -- Room info --
	srv.Get("/api/room", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(roomToJson(snapshot), "application/json");
	});

	// -- Dialog state --
	srv.Get("/api/dialog", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::lock_guard<std::mutex> lock(snapshotMutex);
		res.set_content(dialogToJson(snapshot), "application/json");
	});

	// -- Screenshot (blocking, returns PNG) --
	srv.Get("/api/screenshot", [this](const httplib::Request & /*req*/, httplib::Response &res) {
		std::vector<unsigned char> pngData = commandQueue.requestScreenshot();
		if (pngData.empty()) {
			res.status = 503;
			res.set_content(jsonError("Screenshot capture failed or timed out"), "application/json");
			return;
		}
		res.set_content(
			reinterpret_cast<const char *>(pngData.data()),
			pngData.size(),
			"image/png"
		);
	});

	// -- POST: Execute verb (single object) --
	srv.Post("/api/verb", [this](const httplib::Request &req, httplib::Response &res) {
		int verbId = 0, objectId = 0;
		if (!parseIntField(req.body, "verbId", verbId) ||
		    !parseIntField(req.body, "objectId", objectId)) {
			res.status = 400;
			res.set_content(jsonError("Missing required fields: verbId, objectId"), "application/json");
			return;
		}

		ApiCommand cmd;
		cmd.type = ApiCommandType::VERB_SINGLE;
		cmd.verbId = verbId;
		cmd.objectId = objectId;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- POST: Execute verb (two objects) --
	srv.Post("/api/verb_multi", [this](const httplib::Request &req, httplib::Response &res) {
		int verbId = 0, objectId = 0, targetId = 0;
		if (!parseIntField(req.body, "verbId", verbId) ||
		    !parseIntField(req.body, "objectId", objectId) ||
		    !parseIntField(req.body, "targetId", targetId)) {
			res.status = 400;
			res.set_content(jsonError("Missing required fields: verbId, objectId, targetId"), "application/json");
			return;
		}

		ApiCommand cmd;
		cmd.type = ApiCommandType::VERB_MULTI;
		cmd.verbId = verbId;
		cmd.objectId = objectId;
		cmd.targetId = targetId;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- POST: Walk to coordinates --
	srv.Post("/api/walk", [this](const httplib::Request &req, httplib::Response &res) {
		int x = 0, y = 0;
		if (!parseIntField(req.body, "x", x) ||
		    !parseIntField(req.body, "y", y)) {
			res.status = 400;
			res.set_content(jsonError("Missing required fields: x, y"), "application/json");
			return;
		}

		ApiCommand cmd;
		cmd.type = ApiCommandType::WALK_TO;
		cmd.x = x;
		cmd.y = y;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- POST: Synthetic mouse click --
	srv.Post("/api/click", [this](const httplib::Request &req, httplib::Response &res) {
		int x = 0, y = 0;
		if (!parseIntField(req.body, "x", x) ||
		    !parseIntField(req.body, "y", y)) {
			res.status = 400;
			res.set_content(jsonError("Missing required fields: x, y"), "application/json");
			return;
		}

		bool isRightButton = false;
		parseBoolField(req.body, "rightButton", isRightButton);

		ApiCommand cmd;
		cmd.type = ApiCommandType::CLICK;
		cmd.x = x;
		cmd.y = y;
		cmd.isRightButton = isRightButton;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- POST: Select dialog choice --
	srv.Post("/api/dialog", [this](const httplib::Request &req, httplib::Response &res) {
		int choiceIndex = 0;
		if (!parseIntField(req.body, "choiceIndex", choiceIndex)) {
			res.status = 400;
			res.set_content(jsonError("Missing required field: choiceIndex"), "application/json");
			return;
		}

		ApiCommand cmd;
		cmd.type = ApiCommandType::DIALOG_CHOOSE;
		cmd.choiceIndex = choiceIndex;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- POST: Save game --
	srv.Post("/api/save", [this](const httplib::Request &req, httplib::Response &res) {
		int slot = 0;
		std::string description;
		if (!parseIntField(req.body, "slot", slot)) {
			res.status = 400;
			res.set_content(jsonError("Missing required field: slot"), "application/json");
			return;
		}
		if (!parseStringField(req.body, "description", description)) {
			description = "API Save";
		}

		ApiCommand cmd;
		cmd.type = ApiCommandType::SAVE_GAME;
		cmd.saveSlot = slot;
		cmd.saveDescription = description;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- POST: Load game --
	srv.Post("/api/load", [this](const httplib::Request &req, httplib::Response &res) {
		int slot = 0;
		if (!parseIntField(req.body, "slot", slot)) {
			res.status = 400;
			res.set_content(jsonError("Missing required field: slot"), "application/json");
			return;
		}

		ApiCommand cmd;
		cmd.type = ApiCommandType::LOAD_GAME;
		cmd.saveSlot = slot;
		commandQueue.push(cmd);

		res.set_content(jsonOk(), "application/json");
	});

	// -- SSE: Event stream --
	srv.Get("/events", [this](const httplib::Request &req, httplib::Response &res) {
		res.set_header("Cache-Control", "no-cache");
		res.set_header("Access-Control-Allow-Origin", "*");

		res.set_chunked_content_provider(
			"text/event-stream",
			[this, &req](size_t /*offset*/, httplib::DataSink &sink) -> bool {
				while (!req.is_connection_closed()) {
					std::vector<ApiEvent> events;
					eventQueue.drain(events);

					for (const ApiEvent &event : events) {
						std::string sseMessage =
							"event: " + std::string(eventTypeToString(event.type)) + "\n" +
							"data: " + event.jsonPayload + "\n\n";

						if (!sink.write(sseMessage.c_str(), sseMessage.size()))
							return false;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(50));
				}
				return false;
			}
		);
	});
}

} // End of namespace ScummApi

#endif // USE_SCUMM_API
