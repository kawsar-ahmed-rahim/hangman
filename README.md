# 🪢 Hangman — Web Edition

A browser version of a classic terminal Hangman game — the original **C** game logic still runs for real on every guess, wired up to a **Node.js/Express** backend and a **vanilla HTML/CSS/JS** frontend so it can be played (and deployed) from a browser.

---

## 🚀 Features

* 🔤 Guess a word by clicking on-screen letters or typing on a real keyboard
* 🧠 Real C program picks the word and checks every guess — not reimplemented in JavaScript
* 🎨 Progressive gallows drawing that builds up with each wrong guess
* 🔒 Secret word kept server-side in an httpOnly cookie, invisible to page JavaScript
* ⌨️ Word list themed around the IT sector (programming, mongodb, computer, keyboard, console)
* 🏁 Win/lose screen that reveals the answer once the round ends
* ⚠️ Graceful error handling for invalid guesses or a missing game session
* 📱 Responsive layout, keyboard-focus visible
* ☁️ Deployable to Vercel as a serverless function

---

## 🛠️ Built With

* C (original game logic)
* Node.js + Express (backend, spawns the compiled C binary per action)
* HTML5 / CSS3 / JavaScript (ES6) — no frontend framework or build step
* Vercel (deployment target)

---

## 📂 Project Structure

```text
hangman-web/
│
├── api/
│   └── index.js       # Vercel serverless function entry point
│
├── lib/
│   └── app.js           # Shared Express app (used locally and on Vercel)
│
├── public/
│   └── index.html        # Frontend: HTML, CSS, and JS in one file
│
├── hangman.c              # Original game logic, split into "start" and "guess" commands
├── server.js               # Local dev server (`npm start`)
├── package.json
├── vercel.json              # Vercel build/routing config
└── README.md
```

---

## ⚙️ How It Works

1. On page load, the frontend requests a new game (`POST /new-game`).
2. The server runs the C program in `start` mode, which picks a random word from the list and returns a blank pattern for it. The server stores the secret word in an httpOnly cookie — never sent to the page's JavaScript.
3. Player clicks a letter (or types one).
4. The frontend sends that letter (`POST /guess`).
5. The server reads the word back out of the cookie, runs the C program in `guess` mode with the word, current pattern, attempts left, and the new letter.
6. The C program applies the original matching rules — reveal every matching position, lose an attempt only if nothing was found — and reports the updated pattern, attempts, and whether the game was won, lost, or still going.
7. The browser updates the masked word, the gallows drawing, and the on-screen keyboard. The secret word is only sent back to the browser once the game ends.

---

## 💡 Challenges Faced

The original program looped through one long-running process: pick a word once, then keep reading letters with `scanf()` until the game ended. A web request/response model can't hold that kind of long-running state — each request is independent and stateless.

### Solution

* Split the C program into two commands: `start` (pick a word) and `guess` (apply one letter) — so it stays a simple, stateless "referee" that answers one question per run, same as the rock-paper-scissors adaptation.
* Since the C program itself remembers nothing between runs, the *server* keeps the secret word in an httpOnly cookie tied to the browser session, passing it back into the C program on every guess. This keeps the word out of reach of page JavaScript, though not out of reach of someone deliberately inspecting cookies in devtools — a real "no cheating possible" version would need a server-side session store instead of a cookie.
* On Vercel specifically, the filesystem is read-only and bundled binaries can lose their executable permission — solved the same way as the rock-paper-scissors project: copy the compiled binary into `/tmp` and re-mark it executable at runtime before each cold start.

---

## 📚 What I Learned

* Managing state across multiple stateless requests using cookies
* The difference between `httpOnly` cookies (hidden from JS) and true server-side secrecy
* Spawning and communicating with a compiled C program from Node.js (`child_process.execFile`)
* Designing a small text-based protocol between two programs (parsing plain stdout into JSON)
* Building an interactive on-screen keyboard with both click and physical-keyboard support
* Constraints of serverless deployment (read-only filesystems, cold starts, bundling native binaries)

---

## 🔮 Future Improvements

* 🗂️ Replace the cookie with a real server-side session store for full secrecy
* 📈 Difficulty levels (longer words, fewer attempts)
* 🏆 Track win streak using Local Storage
* 🔊 Sound effects on correct/wrong guesses
* 💬 Category selector (not just IT words)
* 🌐 WebAssembly version that runs the C code directly in-browser, no backend needed

---

## 🔗 Live Demo

👉 **Live Website:** https://hangman-rosy.vercel.app/

---

## 👨‍💻 Author

**Rahim**

If you found this project helpful or interesting, feel free to ⭐ the repository and share your feedback. Contributions, suggestions, and improvements are always welcome!
