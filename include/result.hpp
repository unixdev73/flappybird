#pragma once

namespace fb {
namespace Result {
constexpr int Success{0};
constexpr int Error{1};
constexpr int ReadError{2};
constexpr int DomainError{3};
constexpr int SyntaxError{4};
constexpr int ConversionError{5};
constexpr int NotFound{6};
} // namespace Result
} // namespace fb
