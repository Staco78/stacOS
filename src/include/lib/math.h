#pragma once

template <typename T>
inline constexpr T max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
inline constexpr T min(T a, T b)
{
    return a < b ? a : b;
}