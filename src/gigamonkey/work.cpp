// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/work/proof.hpp>
#include <gigamonkey/hash.hpp>

namespace Gigamonkey::work {
    
    // copied from arith_uint256.cpp and therefore probably works. 
    uint256 expand_compact(uint32_little compact) {
        base_uint<256> expanded;
        int nSize = compact >> 24;
        uint32_t nWord = compact & 0x007fffff;
        if (nSize <= 3) {
            nWord >>= 8 * (3 - nSize);
            expanded = nWord;
        } else {
            expanded = nWord;
            expanded <<= 8 * (nSize - 3);
        }
        
        // negative 
        if (nWord != 0 && (compact & 0x00800000) != 0) return 0;
        
        // overflow
        if (nWord != 0 && ((nSize > 34) || (nWord > 0xff && nSize > 33) ||
                           (nWord > 0xffff && nSize > 32))) return 0;
        
        return digest256(expanded).Value;
    }
        
    bytes string::write() const {
        return Gigamonkey::write(80, Version, Digest, MerkleRoot, Target, Timestamp, Nonce);
    }
    
}

