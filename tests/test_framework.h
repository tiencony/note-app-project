#pragma once
#include <iostream>
#include <string>

inline int failed = 0;

// endl: xa bo dem ngay, nen neu chuong trinh sap thi van thay dong cuoi cung da chay
inline void check(bool condition, const std::string& name){
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << name << std::endl;
    if(!condition) failed++;
}

// Giong check nhung khi FAIL se in gia tri thuc te va mong doi
template <typename A, typename B>
inline void checkEq(const A& actual, const B& expected, const std::string& name){
    const bool ok = (actual == expected);
    check(ok, name);
    if(!ok){
        std::cout << "    thuc te : [" << actual   << "]\n"
                  << "    mong doi: [" << expected << "]" << std::endl;
    }
}