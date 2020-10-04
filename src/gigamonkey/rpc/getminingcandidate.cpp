// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/rpc/getminingcandidate.hpp>

namespace Gigamonkey::bitcoind::rpc::getminingcandidate {
    
    namespace {
        
        template <endian::order o>
        struct hex_digest {
            
            static bool deserialize(const json& j, Gigamonkey::digest256& d) {
                if (!j.is_string()) return false;
                string x(j);
                encoding::hex::view v{x};
                if (!v.valid() || v.size() != 64) return {};
                bytes b{bytes_view(v)};
                if (o == endian::big) std::reverse_copy(b.begin(), b.end(), d.begin());
                else std::copy(b.begin(), b.end(), d.begin());
                return true;
            }
            
        };
        
        template <endian::order o, bool is_signed>
        struct hex_number : json {
            
            static bool deserialize(const json& j, endian::arithmetic<o, is_signed, 4>& d) {
                if (!j.is_string()) return false;
                string x(j);
                encoding::hex::view v{x};
                if (!v.valid() || v.size() == 8) return false;
                bytes b{bytes_view(v)};
                if (o == endian::big) std::reverse_copy(b.begin(), b.end(), d.begin());
                else std::copy(b.begin(), b.end(), d.begin());
                return true;
            }
            
        };
        
        string write_hex_big(const uint32_little f) {
            return encoding::hex::write(bytes_view(f), encoding::hex::lower);
        }
        
        string write_hex_big(const digest256& d) {
            return encoding::hex::write(bytes_view(d), encoding::hex::lower);
        }
        
        string write_hex_little(const digest256& d) {
            return encoding::hex::write(bytes_view(d), endian::little, encoding::hex::lower);
        }
        
        json write_merkle_proof(const Merkle::digests p) {
            json::array_t a;
            a.resize(p.size());
            auto n = a.begin();
            for (const Merkle::digest& d : p) {
                *n = write_hex_little(d);
                n++;
            }
            return a;
        }
        
        bool read_id(const json& j, string& d) {
            if (!j.contains("id")) return false;
            const auto& x = j["id"];
            if (!x.is_string()) return false;
            d = string(j["id"]);
            return true;
        }
        
        bool read_version(const json& j, int32_little& d) {
            if (!j.contains("version")) return false;
            const auto& x = j["version"];
            if (!x.is_number_unsigned()) return false;
            d = int32(x);
            return true;
        }
        
        bool read_prevHash(const json& j, digest256& d) {
            if (!j.contains("prevhash")) return false;
            const auto& x = j["prevhash"];
            if (!hex_digest<endian::big>::deserialize(x, d)) return false;
            return true;
        }
        
        bool read_nBits(const json& j, work::compact& d) {
            if (!j.contains("nBits")) return false;
            const auto& x = j["nBits"];
            uint32_big n;
            if (!hex_number<endian::big, false>::deserialize(x, n)) return false;
            d = work::compact(n);
            return true;
        }
        
        bool read_time(const json& j, Bitcoin::timestamp& d) {
            if (!j.contains("time")) return false;
            const auto& x = j["time"];
            if (!x.is_number_unsigned()) return false;
            d = Bitcoin::timestamp(int32(x));
            return true;
        }
        
        bool read_height(const json& j, int& d) {
            if (!j.contains("height")) return false;
            const auto& x = j["height"];
            if (!x.is_number_integer()) return false;
            d = int(x);
            return true;
        }
        
        bool read_merkleProof(const json& j, Merkle::digests& d) {
            if (!j.contains("merkleProof")) return false;
            const auto& x = j["merkleProof"];
            if (!x.is_array()) return false;
            Merkle::digests m{};
            for (auto& q : x) {
                if (!q.is_string()) return false;
                digest256 g;
                if(!hex_digest<endian::big>::deserialize(string(q), g)) return false;
                m = m << g;
            }
            d = m;
            return true;
        }
        
        bool read_coinbase(const json& j, Bitcoin::transaction& d) {
            if (!j.contains("coinbase")) return false;
            const auto& x = j["coinbase"];
            string st(x);
            encoding::hex::view v{st};
            if (!v.valid()) return false;
            Bitcoin::transaction n = Bitcoin::transaction::read(bytes_view(v));
            if (!n.valid()) return false;
            d = n;
            return true;
        }
        
        bool read_coinbaseValue(const json& j, satoshi& d) {
            if (!j.contains("coinbaseValue")) return false;
            const auto& x = j["coinbaseValue"];
            if (!x.is_number_integer()) return false;
            d = int(x);
            return true;
        }
    
    }
    
    void response::from_json(const json& j, deserialized& d) {
        d = {};
        
        deserialized x{};
        
        if (!(read_id(j, x.id) && 
            read_version(j, x.version) && 
            read_prevHash(j, x.prevhash) && 
            read_nBits(j, x.nBits) && 
            read_time(j, x.time) && 
            read_height(j, x.height) && 
            read_merkleProof(j, x.merkleProof.Digests) && 
            read_coinbaseValue(j, x.coinbaseValue))) return;
        
        Bitcoin::transaction cb;
        if(read_coinbase(j, cb)) x.coinbase = cb;
        
        d = x;
    }
    
    void response::to_json(json& j, const deserialized& d) {
        j = {};
        
        if (!d.valid()) return;
        
        j = {{"id", d.id}, 
            {"version", d.version}, 
            {"prevhash", write_hex_big(d.prevhash)}, 
            {"nBits", write_hex_big(d.nBits)}, 
            {"time", write_hex_big(d.time)},  
            {"height", d.height}, 
            {"merkleProof", write_merkle_proof(d.merkleProof.Digests)}, 
            {"coinbaseValue", d.coinbaseValue}};
        
        if (d.coinbase) j["coinbase"] = encoding::hex::write(d.coinbase->write());
    }
    
}

