import express from "express";
import fetch from "node-fetch";

const ESP_IP = process.env.ESP_IP || "192.168.x.x";
const POLL_MS = 200;
const PORT = 3000;

const EFFECTS = ["SPEED_UP", "REVERSE", "WALLS", "SHRINK"];
const EFFECT_LABELS = {
  SPEED_UP: "OVERCLOCK",
  REVERSE:  "MIRROR MAZE",
  WALLS:    "SECTOR BLOCK",
  SHRINK:   "NEURAL DECAY",
};
let voteState = { phase: "idle", options: [], votes: [0, 0], timeLeft: 0, winner: null };

function startVoteRound() {
  // Pick 2 distinct random effects
  const shuffled = [...EFFECTS].sort(() => Math.random() - 0.5);
  const options = [shuffled[0], shuffled[1]];
  voteState = { phase: "voting", options, votes: [0, 0], timeLeft: 5, winner: null };

  const countdown = setInterval(() => {
    voteState.timeLeft--;
    if (voteState.timeLeft <= 0) clearInterval(countdown);
  }, 1000);

  setTimeout(async () => {
    clearInterval(countdown);
    // Tally winner
    const winnerIdx = voteState.votes[0] >= voteState.votes[1]
      ? (voteState.votes[0] === voteState.votes[1] ? Math.round(Math.random()) : 0)
      : 1;
    const winnerEffect = voteState.options[winnerIdx];

    try {
      await fetch(`http://${ESP_IP}/effect`, {
        method: "POST",
        body: winnerEffect,
        signal: AbortSignal.timeout(3000),
      });
    } catch (e) {
      console.warn(`[vote] Failed to send effect to ESP32: ${e.message}`);
    }

    voteState = { phase: "cooldown", options, votes: voteState.votes, timeLeft: 5, winner: winnerEffect };

    // Cooldown countdown
    const cool = setInterval(() => {
      voteState.timeLeft--;
      if (voteState.timeLeft <= 0) clearInterval(cool);
    }, 1000);

    setTimeout(() => {
      clearInterval(cool);
      startVoteRound();
    }, 5000);
  }, 5000);
}

setTimeout(startVoteRound, 3000);

const HTML = `<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>VIPER-1 // LIVE FEED</title>

    <style>
        :root {
            --void:       #020509;
            --deep:       #060d1a;
            --cyan:       #00e5ff;
            --cyan-dim:   rgba(0,229,255,0.12);
            --cyan-glow:  rgba(0,229,255,0.45);
            --purple:     #7c3aff;
            --food:       #ff2b5e;
            --food-glow:  rgba(255,43,94,0.55);
            --panel:      rgba(0,229,255,0.04);
            --border:     rgba(0,229,255,0.22);
            --muted:      rgba(0,229,255,0.45);
        }
        * { margin:0; padding:0; box-sizing:border-box; }

        body {
            background: var(--void);
            color: var(--cyan);
            font-family: 'Courier New', monospace;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: 20px 20px 20px 32px;
            overflow-x: hidden;
            overflow-y: auto;
            position: relative;
        }

        /* Starfield */
        #stars {
            position: fixed;
            inset: 0;
            z-index: 0;
            pointer-events: none;
        }

        /* Scanlines */
        body::after {
            content: '';
            position: fixed;
            inset: 0;
            background: repeating-linear-gradient(
                0deg,
                transparent,
                transparent 3px,
                rgba(0,0,0,0.07) 3px,
                rgba(0,0,0,0.07) 4px
            );
            pointer-events: none;
            z-index: 200;
        }

        /* Nebula glow blobs */
        body::before {
            content: '';
            position: fixed;
            inset: 0;
            background:
                radial-gradient(ellipse 60% 40% at 15% 20%, rgba(124,58,255,0.07) 0%, transparent 60%),
                radial-gradient(ellipse 50% 35% at 85% 75%, rgba(0,229,255,0.05) 0%, transparent 60%),
                radial-gradient(ellipse 40% 30% at 50% 50%, rgba(124,58,255,0.04) 0%, transparent 70%);
            pointer-events: none;
            z-index: 0;
        }

        .page-layout {
            position: relative;
            z-index: 10;
            display: flex;
            flex-direction: row;
            align-items: flex-start;
            justify-content: center;
            gap: 24px;
            width: 100%;
            max-width: 820px;
        }

        .left-col {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 14px;
            flex-shrink: 0;
        }

        .right-col {
            display: flex;
            flex-direction: column;
            align-self: stretch;
            min-width: 220px;
            width: 240px;
            flex-shrink: 0;
            padding-top: 4px;
        }

        /* keep old .container name working for nothing that still uses it */
        .container { display: contents; }

        /* ── TITLE ── */
        .title-block { text-align: center; }

        .title-eyebrow {
            font-family: 'Courier New', monospace;
            font-size: 9px;
            letter-spacing: 5px;
            color: var(--muted);
            text-transform: uppercase;
            margin-bottom: 4px;
        }

        h1 {
            font-size: clamp(22px, 7vw, 32px);
            font-weight: 900;
            letter-spacing: 4px;
            text-transform: uppercase;
            color: var(--cyan);
            text-shadow:
                0 0 12px var(--cyan),
                0 0 30px rgba(0,229,255,0.35),
                0 0 60px rgba(0,229,255,0.12);
            line-height: 1;
        }

        .title-sub {
            font-family: 'Courier New', monospace;
            font-size: 9px;
            letter-spacing: 3px;
            color: rgba(124,58,255,0.75);
            margin-top: 5px;
            text-shadow: 0 0 8px rgba(124,58,255,0.5);
        }

        /* ── HUD ROW ── */
        .hud-row {
            display: flex;
            gap: 10px;
            width: 100%;
        }

        .hud-panel {
            flex: 1;
            border: 1px solid var(--border);
            background: var(--panel);
            padding: 8px 10px 7px;
            border-radius: 3px;
            position: relative;
            overflow: hidden;
        }

        /* sliding top highlight */
        .hud-panel::before {
            content: '';
            position: absolute;
            top: 0; left: -100%; right: 100%;
            height: 1px;
            background: linear-gradient(90deg, transparent, var(--cyan), transparent);
            animation: hud-sweep 4s linear infinite;
        }
        .hud-panel:nth-child(2)::before { animation-delay: -1.3s; }
        .hud-panel:nth-child(3)::before { animation-delay: -2.6s; }

        @keyframes hud-sweep {
            0%   { left: -100%; right: 100%; }
            100% { left: 100%;  right: -100%; }
        }

        .hud-label {
            font-family: 'Courier New', monospace;
            font-size: 8px;
            letter-spacing: 2px;
            color: var(--muted);
            text-transform: uppercase;
            margin-bottom: 3px;
        }

        .hud-value {
            font-size: 20px;
            font-weight: 700;
            color: var(--cyan);
            text-shadow: 0 0 8px var(--cyan-glow);
            letter-spacing: 2px;
            line-height: 1;
        }

        /* Signal bars */
        .signal-bars {
            display: flex;
            gap: 3px;
            align-items: flex-end;
            height: 20px;
            margin-top: 1px;
        }
        .signal-bars b {
            display: block;
            width: 5px;
            border-radius: 1px;
            background: var(--cyan);
            transition: opacity 0.4s, box-shadow 0.4s;
            opacity: 0.15;
        }
        .signal-bars b:nth-child(1) { height: 5px; }
        .signal-bars b:nth-child(2) { height: 9px; }
        .signal-bars b:nth-child(3) { height: 13px; }
        .signal-bars b:nth-child(4) { height: 17px; }
        .signal-bars b:nth-child(5) { height: 20px; }
        .signal-bars.on b { opacity: 1; box-shadow: 0 0 5px var(--cyan-glow); }

        #hud-status {
            font-size: 11px;
            letter-spacing: 1px;
        }

        /* ── SECTOR BLOCK ── */
        .sector-block {
            position: relative;
            width: min(82vw, 340px);
        }

        .sector-label {
            font-family: 'Courier New', monospace;
            font-size: 8px;
            letter-spacing: 4px;
            color: var(--muted);
            text-align: center;
            margin-bottom: 6px;
            text-transform: uppercase;
        }

        .grid-frame {
            position: relative;
            width: 100%;
        }

        /* Bracket corners */
        .bracket {
            position: absolute;
            width: 14px;
            height: 14px;
            border-color: var(--cyan);
            border-style: solid;
            opacity: 0.5;
            z-index: 2;
            transition: opacity 0.4s;
        }
        .bracket.tl { top:-3px; left:-3px;  border-width:2px 0 0 2px; }
        .bracket.tr { top:-3px; right:-3px; border-width:2px 2px 0 0; }
        .bracket.bl { bottom:-3px; left:-3px;  border-width:0 0 2px 2px; }
        .bracket.br { bottom:-3px; right:-3px; border-width:0 2px 2px 0; }
        .grid-frame:hover .bracket { opacity: 1; }

        /* Coord labels */
        .coord-x, .coord-y {
            font-family: 'Courier New', monospace;
            font-size: 7px;
            color: rgba(0,229,255,0.2);
            display: flex;
            position: absolute;
        }
        .coord-x {
            top: -12px; left: 0; right: 0;
            justify-content: space-around;
            padding: 0 6px;
        }
        .coord-y {
            top: 0; bottom: 0;
            left: -12px;
            flex-direction: column;
            justify-content: space-around;
            padding: 6px 0;
        }

        #grid {
            display: grid;
            grid-template-columns: repeat(8, 1fr);
            gap: 3px;
            background: rgba(0,15,35,0.85);
            padding: 8px;
            border: 1px solid var(--border);
            border-radius: 4px;
            width: 100%;
            aspect-ratio: 1 / 2;
            backdrop-filter: blur(6px);
        }

        .cell {
            border-radius: 2px;
            transition: background 0.08s ease, box-shadow 0.08s ease;
        }
        .cell.empty {
            background: rgba(0,229,255,0.03);
            border: 1px solid rgba(0,229,255,0.06);
        }
        .cell.head {
            background: #00e5ff;
            border-radius: 3px;
            box-shadow:
                0 0 6px var(--cyan),
                0 0 14px var(--cyan-glow),
                inset 0 0 4px rgba(255,255,255,0.4);
            animation: pulse-head 0.45s ease-in-out infinite alternate;
        }
        @keyframes pulse-head {
            from { box-shadow: 0 0 5px var(--cyan), 0 0 10px var(--cyan-glow); }
            to   { box-shadow: 0 0 10px var(--cyan), 0 0 22px var(--cyan-glow), 0 0 40px rgba(0,229,255,0.18); }
        }
        .cell.body {
            background: rgba(0,185,220,0.65);
            box-shadow: 0 0 4px rgba(0,229,255,0.4);
        }
        .cell.food {
            background: var(--food);
            border-radius: 50%;
            animation: pulse-food 0.75s ease-in-out infinite alternate;
        }
        @keyframes pulse-food {
            from { box-shadow: 0 0 5px var(--food), 0 0 10px var(--food-glow); transform: scale(0.82); }
            to   { box-shadow: 0 0 10px var(--food), 0 0 22px var(--food-glow); transform: scale(1.0);  }
        }
        .cell.wall {
            background: rgba(124,58,255,0.6);
            border-radius: 1px;
            box-shadow: 0 0 4px rgba(124,58,255,0.8);
        }

        /* Game Over overlay */
        .over-overlay {
            display: none;
            position: absolute;
            inset: 0;
            background: rgba(2,5,9,0.82);
            align-items: center;
            justify-content: center;
            flex-direction: column;
            gap: 6px;
            border-radius: 4px;
            z-index: 10;
            backdrop-filter: blur(3px);
        }
        .over-overlay.show { display: flex; }

        .over-title {
            font-size: clamp(18px, 5vw, 24px);
            font-weight: 900;
            letter-spacing: 5px;
            color: var(--food);
            text-shadow: 0 0 16px var(--food-glow), 0 0 32px rgba(255,43,94,0.3);
            animation: flicker 0.12s infinite;
        }
        @keyframes flicker {
            0%, 100% { opacity: 1; }
            48%, 52% { opacity: 0.82; }
        }
        .over-sub {
            font-family: 'Courier New', monospace;
            font-size: 9px;
            letter-spacing: 4px;
            color: rgba(255,43,94,0.55);
        }

        /* ── VOTE PANEL ── */
        #vote-panel {
            flex: 1;
            border: 1px solid rgba(255,43,94,0.3);
            background: rgba(255,43,94,0.03);
            border-radius: 4px;
            padding: 18px 16px;
            position: relative;
            overflow: hidden;
            display: flex;
            flex-direction: column;
            gap: 16px;
        }
        #vote-panel::before {
            content: '';
            position: absolute;
            top: 0; left: -100%; right: 100%;
            height: 1px;
            background: linear-gradient(90deg, transparent, var(--food), transparent);
            animation: hud-sweep 3s linear infinite;
        }
        .vote-header {
            display: flex;
            flex-direction: column;
            gap: 6px;
        }
        .vote-title {
            font-size: 8px;
            letter-spacing: 4px;
            color: var(--muted);
            text-transform: uppercase;
        }
        .vote-timer {
            font-size: 28px;
            font-weight: 700;
            letter-spacing: 2px;
            color: var(--food);
            text-shadow: 0 0 14px var(--food-glow);
            line-height: 1;
        }
        .vote-options {
            display: flex;
            flex-direction: column;
            gap: 12px;
            flex: 1;
        }
        .vote-option {
            flex: 1;
            display: flex;
            flex-direction: column;
            gap: 6px;
        }
        .vote-btn {
            width: 100%;
            flex: 1;
            padding: 20px 12px;
            font-family: 'Courier New', monospace;
            font-size: 13px;
            font-weight: 700;
            letter-spacing: 2px;
            text-transform: uppercase;
            background: transparent;
            color: var(--cyan);
            border: 1px solid var(--cyan);
            cursor: pointer;
            border-radius: 3px;
            transition: color 0.2s, background 0.2s, box-shadow 0.2s;
        }
        .vote-btn:hover:not(:disabled) {
            background: rgba(0,229,255,0.1);
            box-shadow: 0 0 18px var(--cyan-glow);
        }
        .vote-btn:disabled {
            opacity: 0.4;
            cursor: default;
        }
        .vote-btn.voted-choice {
            border-color: var(--food);
            color: var(--food);
            opacity: 1;
            background: rgba(255,43,94,0.08);
            box-shadow: 0 0 14px var(--food-glow);
        }
        .vote-count {
            font-size: 10px;
            letter-spacing: 2px;
            color: var(--muted);
            text-align: center;
        }
        .vote-cooldown {
            flex: 1;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            gap: 10px;
            text-align: center;
        }
        .vote-cooldown-label {
            font-size: 9px;
            letter-spacing: 3px;
            color: rgba(255,43,94,0.6);
            text-transform: uppercase;
        }
        .vote-cooldown-effect {
            font-size: 18px;
            font-weight: 700;
            letter-spacing: 3px;
            color: var(--food);
            text-shadow: 0 0 14px var(--food-glow);
            text-transform: uppercase;
            animation: flicker 1.2s infinite;
        }

        /* ── RESTART BUTTON ── */
        #restart {
            display: none;
            padding: 10px 28px;
            font-family: 'Courier New', monospace;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 4px;
            text-transform: uppercase;
            background: transparent;
            color: var(--cyan);
            border: 1px solid var(--cyan);
            cursor: pointer;
            border-radius: 2px;
            position: relative;
            overflow: hidden;
            transition: color 0.2s, text-shadow 0.2s;
        }
        #restart::before {
            content: '';
            position: absolute;
            inset: 0;
            background: var(--cyan);
            transform: translateX(-101%);
            transition: transform 0.25s ease;
            z-index: -1;
        }
        #restart:hover {
            color: var(--void);
            text-shadow: none;
            box-shadow: 0 0 18px var(--cyan-glow);
        }
        #restart:hover::before { transform: translateX(0); }

        /* ── FOOTER ── */
        .footer-line {
            font-family: 'Courier New', monospace;
            font-size: 8px;
            letter-spacing: 3px;
            color: rgba(0,229,255,0.18);
            text-align: center;
        }

        /* ── RESPONSIVE ── */
        @media (max-width: 640px) {
            body {
                padding: 16px 16px 24px 28px;
                justify-content: flex-start;
            }

            .page-layout {
                flex-direction: column;
                align-items: center;
                gap: 16px;
            }

            .left-col {
                width: 100%;
                max-width: 360px;
            }

            .sector-block {
                width: 100%;
                max-width: 360px;
            }

            .right-col {
                width: 100%;
                max-width: 360px;
                align-self: auto;
            }

            #vote-panel {
                min-height: unset;
            }

            .vote-btn {
                padding: 16px 12px;
            }

            .vote-timer {
                font-size: 22px;
            }
        }
    </style>
</head>
<body>
    <canvas id="stars"></canvas>

    <div class="page-layout">

        <div class="left-col">
            <div class="title-block">
                <div class="title-eyebrow">// live spectator feed //</div>
                <h1>VIPER&#8209;1</h1>
                <div class="title-sub">DEEP SPACE SECTOR SURVEILLANCE</div>
            </div>

            <div class="hud-row">
                <div class="hud-panel">
                    <div class="hud-label">Score</div>
                    <div class="hud-value" id="hud-score">0000</div>
                </div>
                <div class="hud-panel">
                    <div class="hud-label">Signal</div>
                    <div class="signal-bars" id="hud-signal">
                        <b></b><b></b><b></b><b></b><b></b>
                    </div>
                </div>
                <div class="hud-panel">
                    <div class="hud-label">Status</div>
                    <div class="hud-value" id="hud-status" style="font-size:11px;letter-spacing:1px;">INIT</div>
                </div>
            </div>

            <div class="sector-block">
                <div class="sector-label">&#9472;&#9472; sector map alpha &#9472;&#9472;</div>
                <div class="grid-frame">
                    <div class="coord-x">
                        <span>A</span><span>B</span><span>C</span><span>D</span>
                        <span>E</span><span>F</span><span>G</span><span>H</span>
                    </div>
                    <div class="coord-y">
                        <span>1</span><span>2</span><span>3</span><span>4</span>
                        <span>5</span><span>6</span><span>7</span><span>8</span>
                        <span>9</span><span>10</span><span>11</span><span>12</span>
                        <span>13</span><span>14</span><span>15</span><span>16</span>
                    </div>
                    <div class="bracket tl"></div>
                    <div class="bracket tr"></div>
                    <div class="bracket bl"></div>
                    <div class="bracket br"></div>
                    <div id="grid"></div>
                    <div class="over-overlay" id="over-overlay">
                        <div class="over-title">SIGNAL LOST</div>
                        <div class="over-sub">MISSION TERMINATED</div>
                    </div>
                </div>
            </div>

            <button id="restart" onclick="restartMission()">// RELAUNCH MISSION //</button>

            <div class="footer-line">ESP-32 UPLINK &nbsp;&#9472;&nbsp; 250ms REFRESH &nbsp;&#9472;&nbsp; AES-256</div>
        </div>

        <div class="right-col">
            <div id="vote-panel" style="display:none;">
                <div class="vote-header">
                    <div class="vote-title">&#9472;&#9472; chaos vote &#9472;&#9472;</div>
                    <div class="vote-timer" id="vote-timer"></div>
                </div>
                <div id="vote-body"></div>
            </div>
        </div>

    </div>

    <script>
        /* ---- Starfield ---- */
        const cv = document.getElementById('stars');
        const cx = cv.getContext('2d');
        let stars = [];
        function rsz() { cv.width = innerWidth; cv.height = innerHeight; }
        function mkStars() {
            stars = [];
            for (let i = 0; i < 220; i++) {
                stars.push({
                    x: Math.random() * cv.width,
                    y: Math.random() * cv.height,
                    r: Math.random() * 1.4 + 0.2,
                    o: Math.random() * 0.7 + 0.15,
                    sp: Math.random() * 0.4 + 0.05,
                    ph: Math.random() * Math.PI * 2,
                    hue: Math.random() < 0.15 ? 210 : (Math.random() < 0.1 ? 280 : 200)
                });
            }
        }
        function drawStars(ts) {
            cx.clearRect(0, 0, cv.width, cv.height);
            const t = ts / 1000;
            stars.forEach(s => {
                const alpha = s.o * (0.55 + 0.45 * Math.sin(t * s.sp + s.ph));
                cx.beginPath();
                cx.arc(s.x, s.y, s.r, 0, Math.PI * 2);
                cx.fillStyle = \`hsla(\${s.hue},80%,90%,\${alpha})\`;
                cx.fill();
            });
            requestAnimationFrame(drawStars);
        }
        rsz(); mkStars(); requestAnimationFrame(drawStars);
        window.addEventListener('resize', () => { rsz(); mkStars(); });

        /* ---- Grid setup ---- */
        const gridEl    = document.getElementById('grid');
        const scoreEl   = document.getElementById('hud-score');
        const signalEl  = document.getElementById('hud-signal');
        const statusEl  = document.getElementById('hud-status');
        const overEl    = document.getElementById('over-overlay');
        const restartBtn = document.getElementById('restart');
        const votePanel  = document.getElementById('vote-panel');
        const voteTimer  = document.getElementById('vote-timer');
        const voteBody   = document.getElementById('vote-body');

        const EFFECT_LABELS = {
          SPEED_UP: 'OVERCLOCK', REVERSE: 'MIRROR MAZE',
          WALLS: 'SECTOR BLOCK', SHRINK: 'NEURAL DECAY'
        };

        let lastVoteOptions = null;
        let hasVoted = false;
        let votedIndex = -1;

        // Client-side countdown — synced to server value on each poll
        let localTimeLeft = 0;
        let localPhase = 'idle';
        let timerInterval = null;

        function startLocalCountdown() {
            if (timerInterval) clearInterval(timerInterval);
            timerInterval = setInterval(() => {
                if (localTimeLeft > 0) {
                    localTimeLeft--;
                    renderTimer();
                }
            }, 1000);
        }

        function renderTimer() {
            if (localPhase === 'voting') {
                voteTimer.textContent = 'VOTING: ' + Math.max(0, localTimeLeft) + 's';
            } else if (localPhase === 'cooldown') {
                voteTimer.textContent = Math.max(0, localTimeLeft) + 's';
            }
        }

        function updateVotePanel(vs) {
            if (!vs || vs.phase === 'idle') {
                votePanel.style.display = 'none';
                if (timerInterval) { clearInterval(timerInterval); timerInterval = null; }
                return;
            }
            votePanel.style.display = 'block';

            // Detect new round — reset voted state
            const optKey = vs.options.join(',');
            if (optKey !== lastVoteOptions) {
                lastVoteOptions = optKey;
                hasVoted = false;
                votedIndex = -1;
            }

            // Sync local countdown to server (only reset if server value is ahead)
            if (vs.phase !== localPhase || vs.timeLeft > localTimeLeft) {
                localTimeLeft = vs.timeLeft;
                startLocalCountdown();
            }
            localPhase = vs.phase;
            renderTimer();

            if (vs.phase === 'voting') {
                const d = (i) => hasVoted ? 'disabled' : '';
                const cls = (i) => 'vote-btn' + (hasVoted && votedIndex === i ? ' voted-choice' : '');
                voteBody.innerHTML = \`
                  <div class="vote-options">
                    <div class="vote-option">
                      <button class="\${cls(0)}" onclick="castVote(0)" \${d(0)}>
                        \${EFFECT_LABELS[vs.options[0]] || vs.options[0]}
                      </button>
                      <span class="vote-count">\${vs.votes[0]} vote\${vs.votes[0] !== 1 ? 's' : ''}</span>
                    </div>
                    <div class="vote-option">
                      <button class="\${cls(1)}" onclick="castVote(1)" \${d(1)}>
                        \${EFFECT_LABELS[vs.options[1]] || vs.options[1]}
                      </button>
                      <span class="vote-count">\${vs.votes[1]} vote\${vs.votes[1] !== 1 ? 's' : ''}</span>
                    </div>
                  </div>\`;
            } else if (vs.phase === 'cooldown') {
                const label = EFFECT_LABELS[vs.winner] || vs.winner;
                voteBody.innerHTML = \`
                  <div class="vote-cooldown">
                    <div class="vote-cooldown-label">&#9889; effect active &#9889;</div>
                    <div class="vote-cooldown-effect">\${label}</div>
                  </div>\`;
            }
        }

        async function castVote(option) {
            if (hasVoted) return;
            hasVoted = true;
            votedIndex = option;
            try {
                await fetch('/vote', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ option })
                });
            } catch(e) {}
        }

        const CLS = ['empty','head','body','food','wall'];

        for (let i = 0; i < 128; i++) {
            const d = document.createElement('div');
            d.className = 'cell empty';
            d.id = 'c' + i;
            gridEl.appendChild(d);
        }

        function updateDisplay(data) {
            for (let y = 0; y < 16; y++)
                for (let x = 0; x < 8; x++)
                    document.getElementById('c' + (y*8+x)).className = 'cell ' + CLS[data.grid[y][x]];

            scoreEl.textContent = String(data.score).padStart(4,'0');
            signalEl.className = 'signal-bars on';

            if (data.gameOver) {
                statusEl.textContent  = 'LOST';
                statusEl.style.color  = '#ff2b5e';
                statusEl.style.textShadow = '0 0 8px rgba(255,43,94,0.6)';
                overEl.classList.add('show');
                restartBtn.style.display = 'block';
            } else {
                statusEl.textContent  = 'LIVE';
                statusEl.style.color  = '';
                statusEl.style.textShadow = '';
                overEl.classList.remove('show');
                restartBtn.style.display = 'none';
            }
        }

        async function fetchStatus() {
            try {
                const r = await fetch('/status');
                const d = await r.json();
                updateDisplay(d);
                updateVotePanel(d.voteState);
            } catch(e) {
                statusEl.textContent = 'ERR';
                statusEl.style.color = '#ff2b5e';
                signalEl.className = 'signal-bars';
            }
        }

        async function restartMission() {
            try {
                await fetch('/restart', { method: 'POST' });
                overEl.classList.remove('show');
                restartBtn.style.display = 'none';
            } catch(e) {}
        }

        setInterval(fetchStatus, 250);
        fetchStatus();
    </script>
</body>
</html>`;

let cachedState = null;
let lastPollError = null;

async function pollESP() {
  try {
    const res = await fetch(`http://${ESP_IP}/status`, { signal: AbortSignal.timeout(1000) });
    cachedState = await res.json();
    if (lastPollError) { console.log("[poller] ESP32 reconnected"); lastPollError = null; }
  } catch (e) {
    if (e.message !== lastPollError) {
      console.warn(`[poller] ESP32 unreachable: ${e.message}`);
      lastPollError = e.message;
    }
  }
}

setInterval(pollESP, POLL_MS);
pollESP();

const app = express();

app.use((req, res, next) => {
  res.setHeader("Access-Control-Allow-Origin", "*");
  next();
});

app.get("/", (req, res) => {
  res.setHeader("Content-Type", "text/html");
  res.send(HTML);
});

app.get("/status", (req, res) => {
  if (!cachedState) {
    return res.status(503).json({ error: "ESP32 not yet reachable" });
  }
  res.json({ ...cachedState, voteState });
});

app.post("/vote", express.json(), (req, res) => {
  if (voteState.phase !== "voting") return res.json({ error: "no active vote" });
  const { option } = req.body;
  if (option !== 0 && option !== 1) return res.json({ error: "invalid" });
  voteState.votes[option]++;
  res.json({ success: true, votes: voteState.votes });
});

app.post("/effect", express.text(), async (req, res) => {
  try {
    await fetch(`http://${ESP_IP}/effect`, {
      method: "POST",
      body: req.body,
      signal: AbortSignal.timeout(3000),
    });
    res.json({ success: true });
  } catch (e) {
    res.status(502).json({ error: `ESP32 unreachable: ${e.message}` });
  }
});

app.post("/restart", async (req, res) => {
  try {
    const upstream = await fetch(`http://${ESP_IP}/restart`, {
      method: "POST",
      signal: AbortSignal.timeout(3000),
    });
    const body = await upstream.json();
    res.json(body);
  } catch (e) {
    res.status(502).json({ error: `ESP32 unreachable: ${e.message}` });
  }
});

app.listen(PORT, () => {
  console.log(`Snake proxy listening on http://localhost:${PORT}`);
  console.log(`Polling ESP32 at http://${ESP_IP}/status every ${POLL_MS}ms`);
});
