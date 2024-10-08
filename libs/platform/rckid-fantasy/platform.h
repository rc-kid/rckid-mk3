#pragma once

#include <stdint.h>

#define UNIMPLEMENTED do { rckid::fatalError(rckid::Error::Unimplemented, __LINE__, __FILE__); } while (false)
#define UNREACHABLE do { rckid::fatalError(rckid::Error::Unreachable, __LINE__, __FILE__); } while (false)

#define ASSERT(...) do { if (!(__VA_ARGS__)) rckid::fatalError(rckid::Error::Assert, __LINE__, __FILE__); } while (false)

#define LOG(...) rckid::debugWrite() << __VA_ARGS__ << '\n';

#include "../pc/platform.h"
