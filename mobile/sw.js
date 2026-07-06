const CACHE_NAME = 'tr808-v12';
const ASSETS = [
  '/TR-808-Synth/',
  '/TR-808-Synth/index.html',
  '/TR-808-Synth/css/style.css',
  '/TR-808-Synth/js/audio-engine.js',
  '/TR-808-Synth/js/sequencer.js',
  '/TR-808-Synth/js/ui.js',
  '/TR-808-Synth/manifest.json',
];

self.addEventListener('install', (event) => {
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => cache.addAll(ASSETS))
  );
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) =>
      Promise.all(keys.filter((k) => k !== CACHE_NAME).map((k) => caches.delete(k)))
    )
  );
  self.clients.claim();
});

self.addEventListener('fetch', (event) => {
  event.respondWith(
    caches.match(event.request).then((cached) => cached || fetch(event.request))
  );
});
