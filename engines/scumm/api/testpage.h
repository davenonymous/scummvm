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

#ifndef SCUMM_API_TESTPAGE_H
#define SCUMM_API_TESTPAGE_H

#ifdef USE_SCUMM_API

const char *SCUMM_API_TEST_PAGE = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ScummVM SCUMM API</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0;}
:root{
  --bg:#1a1a2e;--panel:#16213e;--border:#0f3460;--accent:#e94560;
  --text:#e0e0e0;--text-dim:#8899aa;--text-bright:#ffffff;
  --success:#4caf50;--info:#2196f3;--warn:#ff9800;--purple:#9c27b0;
  --danger:#f44336;--muted:#607d8b;
}
html,body{height:100%;background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;font-size:14px;overflow:hidden;}
body{display:flex;flex-direction:column;height:100vh;}

/* ── Header ───────────────────────────────────────────────── */
.header{display:flex;align-items:center;justify-content:space-between;padding:8px 16px;background:var(--panel);border-bottom:2px solid var(--border);flex-shrink:0;}
.header h1{font-size:18px;font-weight:700;color:var(--text-bright);letter-spacing:0.5px;}
.header h1 span{color:var(--accent);}
.conn-status{display:flex;align-items:center;gap:6px;font-size:12px;font-family:'Consolas','Courier New',monospace;}
.conn-dot{width:10px;height:10px;border-radius:50%;background:var(--danger);transition:background .3s;}
.conn-dot.connected{background:var(--success);}

/* ── Main layout ──────────────────────────────────────────── */
.main{display:flex;flex:1;overflow:hidden;min-height:0;}
.left-col{display:flex;flex-direction:column;flex-shrink:0;border-right:1px solid var(--border);}
.right-col{flex:1;display:flex;flex-direction:column;min-width:0;overflow:hidden;}

/* ── Screenshot panel ─────────────────────────────────────── */
.screenshot-panel{padding:8px;background:var(--panel);border-bottom:1px solid var(--border);}
.screenshot-controls{display:flex;align-items:center;gap:8px;margin-bottom:6px;}
.screenshot-canvas{display:block;background:#000;image-rendering:pixelated;border:1px solid var(--border);}

/* ── Action panel ─────────────────────────────────────────── */
.action-panel{padding:8px;background:var(--panel);border-bottom:1px solid var(--border);overflow-y:auto;max-height:220px;}
.action-panel h3{font-size:13px;color:var(--accent);margin-bottom:6px;text-transform:uppercase;letter-spacing:1px;}
.action-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;}
.action-group{background:rgba(0,0,0,0.2);border:1px solid var(--border);border-radius:4px;padding:8px;}
.action-group h4{font-size:12px;color:var(--text-dim);margin-bottom:6px;text-transform:uppercase;letter-spacing:0.5px;}
.action-row{display:flex;align-items:center;gap:4px;margin-bottom:4px;flex-wrap:wrap;}
.action-row:last-child{margin-bottom:0;}

/* ── Tabs ─────────────────────────────────────────────────── */
.tab-bar{display:flex;background:var(--panel);border-bottom:1px solid var(--border);flex-shrink:0;overflow-x:auto;}
.tab-btn{padding:6px 14px;font-size:12px;cursor:pointer;border:none;background:transparent;color:var(--text-dim);border-bottom:2px solid transparent;transition:all .2s;white-space:nowrap;}
.tab-btn:hover{color:var(--text);}
.tab-btn.active{color:var(--accent);border-bottom-color:var(--accent);}
.tab-content{flex:1;overflow-y:auto;padding:8px;font-family:'Consolas','Courier New',monospace;font-size:12px;}
.tab-page{display:none;}
.tab-page.active{display:block;}

/* ── Event log ────────────────────────────────────────────── */
.event-panel{flex-shrink:0;height:180px;display:flex;flex-direction:column;border-top:2px solid var(--border);background:var(--panel);}
.event-header{display:flex;align-items:center;justify-content:space-between;padding:4px 12px;border-bottom:1px solid var(--border);flex-shrink:0;}
.event-header h3{font-size:13px;color:var(--accent);text-transform:uppercase;letter-spacing:1px;}
.event-list{flex:1;overflow-y:auto;padding:4px 8px;font-family:'Consolas','Courier New',monospace;font-size:11px;}
.event-entry{display:flex;gap:8px;padding:2px 0;border-bottom:1px solid rgba(255,255,255,0.04);}
.event-ts{color:var(--text-dim);flex-shrink:0;min-width:80px;}
.event-badge{display:inline-block;padding:1px 6px;border-radius:3px;font-size:10px;font-weight:600;color:#fff;flex-shrink:0;min-width:120px;text-align:center;}
.event-payload{color:var(--text);word-break:break-all;}

/* ── Shared form elements ─────────────────────────────────── */
input[type="number"],input[type="text"],select{background:rgba(0,0,0,0.3);border:1px solid var(--border);color:var(--text);padding:4px 6px;border-radius:3px;font-size:12px;font-family:inherit;}
input[type="number"]{width:60px;}
input[type="text"]{width:120px;}
select{min-width:80px;}
button{background:var(--accent);color:#fff;border:none;padding:4px 10px;border-radius:3px;cursor:pointer;font-size:12px;font-weight:600;transition:opacity .2s;}
button:hover{opacity:0.85;}
button.secondary{background:var(--border);}
label{font-size:12px;color:var(--text-dim);display:flex;align-items:center;gap:4px;cursor:pointer;}
input[type="checkbox"]{accent-color:var(--accent);}
input[type="radio"]{accent-color:var(--accent);}

/* ── Data tables ──────────────────────────────────────────── */
table{width:100%;border-collapse:collapse;}
th{text-align:left;padding:4px 8px;border-bottom:1px solid var(--border);color:var(--accent);font-size:11px;text-transform:uppercase;letter-spacing:0.5px;position:sticky;top:0;background:var(--panel);}
td{padding:3px 8px;border-bottom:1px solid rgba(255,255,255,0.04);font-size:12px;}
tr:hover td{background:rgba(233,69,96,0.05);}

.refresh-row{display:flex;align-items:center;gap:8px;margin-bottom:8px;}

/* ── Dialog choices ───────────────────────────────────────── */
.dialog-choice-btn{display:block;width:100%;text-align:left;background:rgba(0,0,0,0.3);border:1px solid var(--border);color:var(--text);padding:6px 10px;border-radius:3px;cursor:pointer;margin-bottom:4px;font-size:12px;transition:border-color .2s;}
.dialog-choice-btn:hover{border-color:var(--accent);}

/* ── Toast notifications ──────────────────────────────────── */
.toast-container{position:fixed;top:12px;right:12px;z-index:9999;display:flex;flex-direction:column;gap:6px;}
.toast{background:var(--danger);color:#fff;padding:8px 14px;border-radius:4px;font-size:12px;animation:toastIn .3s ease;max-width:400px;word-break:break-word;}
@keyframes toastIn{from{opacity:0;transform:translateY(-10px);}to{opacity:1;transform:translateY(0);}}
</style>
</head>
<body>

<!-- Toast container -->
<div class="toast-container" id="toastContainer"></div>

<!-- Header -->
<div class="header">
  <h1><span>ScummVM</span> SCUMM API</h1>
  <div class="conn-status">
    <div class="conn-dot" id="connDot"></div>
    <span id="connLabel">Disconnected</span>
  </div>
</div>

<!-- Main area -->
<div class="main">
  <!-- Left column: screenshot + actions -->
  <div class="left-col">
    <div class="screenshot-panel">
      <div class="screenshot-controls">
        <button onclick="loadScreenshot()">Reload</button>
        <label><input type="checkbox" id="autoReload" onchange="toggleAutoReload()"> Auto-Reload</label>
      </div>
      <canvas id="screenshotCanvas" class="screenshot-canvas" width="640" height="400"></canvas>
    </div>
    <div class="action-panel">
      <h3>Actions</h3>
      <div class="action-grid">
        <!-- Verb Action -->
        <div class="action-group">
          <h4>Verb Action</h4>
          <div class="action-row">
            <select id="verbSelect"><option value="">Loading...</option></select>
            <input type="number" id="verbObjectId" placeholder="Obj ID">
          </div>
          <div class="action-row">
            <input type="number" id="verbTargetId" placeholder="Target (opt)">
            <button onclick="executeVerb()">Execute</button>
          </div>
        </div>
        <!-- Walk To -->
        <div class="action-group">
          <h4>Walk To</h4>
          <div class="action-row">
            <label>X<input type="number" id="walkX" value="160"></label>
            <label>Y<input type="number" id="walkY" value="100"></label>
            <button onclick="executeWalk()">Walk</button>
          </div>
        </div>
        <!-- Click -->
        <div class="action-group">
          <h4>Click</h4>
          <div class="action-row">
            <label>X<input type="number" id="clickX" value="160"></label>
            <label>Y<input type="number" id="clickY" value="100"></label>
          </div>
          <div class="action-row">
            <label><input type="radio" name="clickBtn" value="left" checked> Left</label>
            <label><input type="radio" name="clickBtn" value="right"> Right</label>
            <button onclick="executeClick()">Click</button>
          </div>
        </div>
        <!-- Save/Load -->
        <div class="action-group">
          <h4>Save / Load</h4>
          <div class="action-row">
            <label>Slot<input type="number" id="saveSlot" value="1" min="0" max="99"></label>
            <input type="text" id="saveDesc" placeholder="Description">
          </div>
          <div class="action-row">
            <button onclick="executeSave()">Save</button>
            <button onclick="executeLoad()">Load</button>
          </div>
        </div>
      </div>
      <!-- Dialog choices (shown when active) -->
      <div id="dialogPanel" style="display:none;margin-top:8px;">
        <div class="action-group">
          <h4>Dialog Choices</h4>
          <div id="dialogChoices"></div>
        </div>
      </div>
    </div>
  </div>

  <!-- Right column: state tabs -->
  <div class="right-col">
    <div class="tab-bar">
      <button class="tab-btn active" onclick="switchTab('room',this)">Room</button>
      <button class="tab-btn" onclick="switchTab('actors',this)">Actors</button>
      <button class="tab-btn" onclick="switchTab('inventory',this)">Inventory</button>
      <button class="tab-btn" onclick="switchTab('verbs',this)">Verbs</button>
      <button class="tab-btn" onclick="switchTab('hotspots',this)">Hotspots</button>
      <button class="tab-btn" onclick="switchTab('boxes',this)">Boxes</button>
    </div>
    <div class="tab-content">
      <!-- Room tab -->
      <div class="tab-page active" id="tab-room">
        <div class="refresh-row"><button onclick="refreshRoom()">Refresh</button></div>
        <table>
          <tbody id="roomBody">
            <tr><td colspan="2" style="color:var(--text-dim)">Click Refresh to load</td></tr>
          </tbody>
        </table>
      </div>
      <!-- Actors tab -->
      <div class="tab-page" id="tab-actors">
        <div class="refresh-row"><button onclick="refreshActors()">Refresh</button></div>
        <table>
          <thead><tr><th>ID</th><th>Name</th><th>X</th><th>Y</th><th>Facing</th><th>Moving</th><th>Talking</th></tr></thead>
          <tbody id="actorsBody"></tbody>
        </table>
      </div>
      <!-- Inventory tab -->
      <div class="tab-page" id="tab-inventory">
        <div class="refresh-row"><button onclick="refreshInventory()">Refresh</button></div>
        <table>
          <thead><tr><th>ID</th><th>Name</th></tr></thead>
          <tbody id="inventoryBody"></tbody>
        </table>
      </div>
      <!-- Verbs tab -->
      <div class="tab-page" id="tab-verbs">
        <div class="refresh-row"><button onclick="refreshVerbs()">Refresh</button></div>
        <table>
          <thead><tr><th>ID</th><th>Label</th><th>Key</th></tr></thead>
          <tbody id="verbsBody"></tbody>
        </table>
      </div>
      <!-- Hotspots tab -->
      <div class="tab-page" id="tab-hotspots">
        <div class="refresh-row"><button onclick="refreshHotspots()">Refresh</button></div>
        <table>
          <thead><tr><th>ID</th><th>Name</th><th>X</th><th>Y</th><th>W</th><th>H</th><th>WalkTo</th></tr></thead>
          <tbody id="hotspotsBody"></tbody>
        </table>
      </div>
      <!-- Boxes tab -->
      <div class="tab-page" id="tab-boxes">
        <div class="refresh-row"><button onclick="refreshBoxes()">Refresh</button></div>
        <table>
          <thead><tr><th>ID</th><th>UL</th><th>UR</th><th>LL</th><th>LR</th><th>Flags</th><th>Walkable</th></tr></thead>
          <tbody id="boxesBody"></tbody>
        </table>
      </div>
    </div>
  </div>
</div>

<!-- Event log -->
<div class="event-panel">
  <div class="event-header">
    <h3>Event Log</h3>
    <button class="secondary" onclick="clearEvents()">Clear</button>
  </div>
  <div class="event-list" id="eventList"></div>
</div>

<script>
/* ================================================================
   Globals
   ================================================================ */
var autoReloadTimer = null;
var eventSource = null;
var MAX_EVENTS = 200;

/* ================================================================
   Utility helpers
   ================================================================ */
function showToast(message) {
  var container = document.getElementById('toastContainer');
  var toast = document.createElement('div');
  toast.className = 'toast';
  toast.textContent = message;
  container.appendChild(toast);
  setTimeout(function() { toast.remove(); }, 4000);
}

function apiFetch(url) {
  return fetch(url).then(function(res) {
    if (!res.ok) throw new Error(res.status + ' ' + res.statusText);
    return res;
  }).catch(function(err) {
    showToast('GET ' + url + ' failed: ' + err.message);
    throw err;
  });
}

function apiPost(url, body) {
  return fetch(url, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body)
  }).then(function(res) {
    if (!res.ok) return res.json().then(function(j) { throw new Error(j.error || res.statusText); });
    return res.json();
  }).catch(function(err) {
    showToast('POST ' + url + ' failed: ' + err.message);
    throw err;
  });
}

function escapeHtml(str) {
  var div = document.createElement('div');
  div.appendChild(document.createTextNode(str));
  return div.innerHTML;
}

function boolBadge(val) {
  return val
    ? '<span style="color:var(--success)">yes</span>'
    : '<span style="color:var(--text-dim)">no</span>';
}

/* ================================================================
   Screenshot
   ================================================================ */
function loadScreenshot() {
  var canvas = document.getElementById('screenshotCanvas');
  var ctx = canvas.getContext('2d');
  apiFetch('/api/screenshot').then(function(res) {
    return res.blob();
  }).then(function(blob) {
    var img = new Image();
    img.onload = function() {
      canvas.width = img.width * 2;
      canvas.height = img.height * 2;
      ctx.imageSmoothingEnabled = false;
      ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
      URL.revokeObjectURL(img.src);
    };
    img.src = URL.createObjectURL(blob);
  }).catch(function() { /* toast already shown */ });
}

function toggleAutoReload() {
  var checked = document.getElementById('autoReload').checked;
  if (checked) {
    loadScreenshot();
    autoReloadTimer = setInterval(loadScreenshot, 500);
  } else {
    if (autoReloadTimer) { clearInterval(autoReloadTimer); autoReloadTimer = null; }
  }
}

/* ================================================================
   Tab switching
   ================================================================ */
function switchTab(name, btnEl) {
  var pages = document.querySelectorAll('.tab-page');
  for (var i = 0; i < pages.length; i++) pages[i].classList.remove('active');
  var btns = document.querySelectorAll('.tab-btn');
  for (var i = 0; i < btns.length; i++) btns[i].classList.remove('active');
  document.getElementById('tab-' + name).classList.add('active');
  btnEl.classList.add('active');
}

/* ================================================================
   State panels — Room
   ================================================================ */
function refreshRoom() {
  apiFetch('/api/room').then(function(res) { return res.json(); }).then(function(data) {
    var rows = '';
    rows += '<tr><td>Room ID</td><td>' + data.currentRoom + '</td></tr>';
    rows += '<tr><td>Room Name</td><td>' + escapeHtml(data.roomName || '(unnamed)') + '</td></tr>';
    rows += '<tr><td>Room Size</td><td>' + data.roomWidth + ' x ' + data.roomHeight + '</td></tr>';
    rows += '<tr><td>Screen Size</td><td>' + data.screenWidth + ' x ' + data.screenHeight + '</td></tr>';
    rows += '<tr><td>Camera X</td><td>' + data.cameraX + '</td></tr>';
    rows += '<tr><td>Camera Y</td><td>' + data.cameraY + '</td></tr>';
    document.getElementById('roomBody').innerHTML = rows;
  });
}

/* ================================================================
   State panels — Actors
   ================================================================ */
function refreshActors() {
  apiFetch('/api/actors').then(function(res) { return res.json(); }).then(function(actors) {
    var rows = '';
    for (var i = 0; i < actors.length; i++) {
      var a = actors[i];
      rows += '<tr>';
      rows += '<td>' + a.id + '</td>';
      rows += '<td>' + escapeHtml(a.name || '') + '</td>';
      rows += '<td>' + a.x + '</td>';
      rows += '<td>' + a.y + '</td>';
      rows += '<td>' + a.facing + '</td>';
      rows += '<td>' + boolBadge(a.isMoving) + '</td>';
      rows += '<td>' + boolBadge(a.isTalking) + '</td>';
      rows += '</tr>';
    }
    document.getElementById('actorsBody').innerHTML = rows || '<tr><td colspan="7" style="color:var(--text-dim)">No actors in room</td></tr>';
  });
}

/* ================================================================
   State panels — Inventory
   ================================================================ */
function refreshInventory() {
  apiFetch('/api/inventory').then(function(res) { return res.json(); }).then(function(items) {
    var rows = '';
    for (var i = 0; i < items.length; i++) {
      rows += '<tr><td>' + items[i].id + '</td><td>' + escapeHtml(items[i].name || '') + '</td></tr>';
    }
    document.getElementById('inventoryBody').innerHTML = rows || '<tr><td colspan="2" style="color:var(--text-dim)">Empty inventory</td></tr>';
  });
}

/* ================================================================
   State panels — Verbs
   ================================================================ */
function refreshVerbs() {
  apiFetch('/api/verbs').then(function(res) { return res.json(); }).then(function(verbs) {
    var rows = '';
    for (var i = 0; i < verbs.length; i++) {
      rows += '<tr><td>' + verbs[i].id + '</td><td>' + escapeHtml(verbs[i].label || '') + '</td><td>' + verbs[i].key + '</td></tr>';
    }
    document.getElementById('verbsBody').innerHTML = rows || '<tr><td colspan="3" style="color:var(--text-dim)">No verbs</td></tr>';
    populateVerbDropdown(verbs);
  });
}

function populateVerbDropdown(verbs) {
  var sel = document.getElementById('verbSelect');
  sel.innerHTML = '';
  for (var i = 0; i < verbs.length; i++) {
    var opt = document.createElement('option');
    opt.value = verbs[i].id;
    opt.textContent = verbs[i].label || ('Verb ' + verbs[i].id);
    sel.appendChild(opt);
  }
  if (verbs.length === 0) {
    var opt = document.createElement('option');
    opt.value = '';
    opt.textContent = '(none)';
    sel.appendChild(opt);
  }
}

/* ================================================================
   State panels — Hotspots
   ================================================================ */
function refreshHotspots() {
  apiFetch('/api/hotspots').then(function(res) { return res.json(); }).then(function(hotspots) {
    var rows = '';
    for (var i = 0; i < hotspots.length; i++) {
      var h = hotspots[i];
      rows += '<tr>';
      rows += '<td>' + h.id + '</td>';
      rows += '<td>' + escapeHtml(h.name || '') + '</td>';
      rows += '<td>' + h.x + '</td>';
      rows += '<td>' + h.y + '</td>';
      rows += '<td>' + h.width + '</td>';
      rows += '<td>' + h.height + '</td>';
      rows += '<td>' + h.walkToX + ',' + h.walkToY + '</td>';
      rows += '</tr>';
    }
    document.getElementById('hotspotsBody').innerHTML = rows || '<tr><td colspan="7" style="color:var(--text-dim)">No hotspots</td></tr>';
  });
}

/* ================================================================
   State panels — Boxes
   ================================================================ */
function refreshBoxes() {
  apiFetch('/api/boxes').then(function(res) { return res.json(); }).then(function(data) {
    var boxes = data.all || [];
    var rows = '';
    for (var i = 0; i < boxes.length; i++) {
      var b = boxes[i];
      var c = b.coords;
      rows += '<tr>';
      rows += '<td>' + b.id + '</td>';
      rows += '<td>' + c.ulX + ',' + c.ulY + '</td>';
      rows += '<td>' + c.urX + ',' + c.urY + '</td>';
      rows += '<td>' + c.llX + ',' + c.llY + '</td>';
      rows += '<td>' + c.lrX + ',' + c.lrY + '</td>';
      rows += '<td>' + b.flags + '</td>';
      rows += '<td>' + boolBadge(b.isWalkable) + '</td>';
      rows += '</tr>';
    }
    document.getElementById('boxesBody').innerHTML = rows || '<tr><td colspan="7" style="color:var(--text-dim)">No boxes</td></tr>';
  });
}

/* ================================================================
   Actions — Verb
   ================================================================ */
function executeVerb() {
  var verbId = parseInt(document.getElementById('verbSelect').value, 10);
  var objectId = parseInt(document.getElementById('verbObjectId').value, 10);
  var targetId = parseInt(document.getElementById('verbTargetId').value, 10);

  if (!verbId || !objectId) {
    showToast('Verb and Object ID are required');
    return;
  }

  if (targetId) {
    apiPost('/api/verb_multi', { verbId: verbId, objectId: objectId, targetId: targetId });
  } else {
    apiPost('/api/verb', { verbId: verbId, objectId: objectId });
  }
}

/* ================================================================
   Actions — Walk
   ================================================================ */
function executeWalk() {
  var x = parseInt(document.getElementById('walkX').value, 10);
  var y = parseInt(document.getElementById('walkY').value, 10);
  if (isNaN(x) || isNaN(y)) { showToast('X and Y are required'); return; }
  apiPost('/api/walk', { x: x, y: y });
}

/* ================================================================
   Actions — Click
   ================================================================ */
function executeClick() {
  var x = parseInt(document.getElementById('clickX').value, 10);
  var y = parseInt(document.getElementById('clickY').value, 10);
  if (isNaN(x) || isNaN(y)) { showToast('X and Y are required'); return; }
  var radios = document.getElementsByName('clickBtn');
  var isRight = false;
  for (var i = 0; i < radios.length; i++) {
    if (radios[i].checked && radios[i].value === 'right') isRight = true;
  }
  apiPost('/api/click', { x: x, y: y, rightButton: isRight });
}

/* ================================================================
   Actions — Save / Load
   ================================================================ */
function executeSave() {
  var slot = parseInt(document.getElementById('saveSlot').value, 10);
  var desc = document.getElementById('saveDesc').value || 'API Save';
  if (isNaN(slot)) { showToast('Slot is required'); return; }
  apiPost('/api/save', { slot: slot, description: desc });
}

function executeLoad() {
  var slot = parseInt(document.getElementById('saveSlot').value, 10);
  if (isNaN(slot)) { showToast('Slot is required'); return; }
  apiPost('/api/load', { slot: slot });
}

/* ================================================================
   Actions — Dialog
   ================================================================ */
function refreshDialog() {
  apiFetch('/api/dialog').then(function(res) { return res.json(); }).then(function(data) {
    var panel = document.getElementById('dialogPanel');
    var container = document.getElementById('dialogChoices');
    if (!data.isActive || data.choices.length === 0) {
      panel.style.display = 'none';
      return;
    }
    panel.style.display = 'block';
    var html = '';
    if (data.currentLine) {
      html += '<div style="color:var(--info);margin-bottom:6px;font-size:12px;">' + escapeHtml(data.speakerName || 'Speaker') + ': ' + escapeHtml(data.currentLine) + '</div>';
    }
    for (var i = 0; i < data.choices.length; i++) {
      html += '<button class="dialog-choice-btn" onclick="chooseDialog(' + data.choices[i].index + ')">' + escapeHtml(data.choices[i].text) + '</button>';
    }
    container.innerHTML = html;
  });
}

function chooseDialog(index) {
  apiPost('/api/dialog', { choiceIndex: index }).then(function() {
    setTimeout(refreshDialog, 300);
  });
}

/* ================================================================
   SSE Event Log
   ================================================================ */
var EVENT_COLORS = {
  scene_changed: '#4caf50',
  dialog_line: '#2196f3',
  dialog_choices: '#2196f3',
  dialog_ended: '#2196f3',
  actor_moved: '#ff9800',
  actor_talking: '#ff9800',
  inventory_changed: '#9c27b0',
  verb_executed: '#9c27b0',
  game_state_changed: '#f44336',
  walk_complete: '#4caf50',
  player_stopped: '#607d8b'
};

function eventBadgeColor(type) {
  return EVENT_COLORS[type] || '#607d8b';
}

function addEventEntry(type, payload) {
  var list = document.getElementById('eventList');
  var now = new Date();
  var ts = ('0' + now.getHours()).slice(-2) + ':' + ('0' + now.getMinutes()).slice(-2) + ':' + ('0' + now.getSeconds()).slice(-2) + '.' + ('00' + now.getMilliseconds()).slice(-3);

  var entry = document.createElement('div');
  entry.className = 'event-entry';
  entry.innerHTML =
    '<span class="event-ts">' + ts + '</span>' +
    '<span class="event-badge" style="background:' + eventBadgeColor(type) + '">' + escapeHtml(type) + '</span>' +
    '<span class="event-payload">' + escapeHtml(payload) + '</span>';

  list.appendChild(entry);

  /* Enforce max entries */
  while (list.childElementCount > MAX_EVENTS) {
    list.removeChild(list.firstElementChild);
  }

  /* Auto-scroll */
  list.scrollTop = list.scrollHeight;
}

function clearEvents() {
  document.getElementById('eventList').innerHTML = '';
}

function connectEventSource() {
  if (eventSource) {
    eventSource.close();
    eventSource = null;
  }

  var dot = document.getElementById('connDot');
  var label = document.getElementById('connLabel');

  eventSource = new EventSource('/events');

  eventSource.onopen = function() {
    dot.classList.add('connected');
    label.textContent = 'Connected';
  };

  eventSource.onerror = function() {
    dot.classList.remove('connected');
    label.textContent = 'Reconnecting...';
  };

  /* Listen for each known event type */
  var eventTypes = [
    'scene_changed', 'dialog_line', 'dialog_choices', 'dialog_ended',
    'actor_moved', 'actor_talking', 'inventory_changed', 'verb_executed',
    'game_state_changed', 'walk_complete', 'player_stopped'
  ];

  for (var i = 0; i < eventTypes.length; i++) {
    (function(evtType) {
      eventSource.addEventListener(evtType, function(e) {
        addEventEntry(evtType, e.data);

        /* Auto-refresh dialog panel on dialog events */
        if (evtType === 'dialog_choices' || evtType === 'dialog_ended') {
          refreshDialog();
        }
      });
    })(eventTypes[i]);
  }
}

/* ================================================================
   Initialization
   ================================================================ */
function init() {
  connectEventSource();
  refreshVerbs();
  refreshRoom();
  refreshDialog();
  loadScreenshot();
}

init();
</script>
</body>
</html>
)HTMLPAGE";

#endif // USE_SCUMM_API
#endif // SCUMM_API_TESTPAGE_H
