# ---- שלב תלויות production (כולל בניית better-sqlite3 הנייטיב) ----
FROM node:20-bookworm-slim AS deps
WORKDIR /app
# כלי בנייה למקרה שצריך לקמפל מודול נייטיב (better-sqlite3)
RUN apt-get update && apt-get install -y --no-install-recommends python3 make g++ \
    && rm -rf /var/lib/apt/lists/*
COPY package.json package-lock.json ./
RUN npm ci --omit=dev

# ---- שלב build (כולל devDependencies לצורך next build) ----
FROM node:20-bookworm-slim AS build
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends python3 make g++ \
    && rm -rf /var/lib/apt/lists/*
COPY package.json package-lock.json ./
RUN npm ci
COPY . .
ENV NEXT_TELEMETRY_DISABLED=1
RUN npm run build

# ---- שלב הרצה ----
FROM node:20-bookworm-slim AS runner
WORKDIR /app
ENV NODE_ENV=production
ENV NEXT_TELEMETRY_DISABLED=1
# ברירת מחדל: קובץ ה-DB יושב על ה-Volume הקבוע שמומלץ למאונט תחת /data
ENV DATABASE_PATH=/data/app.db

# תלויות production בלבד (עם better-sqlite3 המקומפל)
COPY --from=deps /app/node_modules ./node_modules
# תוצרי הבנייה והקבצים הדרושים בזמן ריצה
COPY --from=build /app/.next ./.next
COPY --from=build /app/public ./public
COPY --from=build /app/next.config.mjs ./next.config.mjs
COPY --from=build /app/package.json ./package.json

# יצירת נקודת המאונט (אם ה-Volume לא הוגדר, לפחות תהיה תיקייה מקומית)
RUN mkdir -p /data

EXPOSE 3000
# Railway מזריק PORT; next start מכבד אותו. מאזין על 0.0.0.0 כברירת מחדל.
CMD ["npm", "run", "start"]
