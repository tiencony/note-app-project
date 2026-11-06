#include <iostream>
#include "../headers/NoteManager.h"

int main() {
    // Khởi tạo NoteManager với sức chứa ban đầu là 2 (để test thử cơ chế resize)
    NoteManager manager(2);

    // 1. Tạo các ghi chú mới
    manager.createNote("Hoc_Cpp", "Tim hieu ve con tro va cap phat dong trong C++.");
    manager.createNote("Ke_hoach_tuan", "1. Hoan thanh NoteManager\n2. Xay dung ExpenseManager.");

    // 2. Liệt kê danh sách ghi chú
    std::cout << "\n--- DANH SACH TAT CA GHI CHU ---\n";
    manager.listNotes();

    // 3. Tìm kiếm ghi chú
    std::cout << "\n--- TIM KIEM GHI CHU ('Cpp') ---\n";
    manager.searchNote("Cpp");

    return 0;
}