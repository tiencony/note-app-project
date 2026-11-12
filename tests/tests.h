#pragma once

void testTags();
void testLinks();
void testCrud();
void testBacklinks();
void testBrokenLinks();
void testLinkIndexFresh();
void testBrokenLinksDedup();
void cleanupTestFiles();
void testReplaceLink();
void testRenameUpdatesLinks();



void testLoadReport();
void testLoadFailureKeepsMemory();
void testCreateErrors();
void testRenameErrors();
void testOpenNote();

void testListNotes();







void testValidateNewTitle();
void testRenameErrors();
void testRenameSelfLink();
void testRenameRefreshesCache();




void testEditLeavesNoTmp();
void testRenameReportsUnreadable();
void testLoadSkipsBadNames();
void testControlCharTitle();
void testRefreshFromDisk();
