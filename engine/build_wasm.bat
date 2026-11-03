@echo off
setlocal
echo [TypeAssist] Compiling C Engine to WebAssembly...

if defined EMSDK (
  call "%EMSDK%\emsdk_env.bat" >nul 2>&1
) else if exist "C:\Users\arolk\emsdk\emsdk_env.bat" (
  call "C:\Users\arolk\emsdk\emsdk_env.bat" >nul 2>&1
)

cd /d "%~dp0"
if not exist "..\public\wasm" mkdir "..\public\wasm"

call emcc -O3 -Wall -Wextra -Iinclude ^
  src/trie.c src/hashtable.c src/minheap.c src/edit_distance.c ^
  src/ngram.c src/stack.c src/linkedlist.c src/engine_api.c ^
  -s WASM=1 ^
  -s MODULARIZE=1 ^
  -s EXPORT_NAME="createTypeAssistModule" ^
  -s EXPORTED_FUNCTIONS="['_engine_init','_engine_cleanup','_engine_free_string','_engine_load_dictionary','_engine_load_ngrams','_engine_autocomplete','_engine_spellcheck','_engine_predict_next','_engine_word_exists','_engine_add_word','_engine_remove_word','_engine_push_undo','_engine_undo','_engine_add_history','_engine_get_history','_engine_clear_history','_engine_get_stats','_malloc','_free']" ^
  -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8']" ^
  -s ALLOW_MEMORY_GROWTH=1 ^
  -s INITIAL_MEMORY=33554432 ^
  -s ENVIRONMENT="web,worker,node" ^
  -o ../public/wasm/typeassist.js

if %ERRORLEVEL% equ 0 (
  echo [TypeAssist] WebAssembly build succeeded!
  echo Output: public/wasm/typeassist.js, public/wasm/typeassist.wasm
) else (
  echo [TypeAssist] WebAssembly build failed with error code %ERRORLEVEL%
  exit /b %ERRORLEVEL%
)
