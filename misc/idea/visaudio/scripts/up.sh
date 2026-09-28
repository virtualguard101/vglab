#!/usr/bin/env bash
# 生产构建, 与 Vercel 的 Build Command 相同.
# 只生成 dist/ 然后退出, 不启动服务器.
# Node.js 与 pnpm 由环境提供: 本地开发者自行安装, Vercel 构建镜像自带.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

if ! command -v node >/dev/null 2>&1; then
  echo "visaudio: 需要 Node.js >= 20" >&2
  exit 1
fi
if ! command -v pnpm >/dev/null 2>&1; then
  echo "visaudio: 需要 pnpm (https://pnpm.io/installation)" >&2
  exit 1
fi

major="$(node -p 'Number(process.versions.node.split(".")[0])')"
if [[ "$major" -lt 20 ]]; then
  echo "visaudio: Node $(node -v) 太旧, 需要 >= 20" >&2
  exit 1
fi

if [[ ! -d node_modules ]]; then
  pnpm install --frozen-lockfile
fi

pnpm build
