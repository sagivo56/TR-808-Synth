"use client";

import { useEffect, useRef } from "react";

// מחזיק נעילת מסך (Screen Wake Lock API) כל עוד active=true, כדי שהמסך
// לא יכבה באמצע אימון. משחרר אוטומטית כשלא פעיל, ומבקש מחדש כשחוזרים ללשונית.

interface WakeLockSentinelLike {
  released: boolean;
  release: () => Promise<void>;
}

type WakeLockNavigator = Navigator & {
  wakeLock?: { request: (type: "screen") => Promise<WakeLockSentinelLike> };
};

export function useWakeLock(active: boolean): void {
  const sentinelRef = useRef<WakeLockSentinelLike | null>(null);

  useEffect(() => {
    if (!active) return;
    let cancelled = false;

    const request = async () => {
      const nav = navigator as WakeLockNavigator;
      if (!nav.wakeLock) return;
      try {
        const sentinel = await nav.wakeLock.request("screen");
        if (cancelled) {
          sentinel.release().catch(() => {});
          return;
        }
        sentinelRef.current = sentinel;
      } catch {
        /* המשתמש/הדפדפן סירב - ממשיכים בלי נעילה */
      }
    };

    const onVisibility = () => {
      if (document.visibilityState === "visible") request();
    };

    request();
    document.addEventListener("visibilitychange", onVisibility);

    return () => {
      cancelled = true;
      document.removeEventListener("visibilitychange", onVisibility);
      const s = sentinelRef.current;
      sentinelRef.current = null;
      if (s && !s.released) s.release().catch(() => {});
    };
  }, [active]);
}
