cd en
mkdocs build
del /S /Q ..\..\..\sites\wui_site\doc
xcopy /E /Y site ..\..\..\\sites\wui_site\doc
del /S /Q site
rmdir /S /Q site
mkdir ..\..\..\sites\wui_site\doc\img
copy img\* ..\..\..\sites\wui_site\doc\img

cd ..\ru
mkdocs build
del /S /Q ..\..\..\sites\wui_site\doc_ru
xcopy /E /Y site ..\..\..\sites\wui_site\doc_ru
del /S /Q site
rmdir /S /Q site
mkdir ..\..\..\sites\wui_site\doc_ru\img
copy img\* ..\..\..\sites\wui_site\doc_ru\img

pause
