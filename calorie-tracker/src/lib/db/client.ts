import Database from "better-sqlite3";
import fs from "node:fs";
import path from "node:path";
import { initSchema } from "./schema";

// חיבור יחיד (singleton) למסד הנתונים SQLite.
// הנתיב נקבע ע"י DATABASE_PATH (ב-production על ה-Volume המתמשך),
// אחרת data/app.db מקומי. הסכמה מאותחלת פעם אחת בעת החיבור.

let db: Database.Database | null = null;

export function getDb(): Database.Database {
  if (db) return db;

  const dbPath =
    process.env.DATABASE_PATH || path.join(process.cwd(), "data", "app.db");
  fs.mkdirSync(path.dirname(dbPath), { recursive: true });

  db = new Database(dbPath);
  db.pragma("journal_mode = WAL");

  initSchema(db);
  return db;
}
