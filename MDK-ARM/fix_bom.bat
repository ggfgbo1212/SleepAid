@echo off
rem fix_bom.bat - ensure UTF-8 BOM on GUI Guider Chinese sources before build
rem (calls fix_bom.ps1; keep this file ASCII-only, cmd parses it in ANSI/GBK)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0fix_bom.ps1"
