# פריסה ל-Railway (כתובת HTTPS קבועה)

הפרויקט מוכן לפריסה עם ה-`Dockerfile` וקובץ `railway.json` שבתיקייה זו.
ה-SQLite נשמר על **Volume** קבוע כדי שהנתונים ישרדו בין הפעלות מחדש.

> ⚠️ הפרויקט יושב בתת-תיקייה `calorie-tracker/` בתוך ה-repo. חובה להגדיר
> **Root Directory = `calorie-tracker`** בשירות, אחרת Railway ינסה לבנות את
> פרויקט ה-TR-808 שבשורש ויקרוס.

## שלבים

1. היכנס ל-https://railway.com והתחבר עם **GitHub**.
2. **New Project → Deploy from GitHub repo** ובחר `sagivo56/TR-808-Synth`.
3. פתח את השירות שנוצר → **Settings**:
   - **Root Directory**: `calorie-tracker`
   - **Build**: יזוהה אוטומטית כ-Dockerfile (בזכות `railway.json`).
4. **Variables** — הוסף משתנה סביבה:
   - `ANTHROPIC_API_KEY` = המפתח שלך מ-https://console.anthropic.com/settings/keys
   - (אופציונלי) `ANTHROPIC_MODEL` = `claude-opus-4-8` (ברירת המחדל).
5. **Volume** — הוסף Volume לשירות (Settings → Volumes / "Add Volume"):
   - **Mount Path**: `/data`
   - ה-`Dockerfile` כבר מגדיר `DATABASE_PATH=/data/app.db`, כך שה-DB יישמר שם.
6. **Networking** — לחץ **Generate Domain**. תקבל כתובת HTTPS כמו
   `https://<name>.up.railway.app`.
7. פתח את הכתובת בטלפון → **הוספה למסך הבית** (Safari: שיתוף → הוספה למסך הבית;
   Chrome: תפריט ⋮ → התקן אפליקציה).

## הערות

- Railway מזריק את המשתנה `PORT` אוטומטית; `next start` מכבד אותו — אין צורך
  להגדיר ידנית.
- אם לא מוגדר Volume, האפליקציה עדיין תרוץ אבל הנתונים יימחקו בכל דיפלוי/ריסטארט.
- בלי `ANTHROPIC_API_KEY` תקין הכל עובד חוץ מהערכת הקלוריות בצ'אט, שתחזיר שגיאה
  ברורה.

## חלופות

- **Fly.io**: `fly launch` בתוך `calorie-tracker/` (מזהה Dockerfile), הוסף Volume
  ומאונט ל-`/data`, והגדר `ANTHROPIC_API_KEY` עם `fly secrets set`.
- **Render**: New → Web Service → Docker, Root Directory `calorie-tracker`, הוסף
  Disk עם Mount Path `/data`, והגדר את משתני הסביבה.
