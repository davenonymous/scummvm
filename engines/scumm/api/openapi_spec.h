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

#ifndef SCUMM_API_OPENAPI_SPEC_H
#define SCUMM_API_OPENAPI_SPEC_H

#ifdef USE_SCUMM_API

const char *SCUMM_API_OPENAPI_SPEC = R"JSONSPEC(
{
  "openapi": "3.0.0",
  "info": {
    "title": "ScummVM SCUMM Engine API",
    "version": "1.0.0",
    "description": "REST API for controlling SCUMM engine games (Monkey Island, Indiana Jones, etc.) via external clients. Supports real-time event streaming via SSE.",
    "license": {
      "name": "GPL-3.0-or-later",
      "url": "https://www.gnu.org/licenses/gpl-3.0.html"
    }
  },
  "servers": [
    {
      "url": "http://localhost:8080",
      "description": "Local ScummVM instance"
    }
  ],
  "paths": {
    "/api/state": {
      "get": {
        "operationId": "getState",
        "summary": "Full game state snapshot",
        "description": "Returns a complete snapshot of the current game state including room, actors, inventory, verbs, hotspots, boxes, and dialog.",
        "tags": ["State"],
        "responses": {
          "200": {
            "description": "Full game state as JSON.",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object",
                  "properties": {
                    "room": { "$ref": "#/components/schemas/Room" },
                    "actors": {
                      "type": "array",
                      "items": { "$ref": "#/components/schemas/Actor" }
                    },
                    "inventory": {
                      "type": "array",
                      "items": { "$ref": "#/components/schemas/InventoryItem" }
                    },
                    "verbs": {
                      "type": "array",
                      "items": { "$ref": "#/components/schemas/Verb" }
                    },
                    "hotspots": {
                      "type": "array",
                      "items": { "$ref": "#/components/schemas/Hotspot" }
                    },
                    "boxes": { "$ref": "#/components/schemas/Boxes" },
                    "dialog": { "$ref": "#/components/schemas/Dialog" }
                  }
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/room": {
      "get": {
        "operationId": "getRoom",
        "summary": "Current room info",
        "description": "Returns information about the current room including dimensions and camera position.",
        "tags": ["Room"],
        "responses": {
          "200": {
            "description": "Current room information.",
            "content": {
              "application/json": {
                "schema": { "$ref": "#/components/schemas/Room" }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/actors": {
      "get": {
        "operationId": "getActors",
        "summary": "List all actors",
        "description": "Returns an array of all actors in the current room with their positions and state.",
        "tags": ["Actors"],
        "responses": {
          "200": {
            "description": "Array of actors.",
            "content": {
              "application/json": {
                "schema": {
                  "type": "array",
                  "items": { "$ref": "#/components/schemas/Actor" }
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/inventory": {
      "get": {
        "operationId": "getInventory",
        "summary": "List inventory items",
        "description": "Returns an array of items currently in the player's inventory.",
        "tags": ["Inventory"],
        "responses": {
          "200": {
            "description": "Array of inventory items.",
            "content": {
              "application/json": {
                "schema": {
                  "type": "array",
                  "items": { "$ref": "#/components/schemas/InventoryItem" }
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/verbs": {
      "get": {
        "operationId": "getVerbs",
        "summary": "List available verbs",
        "description": "Returns an array of verbs available in the current game UI.",
        "tags": ["Verbs"],
        "responses": {
          "200": {
            "description": "Array of verbs.",
            "content": {
              "application/json": {
                "schema": {
                  "type": "array",
                  "items": { "$ref": "#/components/schemas/Verb" }
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/hotspots": {
      "get": {
        "operationId": "getHotspots",
        "summary": "List interactive hotspots",
        "description": "Returns an array of interactive hotspot regions in the current room.",
        "tags": ["Hotspots"],
        "responses": {
          "200": {
            "description": "Array of hotspots.",
            "content": {
              "application/json": {
                "schema": {
                  "type": "array",
                  "items": { "$ref": "#/components/schemas/Hotspot" }
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/boxes": {
      "get": {
        "operationId": "getBoxes",
        "summary": "Walk-box data",
        "description": "Returns walkable navigation boxes and all boxes for the current room.",
        "tags": ["Boxes"],
        "responses": {
          "200": {
            "description": "Walk-box data split into walkable and all boxes.",
            "content": {
              "application/json": {
                "schema": { "$ref": "#/components/schemas/Boxes" }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/dialog": {
      "get": {
        "operationId": "getDialog",
        "summary": "Dialog state",
        "description": "Returns the current dialog state including active speaker and available choices.",
        "tags": ["Dialog"],
        "responses": {
          "200": {
            "description": "Current dialog state.",
            "content": {
              "application/json": {
                "schema": { "$ref": "#/components/schemas/Dialog" }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/screenshot": {
      "get": {
        "operationId": "getScreenshot",
        "summary": "Capture screenshot",
        "description": "Returns a PNG screenshot of the current game display.",
        "tags": ["Screenshot"],
        "responses": {
          "200": {
            "description": "PNG image of the current screen.",
            "content": {
              "image/png": {
                "schema": {
                  "type": "string",
                  "format": "binary"
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/events": {
      "get": {
        "operationId": "getEvents",
        "summary": "Server-Sent Events stream",
        "description": "Opens a persistent SSE connection that pushes real-time game events (room changes, dialog updates, actor movements, etc.) to the client.",
        "tags": ["Events"],
        "responses": {
          "200": {
            "description": "SSE event stream.",
            "content": {
              "text/event-stream": {
                "schema": {
                  "type": "string",
                  "description": "Newline-delimited SSE frames. Each frame contains an event type and JSON data payload."
                }
              }
            }
          },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/verb": {
      "post": {
        "operationId": "executeVerb",
        "summary": "Execute a verb action",
        "description": "Queues a verb command for the game loop. Optionally targets a second object for two-object verbs (e.g. 'Use key with door').",
        "tags": ["Commands"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": { "$ref": "#/components/schemas/VerbCommand" }
            }
          }
        },
        "responses": {
          "202": { "$ref": "#/components/responses/CommandAccepted" },
          "400": { "$ref": "#/components/responses/BadRequest" },
          "422": { "$ref": "#/components/responses/Unprocessable" },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/walk": {
      "post": {
        "operationId": "walkTo",
        "summary": "Walk to coordinates",
        "description": "Queues a walk command to move the active actor to the specified screen coordinates.",
        "tags": ["Commands"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": { "$ref": "#/components/schemas/WalkCommand" }
            }
          }
        },
        "responses": {
          "202": { "$ref": "#/components/responses/CommandAccepted" },
          "400": { "$ref": "#/components/responses/BadRequest" },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/click": {
      "post": {
        "operationId": "click",
        "summary": "Simulate mouse click",
        "description": "Queues a mouse click at the specified screen coordinates with the given button.",
        "tags": ["Commands"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": { "$ref": "#/components/schemas/ClickCommand" }
            }
          }
        },
        "responses": {
          "202": { "$ref": "#/components/responses/CommandAccepted" },
          "400": { "$ref": "#/components/responses/BadRequest" },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/dialog/choose": {
      "post": {
        "operationId": "chooseDialogOption",
        "summary": "Choose a dialog option",
        "description": "Selects a dialog choice by its index from the currently active dialog.",
        "tags": ["Dialog"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": { "$ref": "#/components/schemas/DialogChooseCommand" }
            }
          }
        },
        "responses": {
          "202": { "$ref": "#/components/responses/CommandAccepted" },
          "400": { "$ref": "#/components/responses/BadRequest" },
          "422": { "$ref": "#/components/responses/Unprocessable" },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/save": {
      "post": {
        "operationId": "saveGame",
        "summary": "Save the game",
        "description": "Saves the current game state to the specified slot with a description.",
        "tags": ["SaveLoad"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": { "$ref": "#/components/schemas/SaveCommand" }
            }
          }
        },
        "responses": {
          "202": { "$ref": "#/components/responses/CommandAccepted" },
          "400": { "$ref": "#/components/responses/BadRequest" },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    },
    "/api/load": {
      "post": {
        "operationId": "loadGame",
        "summary": "Load a saved game",
        "description": "Loads a previously saved game from the specified slot.",
        "tags": ["SaveLoad"],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": { "$ref": "#/components/schemas/LoadCommand" }
            }
          }
        },
        "responses": {
          "202": { "$ref": "#/components/responses/CommandAccepted" },
          "400": { "$ref": "#/components/responses/BadRequest" },
          "503": { "$ref": "#/components/responses/EngineNotReady" }
        }
      }
    }
  },
  "components": {
    "schemas": {
      "Room": {
        "type": "object",
        "required": ["id", "name", "width", "height", "camera_x", "camera_y", "screen_width", "screen_height"],
        "properties": {
          "id": { "type": "integer", "description": "Room resource number." },
          "name": { "type": "string", "description": "Human-readable room name." },
          "width": { "type": "integer", "description": "Room width in pixels." },
          "height": { "type": "integer", "description": "Room height in pixels." },
          "camera_x": { "type": "integer", "description": "Current camera X position." },
          "camera_y": { "type": "integer", "description": "Current camera Y position." },
          "screen_width": { "type": "integer", "description": "Visible screen width in pixels." },
          "screen_height": { "type": "integer", "description": "Visible screen height in pixels." }
        }
      },
      "Actor": {
        "type": "object",
        "required": ["id", "name", "x", "y", "is_moving", "is_talking", "is_visible", "facing", "room"],
        "properties": {
          "id": { "type": "integer", "description": "Actor slot number." },
          "name": { "type": "string", "description": "Actor display name." },
          "x": { "type": "integer", "description": "X position in room coordinates." },
          "y": { "type": "integer", "description": "Y position in room coordinates." },
          "is_moving": { "type": "boolean", "description": "Whether the actor is currently walking." },
          "is_talking": { "type": "boolean", "description": "Whether the actor is currently speaking." },
          "is_visible": { "type": "boolean", "description": "Whether the actor is visible on screen." },
          "facing": { "type": "integer", "description": "Direction the actor is facing (0-360)." },
          "room": { "type": "integer", "description": "Room the actor is currently in." }
        }
      },
      "InventoryItem": {
        "type": "object",
        "required": ["id", "name"],
        "properties": {
          "id": { "type": "integer", "description": "Object number of the inventory item." },
          "name": { "type": "string", "description": "Display name of the item." }
        }
      },
      "Verb": {
        "type": "object",
        "required": ["id", "label", "key"],
        "properties": {
          "id": { "type": "integer", "description": "Verb ID used in commands." },
          "label": { "type": "string", "description": "Display label (e.g. 'Pick up', 'Use')." },
          "key": { "type": "string", "description": "Keyboard shortcut key for this verb." }
        }
      },
      "Hotspot": {
        "type": "object",
        "required": ["id", "name", "x", "y", "width", "height", "walk_to_x", "walk_to_y"],
        "properties": {
          "id": { "type": "integer", "description": "Object number of the hotspot." },
          "name": { "type": "string", "description": "Display name of the hotspot." },
          "x": { "type": "integer", "description": "Bounding box X origin." },
          "y": { "type": "integer", "description": "Bounding box Y origin." },
          "width": { "type": "integer", "description": "Bounding box width." },
          "height": { "type": "integer", "description": "Bounding box height." },
          "walk_to_x": { "type": "integer", "description": "X coordinate the actor walks to when interacting." },
          "walk_to_y": { "type": "integer", "description": "Y coordinate the actor walks to when interacting." }
        }
      },
      "Box": {
        "type": "object",
        "required": ["id", "coords", "flags", "is_walkable"],
        "properties": {
          "id": { "type": "integer", "description": "Box index." },
          "coords": {
            "type": "object",
            "description": "Four-corner polygon defining the box area.",
            "required": ["x1", "y1", "x2", "y2", "x3", "y3", "x4", "y4"],
            "properties": {
              "x1": { "type": "integer" },
              "y1": { "type": "integer" },
              "x2": { "type": "integer" },
              "y2": { "type": "integer" },
              "x3": { "type": "integer" },
              "y3": { "type": "integer" },
              "x4": { "type": "integer" },
              "y4": { "type": "integer" }
            }
          },
          "flags": { "type": "integer", "description": "Raw box flags bitmask." },
          "is_walkable": { "type": "boolean", "description": "Whether the box is currently walkable." }
        }
      },
      "Boxes": {
        "type": "object",
        "required": ["walkable", "all"],
        "properties": {
          "walkable": {
            "type": "array",
            "description": "Only boxes that are currently walkable.",
            "items": { "$ref": "#/components/schemas/Box" }
          },
          "all": {
            "type": "array",
            "description": "All boxes regardless of walkability.",
            "items": { "$ref": "#/components/schemas/Box" }
          }
        }
      },
      "DialogChoice": {
        "type": "object",
        "required": ["index", "text"],
        "properties": {
          "index": { "type": "integer", "description": "Zero-based index of this choice." },
          "text": { "type": "string", "description": "Display text for this choice." }
        }
      },
      "Dialog": {
        "type": "object",
        "required": ["is_active", "speaker_id", "speaker_name", "current_line", "choices"],
        "properties": {
          "is_active": { "type": "boolean", "description": "Whether a dialog is currently in progress." },
          "speaker_id": { "type": "integer", "description": "Actor ID of the current speaker, or 0 if none." },
          "speaker_name": { "type": "string", "description": "Name of the current speaker." },
          "current_line": { "type": "string", "description": "Text currently being spoken." },
          "choices": {
            "type": "array",
            "description": "Available dialog choices, empty if not in a choice prompt.",
            "items": { "$ref": "#/components/schemas/DialogChoice" }
          }
        }
      },
      "VerbCommand": {
        "type": "object",
        "required": ["verb_id", "object_id"],
        "properties": {
          "verb_id": { "type": "integer", "description": "ID of the verb to execute." },
          "object_id": { "type": "integer", "description": "ID of the primary target object." },
          "target_id": { "type": "integer", "description": "ID of the secondary target for two-object verbs (e.g. 'Use key with door')." }
        }
      },
      "WalkCommand": {
        "type": "object",
        "required": ["x", "y"],
        "properties": {
          "x": { "type": "integer", "description": "Target X coordinate in screen space." },
          "y": { "type": "integer", "description": "Target Y coordinate in screen space." }
        }
      },
      "ClickCommand": {
        "type": "object",
        "required": ["x", "y", "button"],
        "properties": {
          "x": { "type": "integer", "description": "Click X coordinate in screen space." },
          "y": { "type": "integer", "description": "Click Y coordinate in screen space." },
          "button": {
            "type": "string",
            "enum": ["left", "right"],
            "description": "Mouse button to simulate."
          }
        }
      },
      "DialogChooseCommand": {
        "type": "object",
        "required": ["choice"],
        "properties": {
          "choice": { "type": "integer", "description": "Zero-based index of the dialog choice to select." }
        }
      },
      "SaveCommand": {
        "type": "object",
        "required": ["slot", "description"],
        "properties": {
          "slot": { "type": "integer", "description": "Save slot number." },
          "description": { "type": "string", "description": "Human-readable save description." }
        }
      },
      "LoadCommand": {
        "type": "object",
        "required": ["slot"],
        "properties": {
          "slot": { "type": "integer", "description": "Save slot number to load from." }
        }
      },
      "ErrorResponse": {
        "type": "object",
        "required": ["error"],
        "properties": {
          "error": { "type": "string", "description": "Human-readable error message." }
        }
      },
      "AcceptedResponse": {
        "type": "object",
        "required": ["status"],
        "properties": {
          "status": { "type": "string", "example": "accepted", "description": "Indicates the command was queued for the game loop." }
        }
      }
    },
    "responses": {
      "CommandAccepted": {
        "description": "Command accepted and queued for the next game loop iteration.",
        "content": {
          "application/json": {
            "schema": { "$ref": "#/components/schemas/AcceptedResponse" }
          }
        }
      },
      "BadRequest": {
        "description": "Bad request — missing or invalid fields in the request body.",
        "content": {
          "application/json": {
            "schema": { "$ref": "#/components/schemas/ErrorResponse" }
          }
        }
      },
      "Unprocessable": {
        "description": "Unprocessable entity — the referenced object, verb, or choice ID does not exist.",
        "content": {
          "application/json": {
            "schema": { "$ref": "#/components/schemas/ErrorResponse" }
          }
        }
      },
      "EngineNotReady": {
        "description": "Service unavailable — the SCUMM engine is not initialized or no game is running.",
        "content": {
          "application/json": {
            "schema": { "$ref": "#/components/schemas/ErrorResponse" }
          }
        }
      }
    }
  },
  "tags": [
    { "name": "State", "description": "Full game state queries." },
    { "name": "Room", "description": "Current room information." },
    { "name": "Actors", "description": "Actor positions and states." },
    { "name": "Inventory", "description": "Player inventory management." },
    { "name": "Verbs", "description": "Available verb actions." },
    { "name": "Hotspots", "description": "Interactive room hotspots." },
    { "name": "Boxes", "description": "Walk-box navigation data." },
    { "name": "Dialog", "description": "Dialog state and choices." },
    { "name": "Screenshot", "description": "Screen capture." },
    { "name": "Events", "description": "Real-time Server-Sent Events." },
    { "name": "Commands", "description": "Game actions (verb, walk, click)." },
    { "name": "SaveLoad", "description": "Save and load game state." }
  ]
}
)JSONSPEC";

#endif // USE_SCUMM_API

#endif // SCUMM_API_OPENAPI_SPEC_H
