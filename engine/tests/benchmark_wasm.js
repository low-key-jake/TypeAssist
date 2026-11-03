const fs = require('fs');
const path = require('path');
const createTypeAssistModule = require(path.join(__dirname, '../../public/wasm/typeassist.js'));

async function testFullDict() {
  const Module = await createTypeAssistModule();
  Module._engine_init();

  const dictData = fs.readFileSync(path.join(__dirname, '../../public/data/dictionary.txt'), 'utf8');
  console.time('WASM Dictionary Load');
  const dLen = Buffer.byteLength(dictData, 'utf8') + 1;
  const dPtr = Module._malloc(dLen);
  Module.stringToUTF8(dictData, dPtr, dLen);
  const loaded = Module._engine_load_dictionary(dPtr, dLen);
  Module._free(dPtr);
  console.timeEnd('WASM Dictionary Load');
  console.log('Words loaded into C Trie & HashTable:', loaded);

  // Load N-grams
  const biData = fs.readFileSync(path.join(__dirname, '../../public/data/bigrams.txt'), 'utf8');
  const bLen = Buffer.byteLength(biData, 'utf8') + 1;
  const bPtr = Module._malloc(bLen);
  Module.stringToUTF8(biData, bPtr, bLen);
  const biCount = Module._engine_load_ngrams(bPtr, bLen, 2);
  Module._free(bPtr);
  console.log('Bigrams loaded into C N-Gram Model:', biCount);

  // Test Trie Autocomplete in C
  const p = Module._malloc(10);
  Module.stringToUTF8('str', p, 10);
  const acPtr = Module._engine_autocomplete(p, 5);
  Module._free(p);
  console.log('C Trie Autocomplete for "str":', Module.UTF8ToString(acPtr));
  Module._engine_free_string(acPtr);

  // Test Spellcheck in C
  const typoPtr = Module._malloc(20);
  Module.stringToUTF8('definitiely', typoPtr, 20);
  const scPtr = Module._engine_spellcheck(typoPtr, 4);
  Module._free(typoPtr);
  console.log('C Spellcheck for "definitiely":', Module.UTF8ToString(scPtr));
  Module._engine_free_string(scPtr);

  // Test N-Gram Prediction in C
  const ctxPtr = Module._malloc(20);
  Module.stringToUTF8('i want', ctxPtr, 20);
  const npPtr = Module._engine_predict_next(ctxPtr, 4);
  Module._free(ctxPtr);
  console.log('C N-Gram Next-Word Prediction for "i want":', Module.UTF8ToString(npPtr));
  Module._engine_free_string(npPtr);

  // Test Stats in C
  const statsPtr = Module._engine_get_stats();
  console.log('C Engine Stats:', Module.UTF8ToString(statsPtr));
  Module._engine_free_string(statsPtr);

  Module._engine_cleanup();
}
testFullDict();
