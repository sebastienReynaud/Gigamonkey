// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/coinbase.hpp>
#include <gigamonkey/script/pattern.hpp>

namespace Gigamonkey::Bitcoin {
    
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
    
    void miner_id::to_json(json& j, const static_document& doc) {
        throw 0;
    }
    
    bool miner_id::from_json(const json& j, static_document& doc) {
        throw 0;
    }
    
    bool miner_id::valid() const { 
        bytes st_doc = StaticDocument.write();
        if (!StaticDocument.valid() && verify(StaticDocumentSignature, sha256(st_doc), StaticDocument.minerId)) return false;
        if (!DynamicDocument && !DynamicDocumentSignature) return true;
        if (!(bool(DynamicDocument) && DynamicDocumentSignature && StaticDocument.dynamicMinerId)) return false;
        return verify(*DynamicDocumentSignature, 
            sha256(Gigamonkey::write(st_doc.size() + bytes_view(StaticDocumentSignature).size() + DynamicDocument->size(), 
                st_doc, StaticDocumentSignature, *DynamicDocument)), 
            *StaticDocument.dynamicMinerId);
    }
    
    bytes miner_id::write() const {
        program p;
        p = p << instruction{StaticDocument.write()} << instruction{StaticDocumentSignature};
        if (DynamicDocument && DynamicDocumentSignature) 
            p = p << instruction{*DynamicDocument} << instruction{*DynamicDocumentSignature};
        return compile(p);
    }
    
    miner_id miner_id::read(bytes_view b) {
        bytes st_doc;
        bytes st_doc_sig;
        bytes dy_doc;
        bytes dy_doc_sig;
        if (!pattern{push{st_doc}, push{st_doc_sig}, optional{pattern{push{dy_doc}, push{dy_doc_sig}}}}.match(b)) return {};
        throw 0;
    }
}
