#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "gtrie_io.h"

GTrieDB* gtrie_db_create(const char* path, size_t db_size, int* err) {
    if (!path || !err) {
        if (err) *err = EINVAL;
        return NULL;
    }
    
    GTrieDB* db = calloc(1, sizeof(GTrieDB));
    if (!db) {
        *err = ENOMEM;
        return NULL;
    }
    
    db->trie = gtrie_create(err);
    if (!db->trie) {
        free(db);
        return NULL;
    }
    
    db->db_path = strdup(path);
    if (!db->db_path) {
        *err = ENOMEM;
        gtrie_destroy(db->trie);
        free(db);
        return NULL;
    }
    
    db->db_size = db_size;
    *err = 0;
    return db;
}

int gtrie_db_destroy(GTrieDB* db) {
    if (!db) return EINVAL;
    
    if (db->env) {
        gtrie_db_close(db);
    }
    
    if (db->trie) {
        gtrie_destroy(db->trie);
    }
    
    free((void*)db->db_path);
    free(db);
    return 0;
}

int gtrie_db_open(GTrieDB* db) {
    if (!db) return EINVAL;
    
    int rc = mdb_env_create(&db->env);
    if (rc != 0) return rc;
    
    rc = mdb_env_set_mapsize(db->env, db->db_size);
    if (rc != 0) {
        mdb_env_close(db->env);
        return rc;
    }
    
    rc = mdb_env_open(db->env, db->db_path, 0, 0664);
    if (rc != 0) {
        mdb_env_close(db->env);
        return rc;
    }
    
    MDB_txn* txn;
    rc = mdb_txn_begin(db->env, NULL, 0, &txn);
    if (rc != 0) {
        mdb_env_close(db->env);
        return rc;
    }
    
    rc = mdb_dbi_open(txn, NULL, 0, &db->dbi);
    if (rc != 0) {
        mdb_txn_abort(txn);
        mdb_env_close(db->env);
        return rc;
    }
    
    return mdb_txn_commit(txn);
}

int gtrie_db_close(GTrieDB* db) {
    if (!db || !db->env) return EINVAL;
    
    mdb_dbi_close(db->env, db->dbi);
    mdb_env_close(db->env);
    db->env = NULL;
    
    return 0;
}

int gtrie_db_write(GTrieDB* db, const char* word, PostingList* postings) {
    if (!db || !db->env || !word || !postings) return EINVAL;
    
    MDB_txn* txn;
    int rc = mdb_txn_begin(db->env, NULL, 0, &txn);
    if (rc != 0) return rc;
    
    MDB_val key, data;
    key.mv_size = strlen(word) + 1;
    key.mv_data = (void*)word;
    
    // Serialize posting list
    size_t data_size = 0;
    PostingEntry* entry = postings->head;
    while (entry) {
        data_size += strlen(entry->doc_id) + 1;
        entry = entry->next;
    }
    
    char* serialized = malloc(data_size);
    if (!serialized) {
        mdb_txn_abort(txn);
        return ENOMEM;
    }
    
    char* ptr = serialized;
    entry = postings->head;
    while (entry) {
        strcpy(ptr, entry->doc_id);
        ptr += strlen(entry->doc_id) + 1;
        entry = entry->next;
    }
    
    data.mv_size = data_size;
    data.mv_data = serialized;
    
    rc = mdb_put(txn, db->dbi, &key, &data, 0);
    free(serialized);
    
    if (rc != 0) {
        mdb_txn_abort(txn);
        return rc;
    }
    
    return mdb_txn_commit(txn);
}

PostingList* gtrie_db_read(const GTrieDB* db, const char* word, int* err) {
    if (!db || !db->env || !word) {
        if (err) *err = EINVAL;
        return NULL;
    }
    
    MDB_txn* txn;
    int rc = mdb_txn_begin(db->env, NULL, MDB_RDONLY, &txn);
    if (rc != 0) {
        if (err) *err = rc;
        return NULL;
    }
    
    MDB_val key, data;
    key.mv_size = strlen(word) + 1;
    key.mv_data = (void*)word;
    
    rc = mdb_get(txn, db->dbi, &key, &data);
    if (rc != 0) {
        mdb_txn_abort(txn);
        if (err) *err = rc;
        return NULL;
    }
    
    PostingList* list = malloc(sizeof(PostingList));
    if (!list) {
        mdb_txn_abort(txn);
        if (err) *err = ENOMEM;
        return NULL;
    }
    list->head = NULL;
    
    char* ptr = data.mv_data;
    char* end = ptr + data.mv_size;
    
    while (ptr < end) {
        PostingEntry* entry = malloc(sizeof(PostingEntry));
        if (!entry) {
            // Clean up on error
            while (list->head) {
                PostingEntry* temp = list->head;
                list->head = list->head->next;
                free(temp->doc_id);
                free(temp);
            }
            free(list);
            mdb_txn_abort(txn);
            if (err) *err = ENOMEM;
            return NULL;
        }
        
        entry->doc_id = strdup(ptr);
        entry->next = list->head;
        list->head = entry;
        
        ptr += strlen(ptr) + 1;
    }
    
    mdb_txn_abort(txn);
    if (err) *err = 0;
    return list;
}

int gtrie_db_sync(GTrieDB* db) {
    if (!db || !db->env) return EINVAL;
    return mdb_env_sync(db->env, 1);
}

int gtrie_db_save_trie(GTrieDB* db) {
    // TODO: Implement traversal of trie and save to DB
    return ENOSYS;
}

int gtrie_db_load_trie(GTrieDB* db) {
    if (!db || !db->env) return EINVAL;
    
    MDB_txn* txn;
    int rc = mdb_txn_begin(db->env, NULL, MDB_RDONLY, &txn);
    if (rc != 0) return rc;
    
    MDB_cursor* cursor;
    rc = mdb_cursor_open(txn, db->dbi, &cursor);
    if (rc != 0) {
        mdb_txn_abort(txn);
        return rc;
    }
    
    MDB_val key, data;
    while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
        char* word = key.mv_data;
        char* ptr = data.mv_data;
        char* end = ptr + data.mv_size;
        
        while (ptr < end) {
            rc = gtrie_insert(db->trie, word, ptr);
            if (rc != 0) {
                mdb_cursor_close(cursor);
                mdb_txn_abort(txn);
                return rc;
            }
            ptr += strlen(ptr) + 1;
        }
    }
    
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);
    
    return (rc == MDB_NOTFOUND) ? 0 : rc;
}
