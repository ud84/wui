#!/bin/sh

cd en
mkdocs build
rm -rf ../../../wui_site/doc
mv site ../../../wui_site/doc
mkdir -p ../../../wui_site/doc/img
cp img/* ../../../wui_site/doc/img

cd ../ru
mkdocs build
rm -rf ../../../wui_site/doc_ru
mv site ../../../wui_site/doc_ru
mkdir -p ../../../wui_site/doc_ru/img
cp img/* ../../../wui_site/doc_ru/img