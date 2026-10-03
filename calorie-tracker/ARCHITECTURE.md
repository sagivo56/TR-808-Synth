# מבנה הפרויקט ומדריך הרחבה

מסמך קצר שמסביר איפה כל דבר יושב ואיך מוסיפים פיצ'ר חדש בלי לשבור את הקיים.

## מפת התיקיות

```
calorie-tracker/
├── src/
│   ├── app/                      # Next.js App Router
│   │   ├── page.tsx              # מסך ראשי (client)
│   │   ├── layout.tsx            # שלד HTML + PWA metadata
│   │   ├── globals.css           # עיצוב גלובלי (Tailwind)
│   │   └── api/                  # נקודות קצה בצד שרת
│   │       ├── estimate/route.ts # הערכת מאקרו ממנה חופשית (Claude)
│   │       ├── meals/route.ts    # CRUD לארוחות
│   │       └── settings/route.ts # קריאה/שמירה של הגדרות המשתמש
│   ├── components/               # רכיבי React
│   │   ├── Chat.tsx              # צ'אט דיווח האוכל
│   │   ├── MealList.tsx          # רשימת הארוחות של היום
│   │   ├── Meter.tsx             # מד קלוריות/מאקרו
│   │   ├── SetupForm.tsx         # טופס הגדרות ראשוני
│   │   └── ServiceWorkerRegister.tsx
│   ├── lib/                      # לוגיקה שאינה UI
│   │   ├── db/                   # שכבת הנתונים (SQLite) — ראו למטה
│   │   ├── calc.ts               # חישוב יעדי קלוריות ומאקרו
│   │   ├── date.ts               # עזרי תאריך/שעה מקומיים
│   │   ├── user.ts               # זיהוי משתמש דרך cookie
│   │   └── types.ts              # טיפוסים משותפים
│   └── types/                    # הצהרות טיפוסים (ambient)
├── public/                       # נכסים סטטיים, manifest, service worker, icons
└── scripts/                      # סקריפטים נלווים (יצירת icons וכו')
```

## שכבת הנתונים (`src/lib/db/`)

כל גישה למסד הנתונים עוברת דרך התיקייה הזו, ומייבאים אותה תמיד כ-`@/lib/db`.

| קובץ | אחריות |
|------|---------|
| `client.ts` | חיבור יחיד (singleton) ל-SQLite, קובע את נתיב הקובץ ומאתחל סכמה |
| `schema.ts` | הגדרת **כל** הטבלאות והמיגרציות במקום אחד |
| `users.ts`  | שאילתות על פרופיל/הגדרות המשתמש |
| `meals.ts`  | שאילתות על ארוחות |
| `index.ts`  | barrel — מרכז את כל ה-exports לנקודת ייבוא אחת |

## איך מוסיפים פיצ'ר חדש (למשל: צעדים יומיים)

1. **טבלה** — ב-`src/lib/db/schema.ts`, בתוך `createTables`, הוסיפו
   `CREATE TABLE IF NOT EXISTS steps (...)`. אם משנים טבלה קיימת, הוסיפו
   מיגרציה אידמפוטנטית ב-`runMigrations`.
2. **Repository** — צרו `src/lib/db/steps.ts` עם הפונקציות
   (`getSteps`, `setSteps`...), וייבאו בו `getDb` מ-`./client`.
3. **חשיפה** — הוסיפו `export * from "./steps";` ל-`src/lib/db/index.ts`.
4. **טיפוסים** — הוסיפו את ה-interface ל-`src/lib/types.ts`.
5. **API** — צרו `src/app/api/steps/route.ts` (העתיקו את התבנית מ-`meals/route.ts`:
   `resolveUser` לזיהוי, `force-dynamic`, `runtime = "nodejs"`).
6. **UI** — צרו רכיב ב-`src/components/` וחברו אותו ב-`page.tsx`.

כך כל פיצ'ר נשאר עצמאי: קובץ repository, route ו-component משלו — בלי לנפח קובץ קיים.

## פקודות

```bash
npm run dev      # פיתוח מקומי
npm run build    # בדיקת build מלאה (להריץ לפני push)
npm start        # הרצת production מקומית
```
