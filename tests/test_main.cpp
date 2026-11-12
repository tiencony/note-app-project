#include "test_framework.h"
#include "tests.h"

int main(){
    // testLinks();
    // testTags();
    // testCrud();
    // testBacklinks();
    // testBrokenLinks();
    // testLinkIndexFresh();
    // testBrokenLinksDedup();
    // cleanupTestFiles();   
    // testRenameUpdatesLinks();
    
    
    // testLoadReport();
    // testLoadFailureKeepsMemory();
    // testCreateErrors();
    // testRenameErrors();
    // testOpenNote();
    
    // testListNotes();

    // testEditLeavesNoTmp();
    // testRenameReportsUnreadable();
    // testLoadSkipsBadNames();
    // testControlCharTitle();
    testRefreshFromDisk();
    std::cout << "\nKet qua: " << (failed == 0 ? "TAT CA DEU PASS" : "CO TEST THAT BAI")
              << " (" << failed << " that bai)\n";
    return failed == 0 ? 0 : 1;
}