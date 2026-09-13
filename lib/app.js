// lib/app.js — the actual Express app, shared between local dev (server.js)
// and the Vercel serverless function (api/index.js).
//
// Hangman needs the secret word to persist across several requests (one
// per guessed letter), but each request spawns a fresh, stateless C
// process. So the *word* and current progress live in an httpOnly cookie
// on the server side — the browser carries it automatically, but page
// JavaScript can never read it, so there's no way to peek at the answer
// via devtools' console the way there would be if it were sent as plain
// JSON. (A determined user could still open the cookie itself in the
// browser's Application/Storage panel — true secrecy would need a real
// server-side session store instead of a cookie. Fine for a portfolio
// project; worth knowing if you ever turn this into something with
// real stakes.)
const express = require("express");
const path = require("path");
const os = require("os");
const fs = require("fs");
const { execFile } = require("child_process");

const app = express();
app.use(express.json());
app.use(express.static(path.join(__dirname, "..", "public")));

const SOURCE_BINARY = path.join(__dirname, "..", "hangman");
const COOKIE_NAME = "hangman_state";

// Same read-only-filesystem / lost-executable-bit workaround as the
// rock-paper-scissors project: on Vercel, copy the bundled binary into
// /tmp (the one writable, executable location) before running it.
function resolveBinaryPath() {
  if (!process.env.VERCEL) {
    return SOURCE_BINARY;
  }
  const tmpBinary = path.join(os.tmpdir(), "hangman");
  try {
    if (!fs.existsSync(tmpBinary)) {
      fs.copyFileSync(SOURCE_BINARY, tmpBinary);
    }
    fs.chmodSync(tmpBinary, 0o755);
    return tmpBinary;
  } catch (err) {
    console.error("Could not prepare hangman binary in /tmp:", err);
    return SOURCE_BINARY;
  }
}

function parseCookies(req) {
  const header = req.headers.cookie;
  const cookies = {};
  if (!header) return cookies;
  header.split(";").forEach((pair) => {
    const idx = pair.indexOf("=");
    if (idx === -1) return;
    cookies[pair.slice(0, idx).trim()] = decodeURIComponent(pair.slice(idx + 1).trim());
  });
  return cookies;
}

function encodeState(state) {
  return Buffer.from(JSON.stringify(state)).toString("base64");
}

function decodeState(raw) {
  try {
    return JSON.parse(Buffer.from(raw, "base64").toString("utf8"));
  } catch {
    return null;
  }
}

function setStateCookie(res, state) {
  res.cookie(COOKIE_NAME, encodeState(state), {
    httpOnly: true,
    sameSite: "lax",
    maxAge: 15 * 60 * 1000, // 15 minutes
  });
}

function parseEngineOutput(stdout) {
  const data = {};
  stdout
    .trim()
    .split("\n")
    .forEach((line) => {
      const idx = line.indexOf(":");
      if (idx === -1) return;
      data[line.slice(0, idx).trim().toLowerCase()] = line.slice(idx + 1).trim();
    });
  return data;
}

app.post("/new-game", (req, res) => {
  execFile(resolveBinaryPath(), ["start"], (err, stdout, stderr) => {
    if (err) {
      console.error("hangman start failed:", stderr || err.message);
      return res.status(500).json({ error: "game engine failed to run" });
    }

    const data = parseEngineOutput(stdout);
    if (!data.word || !data.masked || !data.attempts) {
      return res.status(500).json({ error: "unexpected output from game engine" });
    }

    setStateCookie(res, {
      word: data.word,
      masked: data.masked,
      attempts: Number(data.attempts),
    });

    res.json({ masked: data.masked, attempts: Number(data.attempts), status: data.status });
  });
});

app.post("/guess", (req, res) => {
  const cookies = parseCookies(req);
  const state = cookies[COOKIE_NAME] && decodeState(cookies[COOKIE_NAME]);

  if (!state) {
    return res.status(400).json({ error: "no active game — start a new one" });
  }

  const letter = ((req.body && req.body.letter) || "").toString().trim().toLowerCase();
  if (!/^[a-z]$/.test(letter)) {
    return res.status(400).json({ error: "guess must be a single letter a-z" });
  }

  const args = ["guess", state.word, state.masked, String(state.attempts), letter];

  execFile(resolveBinaryPath(), args, (err, stdout, stderr) => {
    if (err) {
      console.error("hangman guess failed:", stderr || err.message);
      return res.status(500).json({ error: "game engine failed to run" });
    }

    const data = parseEngineOutput(stdout);
    if (!data.masked || !data.attempts || !data.status) {
      return res.status(500).json({ error: "unexpected output from game engine" });
    }

    setStateCookie(res, {
      word: state.word,
      masked: data.masked,
      attempts: Number(data.attempts),
    });

    const response = {
      masked: data.masked,
      attempts: Number(data.attempts),
      found: data.found === "yes",
      status: data.status, // "playing" | "won" | "lost"
    };

    if (data.status !== "playing") {
      response.word = state.word; // only reveal the word once the game has ended
    }

    res.json(response);
  });
});

module.exports = app;
