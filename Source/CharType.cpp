#include <Source/Headers.h>
#include "UTF8Tools.h"

// ----------------------------------------------------------------------------
uint8_t gCharTypeData[128] =
{
  utf8Control,              // CTRL+@, 0x00
  utf8Control,              // CTRL+A, 0x01
  utf8Control,              // CTRL+B, 0x02
  utf8Control,              // CTRL+C, 0x03
  utf8Control,              // CTRL+D, 0x04
  utf8Control,              // CTRL+E, 0x05
  utf8Control,              // CTRL+F, 0x06
  utf8Control,              // CTRL+G, 0x07
  utf8Control,              // CTRL+H, 0x08
  utf8Control | utf8Space,  // CTRL+I, 0x09
  utf8Control | utf8Space,  // CTRL+J, 0x0a
  utf8Control | utf8Space,  // CTRL+K, 0x0b
  utf8Control | utf8Space,  // CTRL+L, 0x0c
  utf8Control | utf8Space,  // CTRL+M, 0x0d
  utf8Control,              // CTRL+N, 0x0e
  utf8Control,              // CTRL+O, 0x0f
  utf8Control,              // CTRL+P, 0x10
  utf8Control,              // CTRL+Q, 0x11
  utf8Control,              // CTRL+R, 0x12
  utf8Control,              // CTRL+S, 0x13
  utf8Control,              // CTRL+T, 0x14
  utf8Control,              // CTRL+U, 0x15
  utf8Control,              // CTRL+V, 0x16
  utf8Control,              // CTRL+W, 0x17
  utf8Control,              // CTRL+X, 0x18
  utf8Control,              // CTRL+Y, 0x19
  utf8Control,              // CTRL+Z, 0x1a
  utf8Control,              // CTRL+[, 0x1b
  utf8Control,              // CTRL+\, 0x1c
  utf8Control,              // CTRL+], 0x1d
  utf8Control,              // CTRL+^, 0x1e
  utf8Control,              // CTRL+_, 0x1f
  utf8Space,                // ` ', 0x20
  utf8Punct,                // `!', 0x21
  utf8Punct,                // 0x22
  utf8Punct,                // `#', 0x23
  utf8Punct,                // `$', 0x24
  utf8Punct,                // `%', 0x25
  utf8Punct,                // `&', 0x26
  utf8Punct,                // 0x27
  utf8Punct,                // `(', 0x28
  utf8Punct,                // `)', 0x29
  utf8Punct,                // `*', 0x2a
  utf8Punct,                // `+', 0x2b
  utf8Punct,                // `,', 0x2c
  utf8Punct,                // `-', 0x2d
  utf8Punct,                // `.', 0x2e
  utf8Punct,                // `/', 0x2f
  utf8Digit | utf8Hex,      // `0', 0x30
  utf8Digit | utf8Hex,      // `1', 0x31
  utf8Digit | utf8Hex,      // `2', 0x32
  utf8Digit | utf8Hex,      // `3', 0x33
  utf8Digit | utf8Hex,      // `4', 0x34
  utf8Digit | utf8Hex,      // `5', 0x35
  utf8Digit | utf8Hex,      // `6', 0x36
  utf8Digit | utf8Hex,      // `7', 0x37
  utf8Digit | utf8Hex,      // `8', 0x38
  utf8Digit | utf8Hex,      // `9', 0x39
  utf8Punct,                // `:', 0x3a
  utf8Punct,                // `;', 0x3b
  utf8Punct,                // `<', 0x3c
  utf8Punct,                // `=', 0x3d
  utf8Punct,                // `>', 0x3e
  utf8Punct,                // `?', 0x3f
  utf8Punct,                // `@', 0x40
  utf8Upper | utf8Hex,      // `A', 0x41
  utf8Upper | utf8Hex,      // `B', 0x42
  utf8Upper | utf8Hex,      // `C', 0x43
  utf8Upper | utf8Hex,      // `D', 0x44
  utf8Upper | utf8Hex,      // `E', 0x45
  utf8Upper | utf8Hex,      // `F', 0x46
  utf8Upper,                // `G', 0x47
  utf8Upper,                // `H', 0x48
  utf8Upper,                // `I', 0x49
  utf8Upper,                // `J', 0x4a
  utf8Upper,                // `K', 0x4b
  utf8Upper,                // `L', 0x4c
  utf8Upper,                // `M', 0x4d
  utf8Upper,                // `N', 0x4e
  utf8Upper,                // `O', 0x4f
  utf8Upper,                // `P', 0x50
  utf8Upper,                // `Q', 0x51
  utf8Upper,                // `R', 0x52
  utf8Upper,                // `S', 0x53
  utf8Upper,                // `T', 0x54
  utf8Upper,                // `U', 0x55
  utf8Upper,                // `V', 0x56
  utf8Upper,                // `W', 0x57
  utf8Upper,                // `X', 0x58
  utf8Upper,                // `Y', 0x59
  utf8Upper,                // `Z', 0x5a
  utf8Punct,                // `[', 0x5b
  utf8Punct,                // 0x5c
  utf8Punct,                // `]', 0x5d
  utf8Punct,                // `^', 0x5e
  utf8Punct,                // `_', 0x5f
  utf8Punct,                // 0x60
  utf8Lower | utf8Hex,      // `a', 0x61
  utf8Lower | utf8Hex,      // `b', 0x62
  utf8Lower | utf8Hex,      // `c', 0x63
  utf8Lower | utf8Hex,      // `d', 0x64
  utf8Lower | utf8Hex,      // `e', 0x65
  utf8Lower | utf8Hex,      // `f', 0x66
  utf8Lower,                // `g', 0x67
  utf8Lower,                // `h', 0x68
  utf8Lower,                // `i', 0x69
  utf8Lower,                // `j', 0x6a
  utf8Lower,                // `k', 0x6b
  utf8Lower,                // `l', 0x6c
  utf8Lower,                // `m', 0x6d
  utf8Lower,                // `n', 0x6e
  utf8Lower,                // `o', 0x6f
  utf8Lower,                // `p', 0x70
  utf8Lower,                // `q', 0x71
  utf8Lower,                // `r', 0x72
  utf8Lower,                // `s', 0x73
  utf8Lower,                // `t', 0x74
  utf8Lower,                // `u', 0x75
  utf8Lower,                // `v', 0x76
  utf8Lower,                // `w', 0x77
  utf8Lower,                // `x', 0x78
  utf8Lower,                // `y', 0x79
  utf8Lower,                // `z', 0x7a
  utf8Punct,                // `{', 0x7b
  utf8Punct,                // `|', 0x7c
  utf8Punct,                // `}', 0x7d
  utf8Punct,                // `~', 0x7e
  utf8Control,              //      0x7f};
};
