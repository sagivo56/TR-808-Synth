# ---- production deps (better-sqlite3 ships a prebuilt binary; no compiler needed) ----
FROM node:22-bookworm-slim AS deps
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci --omit=dev

# ---- build stage (includes devDependencies for `next build`) ----
FROM node:22-bookworm-slim AS build
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci
COPY . .
ENV NEXT_TELEMETRY_DISABLED=1
RUN npm run build

# ---- runtime ----
FROM node:22-bookworm-slim AS runner
WORKDIR /app
ENV NODE_ENV=production
ENV NEXT_TELEMETRY_DISABLED=1
# SQLite file lives on the persistent Volume (mount it at /data on Railway)
ENV DATABASE_PATH=/data/app.db

# production node_modules (with the prebuilt better-sqlite3 binary)
COPY --from=deps /app/node_modules ./node_modules
# build output + runtime files
COPY --from=build /app/.next ./.next
COPY --from=build /app/public ./public
COPY --from=build /app/next.config.mjs ./next.config.mjs
COPY --from=build /app/package.json ./package.json

RUN mkdir -p /data

EXPOSE 3000
# Railway injects PORT; `next start` honors it and binds 0.0.0.0 by default.
CMD ["npm", "run", "start"]
