// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/coinbase.hpp>

namespace Gigamonkey::Bitcoin::mining {
    
    bytes write_cscript_compact(int32 height);
    
    bytes_reader read_cscript_compact(bytes_reader, int32&); 
    
    size_t cscript_compact_size(int32);
    
    bytes part_1(int32 height) {
        return write(36 + cscript_compact_size(height), outpoint::coinbase(), write_cscript_compact(height));
    }
    
    bytes part_2(list<Bitcoin::output> outputs) {
        bytes p2;
        p2.resize(var_int_size(outputs.size() + 8));
        bytes_writer w{p2.begin(), p2.end()};
        write_sequence(w << uint32_little{0xffffffff}, outputs) << uint32_little{0};
        return p2;
    }
    
    bool coinbase::bip44(int32 height) const {
        const script& x = transaction::Inputs.first().Script;
        bytes_reader b{x.data(), x.data() + x.size()};
        int32 h;
        read_cscript_compact(b, h);
        return h == height;
    }
    
    coinbase::coinbase(int32 height, Stratum::session_id nonce1, uint64_big nonce2, list<Bitcoin::output> outputs) : 
        transaction{
            list<Bitcoin::input>{Bitcoin::input{
                outpoint::coinbase(), 
                Gigamonkey::write(cscript_compact_size(height) + 12, write_cscript_compact(height), nonce1, nonce2)}}, outputs, 0} {}
}
