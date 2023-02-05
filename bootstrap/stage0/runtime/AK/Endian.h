/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Format.h>
#include <AK/Forward.h>
#include <AK/Platform.h>

#if defined(AK_OS_MACOS)
#    include <libkern/OSByteOrder.h>
#    include <machine/endian.h>

#    define htobe16(x) OSSwapHostToBigInt16(x)
#    define htole16(x) OSSwapHostToLittleInt16(x)
#    define be16toh(x) OSSwapBigToHostInt16(x)
#    define le16toh(x) OSSwapLittleToHostInt16(x)

#    define htobe32(x) OSSwapHostToBigInt32(x)
#    define htole32(x) OSSwapHostToLittleInt32(x)
#    define be32toh(x) OSSwapBigToHostInt32(x)
#    define le32toh(x) OSSwapLittleToHostInt32(x)

#    define htobe64(x) OSSwapHostToBigInt64(x)
#    define htole64(x) OSSwapHostToLittleInt64(x)
#    define be64toh(x) OSSwapBigToHostInt64(x)
#    define le64toh(x) OSSwapLittleToHostInt64(x)

#    define __BIG_ENDIAN BIG_ENDIAN
#    define __LITTLE_ENDIAN LITTLE_ENDIAN
#    define __BYTE_ORDER BYTE_ORDER
#endif

namespace AK {

template<typename T>
ALWAYS_INLINE constexpr T convert_between_host_and_little_endian(T value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    if constexpr (sizeof(T) == 8)
        return static_cast<T>(__builtin_bswap64(static_cast<u64>(value)));
    if constexpr (sizeof(T) == 4)
        return static_cast<T>(__builtin_bswap32(static_cast<u32>(value)));
    if constexpr (sizeof(T) == 2)
        return static_cast<T>(__builtin_bswap16(static_cast<u16>(value)));
    if constexpr (sizeof(T) == 1)
        return value;
#endif
}

template<typename T>
ALWAYS_INLINE constexpr T convert_between_host_and_big_endian(T value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    if constexpr (sizeof(T) == 8)
        return static_cast<T>(__builtin_bswap64(static_cast<u64>(value)));
    if constexpr (sizeof(T) == 4)
        return static_cast<T>(__builtin_bswap32(static_cast<u32>(value)));
    if constexpr (sizeof(T) == 2)
        return static_cast<T>(__builtin_bswap16(static_cast<u16>(value)));
    if constexpr (sizeof(T) == 1)
        return value;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return value;
#endif
}

template<typename T>
ALWAYS_INLINE T convert_between_host_and_network_endian(T value)
{
    return convert_between_host_and_big_endian(value);
}

template<typename T>
class LittleEndian;

template<typename T>
DeprecatedInputStream& operator>>(DeprecatedInputStream&, LittleEndian<T>&);

template<typename T>
DeprecatedOutputStream& operator<<(DeprecatedOutputStream&, LittleEndian<T>);

template<typename T>
class [[gnu::packed]] LittleEndian {
public:
    friend DeprecatedInputStream& operator>><T>(DeprecatedInputStream&, LittleEndian<T>&);
    friend DeprecatedOutputStream& operator<< <T>(DeprecatedOutputStream&, LittleEndian<T>);

    constexpr LittleEndian() = default;

    constexpr LittleEndian(T value)
        : m_value(convert_between_host_and_little_endian(value))
    {
    }

    constexpr operator T() const { return convert_between_host_and_little_endian(m_value); }

    // This returns the internal representation. In this case, that is the value stored in little endian format.
    constexpr Bytes bytes() { return Bytes { &m_value, sizeof(m_value) }; }
    constexpr ReadonlyBytes bytes() const { return ReadonlyBytes { &m_value, sizeof(m_value) }; }

private:
    T m_value { 0 };
};

template<typename T>
class BigEndian;

template<typename T>
DeprecatedInputStream& operator>>(DeprecatedInputStream&, BigEndian<T>&);

template<typename T>
DeprecatedOutputStream& operator<<(DeprecatedOutputStream&, BigEndian<T>);

template<typename T>
class [[gnu::packed]] BigEndian {
public:
    friend DeprecatedInputStream& operator>><T>(DeprecatedInputStream&, BigEndian<T>&);
    friend DeprecatedOutputStream& operator<< <T>(DeprecatedOutputStream&, BigEndian<T>);

    constexpr BigEndian() = default;

    constexpr BigEndian(T value)
        : m_value(convert_between_host_and_big_endian(value))
    {
    }

    constexpr operator T() const { return convert_between_host_and_big_endian(m_value); }

    // This returns the internal representation. In this case, that is the value stored in big endian format.
    constexpr Bytes bytes() { return Bytes { &m_value, sizeof(m_value) }; }
    constexpr ReadonlyBytes bytes() const { return ReadonlyBytes { &m_value, sizeof(m_value) }; }

private:
    T m_value { 0 };
};

template<typename T>
using NetworkOrdered = BigEndian<T>;

template<typename T>
requires(HasFormatter<T>) struct Formatter<LittleEndian<T>> : Formatter<T> {
};

template<typename T>
requires(HasFormatter<T>) struct Formatter<BigEndian<T>> : Formatter<T> {
};

template<typename T>
struct Traits<LittleEndian<T>> : public GenericTraits<LittleEndian<T>> {
    static constexpr bool is_trivially_serializable() { return Traits<T>::is_trivially_serializable(); }
};

template<typename T>
struct Traits<BigEndian<T>> : public GenericTraits<BigEndian<T>> {
    static constexpr bool is_trivially_serializable() { return Traits<T>::is_trivially_serializable(); }
};

}

#if USING_AK_GLOBALLY
using AK::BigEndian;
using AK::LittleEndian;
using AK::NetworkOrdered;
#endif