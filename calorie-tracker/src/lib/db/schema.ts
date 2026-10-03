import type Database from "better-sqlite3";

// כל סכמת מסד הנתונים מרוכזת כאן.
//
// להוספת פיצ'ר חדש (למשל: צעדים, משקל לאורך זמן, אימונים, שתיית מים):
//   1. הוסיפו טבלה חדשה בתוך createTables (עם CREATE TABLE IF NOT EXISTS).
//   2. אם צריך לשנות טבלה קיימת בלי לאבד נתונים — הוסיפו מיגרציה ב-runMigrations.
//   3. צרו קובץ repository ייעודי: src/lib/db/<feature>.ts וחשפו אותו ב-index.ts.

function createTables(db: Database.Database): void {
  db.exec(`
    CREATE TABLE IF NOT EXISTS users (
      id          TEXT PRIMARY KEY,
      height_cm   REAL NOT NULL,
      weight_kg   REAL NOT NULL,
      age         INTEGER NOT NULL,
      sex         TEXT NOT NULL,
      activity    REAL NOT NULL,
      goal        TEXT NOT NULL,
      muscle_goal INTEGER NOT NULL DEFAULT 0,
      target      INTEGER NOT NULL,
      created_at  INTEGER NOT NULL,
      updated_at  INTEGER NOT NULL
    );

    CREATE TABLE IF NOT EXISTS meals (
      id          TEXT PRIMARY KEY,
      user_id     TEXT NOT NULL,
      date        TEXT NOT NULL,
      time        TEXT NOT NULL,
      name        TEXT NOT NULL,
      calories    INTEGER NOT NULL,
      protein_g   REAL NOT NULL,
      carbs_g     REAL NOT NULL,
      fat_g       REAL NOT NULL,
      created_at  INTEGER NOT NULL
    );

    CREATE INDEX IF NOT EXISTS idx_meals_user_date ON meals (user_id, date);
  `);
}

function runMigrations(db: Database.Database): void {
  // muscle_goal נוסף אחרי ההשקה הראשונית — מוסיפים אותו לטבלאות קיימות.
  const userCols = db
    .prepare("PRAGMA table_info(users)")
    .all() as { name: string }[];
  if (!userCols.some((c) => c.name === "muscle_goal")) {
    db.exec(
      "ALTER TABLE users ADD COLUMN muscle_goal INTEGER NOT NULL DEFAULT 0"
    );
  }
}

/** יוצר טבלאות חסרות ומריץ מיגרציות. אידמפוטנטי — בטוח לקרוא בכל עלייה. */
export function initSchema(db: Database.Database): void {
  createTables(db);
  runMigrations(db);
}
