#!/bin/sh

cd en
mkdocs build
rm -rf ../../../wui_site/doc
mv site ../../../wui_site/doc

cd ../ru
mkdocs build
rm -rf ../../../wui_site/doc_ru
mv site ../../../wui_site/doc_ru
