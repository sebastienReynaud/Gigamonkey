// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_WORK_TARGET
#define GIGAMONKEY_WORK_TARGET

#include <gigamonkey/hash.hpp>
#include <gigamonkey/timestamp.hpp>
#include <vector>

namespace Gigamonkey::work {
    using uint256 = Gigamonkey::uint256;
    
    struct difficulty : Q {
        explicit difficulty(const Q&);
        explicit difficulty(const uint256& u);
        explicit operator double() const;
        
        static Q minimum() {
            static Q Minimum{Z{"0x00000000FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"}};
            return Minimum;
        }
    };
    
    inline difficulty::difficulty(const uint256& u) : difficulty{minimum() / N{u}} {}
    
    uint256 expand_compact(uint32_little);
    
    struct target : uint32_little {
        
        static target encode(byte e, uint24_little v) {
            target t;
            bytes_writer(t.begin(), t.end()) << v << e;
            return t;
        }
        
        target() : uint32_little{0} {}
        explicit target(uint32_little i) : uint32_little{i} {}
        target(byte e, uint24_little v) : target{encode(e, v)} {}
        
        byte exponent() const {
            return static_cast<byte>(static_cast<uint32_little>(*this) & 0x000000ff);
        }
        
        uint24_little digits() const {
            return uint24_little{static_cast<uint32_little>(*this) >> 8};
        }
        
        bool valid() const {
            byte e = exponent();
            return e >= 3 && e <= 32 && digits() != 0;
        }
        
        uint256 expand() const {
            return expand_compact(static_cast<uint32_little>(*this));
        }
        
        work::difficulty difficulty() const {
            return work::difficulty(expand());
        };
        
        operator bytes_view() const {
            return bytes_view{uint32_little::data(), 4};
        }
    };
    
    const target SuccessHalf{32, 0x800000};
    const target SuccessQuarter{32, 0x400000};
    const target SuccessEighth{32, 0x200000};
    const target SuccessSixteenth{32, 0x100000};

}

#endif 