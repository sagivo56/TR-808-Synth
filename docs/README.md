# אימון קליסטניקס — גרסת GitHub Pages

`index.html` כאן הוא אפליקציית מעקב האימונים (Tom Holland) כעמוד בודד,
עצמאי לחלוטין (אין build, אין שרת). מתארח דרך **GitHub Pages**.

## הפעלה חד-פעמית של GitHub Pages
בריפו ב-GitHub: **Settings → Pages → Build and deployment**
- **Source**: Deploy from a branch
- **Branch**: `main` · **Folder**: `/docs` → **Save**

הכתובת שתתקבל (אחרי דקה-שתיים):

```
https://sagivo56.github.io/TR-808-Synth/
```

## פיתוח
עורכים את `docs/index.html`, דוחפים ל-`main` (דרך PR) — ו-GitHub Pages
מתפרסם אוטומטית מחדש. בלי Railway.

## מה בפנים
- מצב A — AMRAP 20 דקות (Cindy)
- מצב B — סולם 1,500 חזרות (Monster Ladder)
- תרגיל אחד בכל לחיצה, עם אנימציית הדגמה (SVG)
- מתג ציוד (מחליף תרגילים ואנימציות), סאונד 8-ביט, נעילת מסך, שיאים ב-localStorage
