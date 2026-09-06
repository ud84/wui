#!/usr/bin/env bash
# Build both languages before uploading; only /doc/ and /doc_ru/ are touched.
set -euo pipefail
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PYTHON_BIN=${PYTHON_BIN:-python3}
AWS_BIN=${AWS_BIN:-aws}
BUCKET=${WUI_SITE_BUCKET:-libwui.org}
ENDPOINT=${S3_ENDPOINT_URL:-https://storage.yandexcloud.net}
REGION=${AWS_DEFAULT_REGION:-ru-central1}
DRY_RUN=false
case "${1:-}" in
    --dry-run) DRY_RUN=true ;;
    '') ;;
    *) echo 'Usage: doc/deploy.sh [--dry-run]' >&2; exit 2 ;;
esac
[[ $# -le 1 && -n "$BUCKET" && "$BUCKET" != */* ]] || exit 2
command -v "$PYTHON_BIN" >/dev/null
command -v "$AWS_BIN" >/dev/null
DOCS_TMP=$(mktemp -d "${TMPDIR:-/tmp}/wui-docs-deploy.XXXXXX")
trap 'rm -rf "$DOCS_TMP"' EXIT
"$PYTHON_BIN" "$SCRIPT_DIR/build.py" --output "$DOCS_TMP/site"
export AWS_REQUEST_CHECKSUM_CALCULATION=WHEN_REQUIRED
export AWS_RESPONSE_CHECKSUM_VALIDATION=WHEN_REQUIRED
AWS=("$AWS_BIN" --endpoint-url="$ENDPOINT" --region "$REGION")
OPTIONS=(--no-progress --no-follow-symlinks --cache-control no-cache)
if "$DRY_RUN"; then OPTIONS+=(--dryrun); fi
for prefix in doc doc_ru; do
    "${AWS[@]}" s3 cp "$DOCS_TMP/site/$prefix/" "s3://$BUCKET/$prefix/" --recursive \
        "${OPTIONS[@]}" --exclude '*.html'
    "${AWS[@]}" s3 cp "$DOCS_TMP/site/$prefix/" "s3://$BUCKET/$prefix/" --recursive \
        "${OPTIONS[@]}" --exclude '*' --include '*.html' --content-type 'text/html; charset=utf-8'
done
if "$DRY_RUN"; then echo 'Dry run complete; nothing uploaded.'; exit 0; fi
for prefix in doc doc_ru; do
    for key in index.html controls/button/index.html controls/text/index.html controls/scroll/index.html search/search_index.json sitemap.xml; do
        "${AWS[@]}" s3 cp "s3://$BUCKET/$prefix/$key" "$DOCS_TMP/verify" --no-progress --only-show-errors
        cmp "$DOCS_TMP/site/$prefix/$key" "$DOCS_TMP/verify"
        printf 'Verified: %s/%s\n' "$prefix" "$key"
    done
done
printf 'Published documentation to https://%s/doc/ and /doc_ru/\n' "$BUCKET"
