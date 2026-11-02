CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -Iinclude
EMCC ?= emcc

SRCS = src/trie.c src/hashtable.c src/minheap.c src/edit_distance.c \
       src/ngram.c src/stack.c src/linkedlist.c src/engine_api.c

TEST_DIR = tests
BUILD_DIR = build
OUT_WASM_DIR = ../public/wasm

all: test_bins

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test_bins: $(BUILD_DIR)
	$(CC) $(CFLAGS) $(TEST_DIR)/test_trie.c src/trie.c -o $(BUILD_DIR)/test_trie
	$(CC) $(CFLAGS) $(TEST_DIR)/test_hashtable.c src/hashtable.c -o $(BUILD_DIR)/test_hashtable
	$(CC) $(CFLAGS) $(TEST_DIR)/test_minheap.c src/minheap.c -o $(BUILD_DIR)/test_minheap
	$(CC) $(CFLAGS) $(TEST_DIR)/test_edit_distance.c src/edit_distance.c -o $(BUILD_DIR)/test_edit_distance
	$(CC) $(CFLAGS) $(TEST_DIR)/test_ngram.c src/ngram.c src/hashtable.c -o $(BUILD_DIR)/test_ngram
	$(CC) $(CFLAGS) $(TEST_DIR)/test_stack.c src/stack.c -o $(BUILD_DIR)/test_stack
	$(CC) $(CFLAGS) $(TEST_DIR)/test_linkedlist.c src/linkedlist.c -o $(BUILD_DIR)/test_linkedlist
	$(CC) $(CFLAGS) $(TEST_DIR)/test_benchmark.c src/trie.c src/hashtable.c src/minheap.c src/edit_distance.c src/ngram.c -o $(BUILD_DIR)/test_benchmark

test: test_bins
	./$(BUILD_DIR)/test_trie
	./$(BUILD_DIR)/test_hashtable
	./$(BUILD_DIR)/test_minheap
	./$(BUILD_DIR)/test_edit_distance
	./$(BUILD_DIR)/test_ngram
	./$(BUILD_DIR)/test_stack
	./$(BUILD_DIR)/test_linkedlist

benchmark: test_bins
	./$(BUILD_DIR)/test_benchmark

wasm:
	mkdir -p $(OUT_WASM_DIR)
	$(EMCC) -O3 $(CFLAGS) $(SRCS) \
		-s WASM=1 \
		-s MODULARIZE=1 \
		-s EXPORT_NAME="createTypeAssistModule" \
		-s EXPORTED_FUNCTIONS="['_engine_init','_engine_cleanup','_engine_free_string','_engine_load_dictionary','_engine_load_ngrams','_engine_autocomplete','_engine_spellcheck','_engine_predict_next','_engine_word_exists','_engine_add_word','_engine_remove_word','_engine_push_undo','_engine_undo','_engine_add_history','_engine_get_history','_engine_clear_history','_engine_get_stats','_malloc','_free']" \
		-s EXPORTED_RUNTIME_METHODS="['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8']" \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s INITIAL_MEMORY=33554432 \
		-s ENVIRONMENT="web,worker" \
		-o $(OUT_WASM_DIR)/typeassist.js

clean:
	rm -rf $(BUILD_DIR)

