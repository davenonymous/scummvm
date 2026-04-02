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

#ifndef SCUMM_API_STATEBUILDER_H
#define SCUMM_API_STATEBUILDER_H

#ifdef USE_SCUMM_API

#include <string>
#include "backends/api/apistate.h"

namespace Scumm {
class ScummEngine;
} // End of namespace Scumm

namespace ScummApi {

/**
 * Build a complete snapshot of the current SCUMM engine state.
 * The returned struct is a self-contained value type with no engine pointers,
 * safe for reading from another thread.
 */
ScummApiSnapshot buildSnapshot(Scumm::ScummEngine *vm);

/** Serialize the full snapshot to a JSON string. */
std::string snapshotToJson(const ScummApiSnapshot &snap);

/** Serialize only the actors array to a JSON string. */
std::string actorsToJson(const ScummApiSnapshot &snap);

/** Serialize only the inventory array to a JSON string. */
std::string inventoryToJson(const ScummApiSnapshot &snap);

/** Serialize only the verbs array to a JSON string. */
std::string verbsToJson(const ScummApiSnapshot &snap);

/** Serialize only the hotspots array to a JSON string. */
std::string hotspotsToJson(const ScummApiSnapshot &snap);

/** Serialize only the boxes arrays to a JSON string. */
std::string boxesToJson(const ScummApiSnapshot &snap);

/** Serialize only the room metadata to a JSON string. */
std::string roomToJson(const ScummApiSnapshot &snap);

/** Serialize only the dialog state to a JSON string. */
std::string dialogToJson(const ScummApiSnapshot &snap);

/** Escape a string for safe embedding in JSON values. */
std::string jsonEscape(const std::string &s);

} // End of namespace ScummApi

#endif // USE_SCUMM_API
#endif // SCUMM_API_STATEBUILDER_H
