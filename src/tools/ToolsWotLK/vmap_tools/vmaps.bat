@echo off
mkdir vmaps
vmap4_extractor.exe
vmap4_assembler Buildings vmaps
pause
