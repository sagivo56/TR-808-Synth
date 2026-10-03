"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";

// ניווט טאבים עליון, משותף לכל המסכים. מודגש לפי הנתיב הנוכחי.

const TABS = [
  { href: "/", label: "יומן", icon: "🍽️" },
  { href: "/workout", label: "אימונים", icon: "💪" },
] as const;

export default function TabBar() {
  const pathname = usePathname();

  return (
    <nav className="sticky top-0 z-30 bg-bg/90 backdrop-blur border-b border-border">
      <div className="max-w-xl mx-auto px-4 py-2">
        <div className="grid grid-cols-2 gap-1 rounded-xl border border-border bg-panel p-1">
          {TABS.map((t) => {
            const active =
              t.href === "/" ? pathname === "/" : pathname.startsWith(t.href);
            return (
              <Link
                key={t.href}
                href={t.href}
                aria-current={active ? "page" : undefined}
                className={`flex items-center justify-center gap-2 rounded-lg py-2 text-sm font-display font-bold transition-colors ${
                  active
                    ? "bg-bg text-text-main shadow-inner"
                    : "text-text-muted hover:text-text-main"
                }`}
              >
                <span aria-hidden>{t.icon}</span>
                <span>{t.label}</span>
              </Link>
            );
          })}
        </div>
      </div>
    </nav>
  );
}
