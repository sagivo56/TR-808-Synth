// יצירת אייקוני PWA מ-SVG באמצעות sharp.
// הרצה: npm run gen-icons
import sharp from "sharp";
import { mkdirSync } from "node:fs";
import { fileURLToPath } from "node:url";
import path from "node:path";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const OUT = path.join(__dirname, "..", "public", "icons");
mkdirSync(OUT, { recursive: true });

const BG = "#14171A";
const PANEL = "#1E2226";
const BORDER = "#3A4046";
const GREEN = "#8FBF6B";
const AMBER = "#E0A94C";
const RED = "#D9634B";

// מייצר SVG בגודל 512x512. pad = שוליים בטוחים ל-maskable.
function svg({ maskable }) {
  const S = 512;
  const pad = maskable ? 74 : 40; // אזור בטוח ל-maskable
  const inner = S - pad * 2;
  const r = maskable ? 0 : 96;

  // פס מפולח: 12 מקטעים, ירוק->כתום->אדום
  const segCount = 12;
  const barW = inner * 0.8;
  const barH = inner * 0.16;
  const barX = pad + (inner - barW) / 2;
  const barY = S / 2 - barH / 2;
  const gap = barW * 0.02;
  const segW = (barW - gap * (segCount - 1)) / segCount;

  let segs = "";
  for (let i = 0; i < segCount; i++) {
    const frac = (i + 1) / segCount;
    let color;
    if (frac <= 0.66) color = GREEN;
    else if (frac <= 0.9) color = AMBER;
    else color = RED;
    const x = barX + i * (segW + gap);
    segs += `<rect x="${x.toFixed(1)}" y="${barY.toFixed(1)}" width="${segW.toFixed(1)}" height="${barH.toFixed(1)}" rx="4" fill="${color}"/>`;
  }

  const dotY = barY + barH + inner * 0.14;
  const dotR = inner * 0.05;

  return `<svg xmlns="http://www.w3.org/2000/svg" width="${S}" height="${S}" viewBox="0 0 ${S} ${S}">
  <rect width="${S}" height="${S}" fill="${BG}"/>
  <rect x="${pad}" y="${pad}" width="${inner}" height="${inner}" rx="${r}" fill="${PANEL}" stroke="${BORDER}" stroke-width="4"/>
  ${segs}
  <circle cx="${(barX + segW / 2).toFixed(1)}" cy="${dotY.toFixed(1)}" r="${dotR.toFixed(1)}" fill="${GREEN}"/>
  <circle cx="${(barX + barW / 2).toFixed(1)}" cy="${dotY.toFixed(1)}" r="${dotR.toFixed(1)}" fill="${AMBER}"/>
  <circle cx="${(barX + barW - segW / 2).toFixed(1)}" cy="${dotY.toFixed(1)}" r="${dotR.toFixed(1)}" fill="${RED}"/>
</svg>`;
}

async function render(name, size, { maskable }) {
  const buf = Buffer.from(svg({ maskable }));
  await sharp(buf).resize(size, size).png().toFile(path.join(OUT, name));
  console.log("wrote", name);
}

await render("icon-192.png", 192, { maskable: false });
await render("icon-512.png", 512, { maskable: false });
await render("icon-maskable-512.png", 512, { maskable: true });
await render("apple-touch-icon.png", 180, { maskable: false });

console.log("Done.");
