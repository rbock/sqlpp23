#pragma once

#define assert_throw(code, exception)                \
  {                                                  \
    bool exceptionThrown = false;                    \
    try {                                            \
      code;                                          \
    } catch (const exception& e) {                   \
      exceptionThrown = true;                        \
      std::cerr << "message: " << e.what() << '\n';  \
    }                                                \
    if (not exceptionThrown)                         \
      throw std::runtime_error("missing exception"); \
  }
