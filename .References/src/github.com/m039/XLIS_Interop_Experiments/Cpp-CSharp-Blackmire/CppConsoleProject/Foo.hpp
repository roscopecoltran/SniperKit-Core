#pragma once

#include <cstdint>

namespace CSharpFooBarLibrary {
  class Foo {
  public:
    int32_t number = 0;

    Foo();

    int GetNumber();

    void SetNumber(int32_t number);
  };
} /* CSharpFooBarLibrary */
