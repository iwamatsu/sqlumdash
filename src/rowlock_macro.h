/*
** SPDX-License-Identifier: MIT
**
** Copyright (c) 2018 Toshiba Corporation
**
*************************************************************************
** This file contains macros for row lock feature in order to reduce the
** amount of change for SQLite original source files.
*/
#ifndef SQLITE_OMIT_ROWLOCK

#ifdef SQLITE_OMIT_SHARED_CACHE
#error rowlock requires to enable shared cache
#endif


/* Macro for btree.h */
/* The flags parameter to sqlite3BtreeOpen */
#define BTREE_TRANS         16 /* This is a transaction DB */



/* Macro for btree.c */
#define ROWLOCK_TRANS_BTREE_COMMIT(p) \
  do { \
    int rc2 = sqlite3TransBtreeCommit(p); \
    if( rc2!=SQLITE_OK ){ \
      sqlite3BtreeLeave(p); \
      return rc2; \
    } \
  } while(0)

#define ROWLOCK_TRANS_BTREE_CURSOR_CLOSE(pCur) \
  sqlite3TransBtreeCloseCursor(pCur)


/* Macro for alter.c and build.c */
#define ROWLOCK_TABLE_LOCK() \
  sqlite3TableLock(pParse, iDb, pTab->tnum, 2, pTab->zName)

/* Macro for build.c */
#define ROWLOCK_TABLE_LOCK_FOR_INDEX() \
  sqlite3TableLock(pParse, iDb, pIndex->pTable->tnum, 2, pIndex->pTable->zName)

/* Macro for trigger.c */
#define ROWLOCK_TABLE_LOCK_FOR_TRIGGER() \
  sqlite3TableLock(pParse, iDb, pTable->tnum, 2, pTable->zName)

/* Macro for vdbe.c */
#define ROWLOCK_SET_FORCE_COMMIT(p) \
  sqlite3SetForceCommit(p)

#define ROWLOCK_ENABLE_STMT_JOURNAL() \
  do { \
    /*
    ** There is possible that statement journal is not used for a
    ** write-transaction. So we force to use statement journal
    ** because statement rollback is required when record lock is
    ** occurred.
    */ \
    if( pOp->p2 ){ \
      p->usesStmtJournal = 1; \
    } \
  } while(0)

#define ROWLOCK_CACHED_ROWID_FLAG_SET() \
  sqlite3BtreeCachedRowidFlagSet(pC->uc.pCursor, 1)

#define ROWLOCK_CACHED_ROWID_SET() \
  sqlite3BtreeCachedRowidSet(pC->uc.pCursor, v)

#define ROWLOCK_CACHED_ROWID_SET_BY_OPEN_CURSOR() \
  do { \
    rc = sqlite3BtreeCachedRowidSetByOpenCursor(pCur->uc.pCursor); \
    if( rc ) goto abort_due_to_error; \
  } while(0)

#define ROWLOCK_CACHED_ROWID_UPDATE() \
  do { \
    if( sqlite3BtreeCachedRowidGet(pC->uc.pCursor) != 0 && \
        sqlite3BtreeCachedRowidGet(pC->uc.pCursor) <= x.nKey ){ \
      sqlite3BtreeCachedRowidSet(pC->uc.pCursor, x.nKey); \
    } \
  } while(0)

#define ROWLOCK_CACHED_ROWID_NEW_ROWID() \
  do { \
      v = sqlite3BtreeCachedRowidGet(pC->uc.pCursor); \
      if( v==MAX_ROWID ){ \
        pC->useRandomRowid = 1; \
      }else if( v!=0 ){ \
        v++; \
      }else{ \
        rc = sqlite3BtreeLastAll(pC->uc.pCursor, &res); \
        if( rc!=SQLITE_OK ){ \
          goto abort_due_to_error; \
        } \
        if( res ){ \
          v = 1; \
        }else{ \
          assert( sqlite3BtreeCursorIsValid(pC->uc.pCursor) ); \
          v = sqlite3BtreeIntegerKey(pC->uc.pCursor); \
          if( v==MAX_ROWID ){ \
            pC->useRandomRowid = 1; \
          }else{ \
            v++; \
          } \
        } \
      } \
  } while(0)

/* Macro for vdbeapi.c */
#define SQLITE_MAX_ROWID_RETRY 50
/* Retry the query execution if rowid generated by engine automatically 
** is corrupted.
*/
#define ROWLOCK_CACHED_ROWID_RETRY \
static int sqlite3StepOriginal(Vdbe *p); \
static int sqlite3Step(Vdbe *p){ \
  int rc = SQLITE_OK; \
  int cnt = 0; \
  while( (rc = sqlite3StepOriginal(p))==SQLITE_CORRUPT_ROWID \
         && cnt++ < SQLITE_MAX_ROWID_RETRY ){ \
    sqlite3_reset((sqlite3_stmt*)p); \
  } \
  return rc; \
}

/* Macro for vdbeaux.c */
#define ROWLOCK_VDBE_HALT(p) \
  do { \
    /* Release lock of tables for preventing COMMIT during statemet execution. */ \
    sqlite3BtreeUnlockStmtTableLock(p->db); \
    /* Close savepoints. OP_AutoCommit is not operated for force commit by DDL.
    ** So we do it here.
    */ \
    if( p->forceCommit ) sqlite3CloseSavepoints(p->db); \
  } while(0)

/* Macro for main.c */
#define ROWLOCK_INIT \
  do { \
    rc = rowlockInitialize(); \
    if( rc ) return rc; \
  } while(0)

#define ROWLOCK_MMAP_CONFIG \
    case SQLITE_CONFIG_MMAP_ROW_SIZE: { \
      sqlite3GlobalConfig.szMmapRowLock = va_arg(ap, sqlite3_uint64); \
      break; \
    } \
    case SQLITE_CONFIG_MMAP_TABLE_SIZE: { \
      sqlite3GlobalConfig.szMmapTableLock = va_arg(ap, sqlite3_uint64); \
      break; \
    }

/* Macro for global.c */
#define ROWLOCK_DEFAULT_MMAP_ROW_SIZE    (1024 * 1024)
#define ROWLOCK_DEFAULT_MMAP_TABLE_SIZE  (128 * 1024)

/* Macro for pager.c */
#define ROWLOCK_WAIT_ON_EXCLUSIVE_LOCK() \
  if (locktype==EXCLUSIVE_LOCK) return SQLITE_OK


#define ROWLOCK_SKIP_GET_EXCLUSIVE_LOCK() \
  exFlag = 0

#endif