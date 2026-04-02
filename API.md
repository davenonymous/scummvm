# SCUMM API Reference

The SCUMM API is an HTTP server built into the engine (requires `USE_SCUMM_API=1`
build). It exposes game state as JSON and accepts commands. All responses include
`Access-Control-Allow-Origin: *`.

**Default base URL:** `http://localhost:9000`  
**OpenAPI spec:** `GET /openapi.json`  
**Interactive test page:** `GET /`

---

## Read endpoints

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/state` | Full snapshot of all state below in one response |
| `GET` | `/api/room` | Current room id, name, dimensions, camera position |
| `GET` | `/api/actors` | Actors visible in the current room |
| `GET` | `/api/inventory` | Player's inventory items |
| `GET` | `/api/verbs` | Active verb slots (e.g. Open, Pick up, Use) |
| `GET` | `/api/hotspots` | Interactive objects in the current room |
| `GET` | `/api/boxes` | Walk boxes (`walkable` and `all` arrays) |
| `GET` | `/api/dialog` | Current dialog state (speaker, line, choices) |
| `GET` | `/api/screenshot` | Current frame as `image/png` (blocking) |

---

## Command endpoints

All command endpoints accept `Content-Type: application/json` and return
`{"ok": true}` on success or `{"error": "..."}` with HTTP 400 on bad input.
Commands are queued and executed on the next game loop tick.

### `POST /api/verb`
Apply a verb to a single object.
```json
{ "verbId": 13, "objectId": 42 }
```

### `POST /api/verb_multi`
Apply a verb to two objects (e.g. "Use key with door").
```json
{ "verbId": 13, "objectId": 42, "targetId": 7 }
```

### `POST /api/walk`
Walk the player actor to room coordinates.
```json
{ "x": 160, "y": 120 }
```

### `POST /api/click`
Synthesise a mouse click at screen coordinates.
```json
{ "x": 160, "y": 120, "rightButton": false }
```
`rightButton` is optional and defaults to `false`.

### `POST /api/dialog`
Select a dialog choice by its zero-based index.
```json
{ "choiceIndex": 0 }
```

### `POST /api/save`
Save the game to a slot.
```json
{ "slot": 1, "description": "My save" }
```
`description` is optional and defaults to `"API Save"`.

### `POST /api/load`
Load the game from a slot.
```json
{ "slot": 1 }
```

---

## SSE event stream

`GET /events` — Server-Sent Events stream. Each event has a named type and a
JSON `data` payload. Connect and keep the connection open; events are pushed as
they occur.

```
event: scene_changed
data: {"room_id": 5, "room_name": "Kitchen"}

event: actor_moved
data: {"actor_id": 1, "name": "Guybrush", "x": 80, "y": 140, "room": 5}
```

| Event | Fired when |
|-------|-----------|
| `scene_changed` | Player enters a new room |
| `dialog_line` | An actor speaks a line |
| `dialog_choices` | Dialog choice menu appears |
| `dialog_ended` | Dialog sequence finishes |
| `actor_moved` | An actor changes position |
| `actor_talking` | An actor begins speaking |
| `inventory_changed` | Inventory is modified |
| `verb_executed` | A verb action is queued |
| `game_state_changed` | State changes between `gameplay`, `cutscene`, `dialog`, `frozen` |
| `walk_complete` | A walk command finishes |
| `player_stopped` | Player actor stops moving |
