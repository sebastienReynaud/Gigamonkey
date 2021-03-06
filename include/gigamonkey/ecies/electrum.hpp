// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_ECIES_ELECTRUM
#define GIGAMONKEY_ECIES_ELECTRUM

#include <gigamonkey/hash.hpp>
#include <gigamonkey/secp256k1.hpp>
#include <data/encoding/base58.hpp>

namespace Gigamonkey::ECIES::electrum {
    
    bytes encrypt(const bytes message, const secp256k1::pubkey& to);
        
    bytes decrypt(const bytes message, const secp256k1::secret& to);
    
}

#endif

