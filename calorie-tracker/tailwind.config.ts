import type { Config } from "tailwindcss";

const config: Config = {
  content: [
    "./src/app/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/components/**/*.{js,ts,jsx,tsx,mdx}",
  ],
  theme: {
    extend: {
      colors: {
        bg: "#14171A",
        panel: "#1E2226",
        border: "#2A2F34",
        "border-2": "#3A4046",
        "text-main": "#EDEDE6",
        "text-muted": "#8A8F94",
        "meter-green": "#8FBF6B",
        "meter-amber": "#E0A94C",
        "meter-red": "#D9634B",
      },
      fontFamily: {
        display: ["var(--font-space-grotesk)", "sans-serif"],
        sans: ["var(--font-inter)", "sans-serif"],
        mono: ["var(--font-plex-mono)", "monospace"],
      },
    },
  },
  plugins: [],
};

export default config;
