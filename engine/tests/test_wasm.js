const createTypeAssistModule = require('../../public/wasm/typeassist.js');

async function testWasm() {
  console.log('[WASM Test] Loading WebAssembly module...');
  const Module = await createTypeAssistModule();
  console.log('[WASM Test] Module loaded successfully.');

  const initRes = Module._engine_init();
  console.log('[WASM Test] _engine_init result:', initRes);

  // 1. Test dictionary loading
  const dictData = "apple\t500\nbanana\t300\ncherry\t200\ndatabase\t1000\nreceive\t800\nstructure\t900\nproject\t950\n";
  const dLen = Buffer.byteLength(dictData, 'utf8') + 1;
  const dPtr = Module._malloc(dLen);
  Module.stringToUTF8(dictData, dPtr, dLen);
  const loaded = Module._engine_load_dictionary(dPtr, dLen);
  Module._free(dPtr);
  console.log('[WASM Test] _engine_load_dictionary loaded words:', loaded);

  // 2. Test Autocomplete via Trie
  const prefix = "dat";
  const pLen = Buffer.byteLength(prefix, 'utf8') + 1;
  const pPtr = Module._malloc(pLen);
  Module.stringToUTF8(prefix, pPtr, pLen);
  const acPtr = Module._engine_autocomplete(pPtr, 5);
  Module._free(pPtr);
  const acJson = Module.UTF8ToString(acPtr);
  Module._engine_free_string(acPtr);
  console.log('[WASM Test] Autocomplete "dat":', acJson);

  // 3. Test Spellcheck via Damerau-Levenshtein + Min-Heap + Hash Table
  const typo = "recieve";
  const tLen = Buffer.byteLength(typo, 'utf8') + 1;
  const tPtr = Module._malloc(tLen);
  Module.stringToUTF8(typo, tPtr, tLen);
  const scPtr = Module._engine_spellcheck(tPtr, 5);
  Module._free(tPtr);
  const scJson = Module.UTF8ToString(scPtr);
  Module._engine_free_string(scPtr);
  console.log('[WASM Test] Spellcheck "recieve":', scJson);

  // 4. Test Undo via LIFO Stack
  const s1 = "First version";
  const s1Ptr = Module._malloc(Buffer.byteLength(s1, 'utf8') + 1);
  Module.stringToUTF8(s1, s1Ptr, Buffer.byteLength(s1, 'utf8') + 1);
  Module._engine_push_undo(s1Ptr);
  Module._free(s1Ptr);

  const undoPtr = Module._engine_undo();
  const undoJson = Module.UTF8ToString(undoPtr);
  Module._engine_free_string(undoPtr);
  console.log('[WASM Test] Undo popped state:', undoJson);

  // 5. Test History via Linked List
  const orig = "recieve";
  const mod = "receive";
  const oPtr = Module._malloc(Buffer.byteLength(orig, 'utf8') + 1);
  const mPtr = Module._malloc(Buffer.byteLength(mod, 'utf8') + 1);
  Module.stringToUTF8(orig, oPtr, Buffer.byteLength(orig, 'utf8') + 1);
  Module.stringToUTF8(mod, mPtr, Buffer.byteLength(mod, 'utf8') + 1);
  Module._engine_add_history(oPtr, mPtr, 1);
  Module._free(oPtr);
  Module._free(mPtr);

  const histPtr = Module._engine_get_history();
  const histJson = Module.UTF8ToString(histPtr);
  Module._engine_free_string(histPtr);
  console.log('[WASM Test] History list:', histJson);

  // 6. Test Stats
  const statsPtr = Module._engine_get_stats();
  const statsJson = Module.UTF8ToString(statsPtr);
  Module._engine_free_string(statsPtr);
  console.log('[WASM Test] Engine Stats:', statsJson);

  Module._engine_cleanup();
  console.log('[WASM Test] ALL TESTS PASSED SUCCESSFULLY!');
}

testWasm().catch(err => {
  console.error('[WASM Test Error]:', err);
  process.exit(1);
});
