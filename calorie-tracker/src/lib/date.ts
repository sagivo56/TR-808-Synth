// עזרי תאריך/שעה מקומיים (בצד לקוח) - כדי שהיומן יעבוד לפי שעון המשתמש
// ולא לפי אזור הזמן של השרת.

export function localDateISO(d: Date = new Date()): string {
  const off = d.getTimezoneOffset() * 60000;
  return new Date(d.getTime() - off).toISOString().slice(0, 10);
}

export function localTimeHM(d: Date = new Date()): string {
  return `${String(d.getHours()).padStart(2, "0")}:${String(
    d.getMinutes()
  ).padStart(2, "0")}`;
}

export function shiftDateISO(iso: string, days: number): string {
  const [y, m, dd] = iso.split("-").map(Number);
  const d = new Date(y, m - 1, dd);
  d.setDate(d.getDate() + days);
  return localDateISO(d);
}

// האם תאריך+שעה נתונים הם בעתיד (לפי השעון המקומי)
export function isFutureLocal(dateISO: string, timeHM: string): boolean {
  return new Date(`${dateISO}T${timeHM}`).getTime() > Date.now();
}

export function dayLabel(iso: string): string {
  const today = localDateISO();
  if (iso === today) return "היום";
  if (iso === shiftDateISO(today, -1)) return "אתמול";
  const [y, m, d] = iso.split("-");
  return `${d}/${m}/${y.slice(2)}`;
}
