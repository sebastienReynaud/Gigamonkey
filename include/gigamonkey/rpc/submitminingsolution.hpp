// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_RPC_SUBMITMININGSOLUTION
#define GIGAMONKEY_RPC_SUBMITMININGSOLUTION

#include <gigamonkey/work/string.hpp>

namespace Gigamonkey::bitcoind::rpc::submitminingsolution {

    struct request : json {
        
        struct deserialized;
        
        static void to_json(json& j, const deserialized&);
        static void from_json(const json&, deserialized&);
        
        static request serialize(const deserialized& d) {
            request e;
            to_json(static_cast<json&>(e), d);
            return e;
        }
        
        static deserialized deserialize(const json& j) {
            deserialized d;
            from_json(j, d);
            return d;
        }
        
        struct deserialized {
        
            // The assigned id from Bitcoind. 
            string id;
            
            Gigamonkey::nonce nonce;
            
            optional<Bitcoin::transaction> coinbase;
            
            optional<Bitcoin::timestamp> time;
            
            optional<int32_little> version;
            
            bool valid() const {
                return id != "" && (!coinbase || coinbase->valid()) && (!time || time->valid());
            }
            
            bool operator==(const deserialized& j) {
                return id == j.id && nonce == j.nonce && coinbase == j.coinbase && time == j.time && version == j.version;
            }
            
            bool operator!=(const deserialized& j) {
                return !(*this == j);
            }
        
            request serialize() {
                return request::serialize(*this);
            }
            
        private:
            deserialized() : id{}, nonce{}, coinbase{}, time{}, version{} {}
            
            deserialized(const string& i, Gigamonkey::nonce n) : id{i}, nonce{n}, coinbase{}, time{}, version{} {}
            
            deserialized(const string& i, Gigamonkey::nonce n, const Bitcoin::transaction& c, Bitcoin::timestamp t, int32_little v) : 
                id{i}, nonce{n}, coinbase{c}, time{t}, version{v} {}
                
            friend struct request;
            friend void from_json(const json&, deserialized&);
        };
        
        deserialized deserialize() const {
            return deserialize(*this);
        }
        
        request() : json{} {}
        
        request(const json& j) : json{j} {}
        
        request(const string& json_string) : request{json::parse(json_string)} {}
        
        request(const deserialized& d) : request{serialize(d)} {}
        
        request(const string& i, Gigamonkey::nonce n) : 
            request{deserialized{i, n}} {}
            
        request(const string& i, Gigamonkey::nonce n, const Bitcoin::transaction& c, Bitcoin::timestamp t, int32_little v) : 
            request{deserialized{i, n, c, t, v}} {}
        
        static bool valid(const json& j) {
            return deserialize(j).valid();
        }
        
        static string id(const json& j) {
            return deserialize(j).id;
        }
        
        static Gigamonkey::nonce nonce(const json& j) {
            return deserialize(j).nonce;
        }
        
        static optional<Bitcoin::transaction> coinbase(const json& j) {
            return deserialize(j).coinbase;
        }
        
        static optional<Bitcoin::timestamp> time(const json& j) {
            return deserialize(j).time;
        }
        
        static optional<int32_little> version(const json& j) {
            return deserialize(j).version;
        }
        
        bool valid() const {
            return valid(*this);
        }
        
        string id() const {
            return id(*this); 
        }
        
        Gigamonkey::nonce nonce() const {
            return nonce(*this); 
        }
        
        optional<Bitcoin::transaction> coinbase() const {
            return coinbase(*this); 
        }
        
        optional<Bitcoin::timestamp> time() const {
            return time(*this); 
        }
        
        optional<int32_little> version() const {
            return version(*this); 
        }
    };
}

#endif
