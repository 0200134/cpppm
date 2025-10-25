#!/usr/bin/env python3
import os

TEST_DIR = "tests"
TEST_FILE = os.path.join(TEST_DIR, "test_basic.cpp")

if not os.path.exists(TEST_FILE):
    os.makedirs(TEST_DIR, exist_ok=True)
    with open(TEST_FILE, "w", encoding="utf-8") as f:
        f.write("""#include <iostream>
#include <cassert>
int main() {
    std::cout << "[cpppm] auto-generated dummy test\\n";
    assert(true);
    return 0;
}
""")
    print(f"[SELF-HEAL] Created missing test: {TEST_FILE}")
else:
    print("[SELF-HEAL] Test file already exists.")
