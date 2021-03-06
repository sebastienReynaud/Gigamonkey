// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_STRATUM_MINING_SUBSCRIBE
#define GIGAMONKEY_STRATUM_MINING_SUBSCRIBE

#include <gigamonkey/stratum/stratum.hpp>
#include <gigamonkey/stratum/mining.hpp>

namespace Gigamonkey::Stratum::mining {
    
    struct subscription {
        method Method;
        session_id ID;
        
        subscription() : Method{unset}, ID{} {}
        subscription(method m, session_id id) : Method{m}, ID{id} {}
        explicit subscription(const json&);
        explicit operator json() const;
        
        bool valid() const {
            return Method != unset;
        }
    };
    
    bool operator==(const subscription& a, const subscription& b);
    bool operator!=(const subscription& a, const subscription& b);
    
    std::ostream& operator<<(std::ostream&, const subscription&);
    
    struct subscribe_request : request {
        struct parameters {
            string UserAgent;
            std::optional<session_id> ExtraNonce1;
            
            parameters(const string& u) : UserAgent{u}, ExtraNonce1{} {}
            parameters(const string& u, session_id i) : UserAgent{u}, ExtraNonce1{i} {}
            
            bool valid() const;
            bool operator==(const parameters& p) const;
            bool operator!=(const parameters& p) const;
            
        private:
            parameters() : UserAgent{}, ExtraNonce1{} {}
            
            friend struct subscribe_request;
        };
        
        static Stratum::parameters serialize(const parameters&);
        static parameters deserialize(const Stratum::parameters&);
        
        static bool valid(const json& j);
        static string user_agent(const json& j);
        static std::optional<session_id> extra_nonce_1(const json& j);
        
        bool valid() const;
        string user_agent() const;
        std::optional<session_id> extra_nonce_1() const;
        
        using request::request;
        subscribe_request(request_id id, const string& u) : request{id, mining_subscribe, {u}} {}
        subscribe_request(request_id id, const string& u, session_id i) : request{id, mining_subscribe, serialize(parameters{u, i})} {}
        
    };
    
    struct subscribe_response : response {
        struct parameters {
            list<subscription> Subscriptions;
            session_id ExtraNonce1;
            uint32 ExtraNonce2Size;
            
            bool valid() const;
            
            parameters(list<subscription> s, session_id n1, uint32 n2x);
            parameters(list<subscription> s, session_id n1);
            
            bool operator==(const parameters& p) const;
            bool operator!=(const parameters& p) const;
            
        private:
            parameters() : Subscriptions{}, ExtraNonce1{}, ExtraNonce2Size{} {}
            friend struct subscribe_response;
        };
        
        static Stratum::parameters serialize(const parameters&);
        static parameters deserialize(const Stratum::parameters&);
        
        static bool valid(const json& j);
        static list<subscription> subscriptions(const json& j);
        static session_id extra_nonce_1(const json& j);
        static uint32 extra_nonce_2_size(const json& j);
        
        bool valid() const;
        list<subscription> subscriptions() const;
        session_id extra_nonce_1() const;
        uint32 extra_nonce_2_size() const;
        
        using response::response;
        subscribe_response(request_id id, list<subscription> sub, session_id i, uint32 x) : 
            response{id, serialize(parameters{sub, i, x})} {}
        
    };
    
    bool inline operator==(const subscription& a, const subscription& b) {
        return a.Method && b.Method && a.ID == b.ID;
    }
    
    bool inline operator!=(const subscription& a, const subscription& b) {
        return !(a == b);
    }
    
    bool inline subscribe_request::parameters::valid() const {
        return UserAgent != "" && (!bool(ExtraNonce1) || data::valid(*ExtraNonce1));
    }
    
    bool inline subscribe_request::parameters::operator==(const parameters& p) const {
        return UserAgent == p.UserAgent && ExtraNonce1 == p.ExtraNonce1;
    }
    
    bool inline subscribe_request::parameters::operator!=(const parameters& p) const {
        return UserAgent != p.UserAgent || ExtraNonce1 != p.ExtraNonce1;
    }
    
    bool inline subscribe_response::parameters::valid() const {
        return ExtraNonce2Size != 0;
    }
    
    inline std::ostream& operator<<(std::ostream& o, const subscription& s) {
        return o << json(s);
    }
    
    inline subscribe_response::parameters::parameters(list<subscription> s, session_id n1, uint32 n2x) : 
        Subscriptions{s}, ExtraNonce1{n1}, ExtraNonce2Size{n2x} {}
    
    inline subscribe_response::parameters::parameters(list<subscription> s, session_id n1) : 
        Subscriptions{s}, ExtraNonce1{n1}, ExtraNonce2Size{worker::ExtraNonce2_size} {}
    
    bool inline subscribe_response::parameters::operator==(const parameters& p) const {
        return Subscriptions == p.Subscriptions && ExtraNonce1 == ExtraNonce1 && ExtraNonce2Size == ExtraNonce2Size;
    }
    
    bool inline subscribe_response::parameters::operator!=(const parameters& p) const {
        return !(*this == p);
    }
    
    bool inline subscribe_request::valid() const {
        return valid(*this);
    }
    
    string inline subscribe_request::user_agent() const {
        return user_agent(*this);
    }
    
    std::optional<session_id> inline subscribe_request::extra_nonce_1() const {
        return extra_nonce_1(*this);
    }
    
    bool inline subscribe_request::valid(const json& j) {
        return request::valid(j) && deserialize(j["params"]).valid();
    }
    
    string inline subscribe_request::user_agent(const json& j) {
        return deserialize(j["params"]).UserAgent;
    }
    
    std::optional<session_id> inline subscribe_request::extra_nonce_1(const json& j) {
        return deserialize(j["params"]).ExtraNonce1;
    }
        
    bool inline subscribe_response::valid(const json& j) {
        return response::valid(j) && deserialize(j["result"]).valid();
    }
    
    list<subscription> inline subscribe_response::subscriptions(const json& j) {
        return deserialize(j["result"]).Subscriptions;
    }
    
    session_id inline subscribe_response::extra_nonce_1(const json& j) {
        return deserialize(j["result"]).ExtraNonce1;
    }
    
    uint32 inline subscribe_response::extra_nonce_2_size(const json& j) {
        return deserialize(j["result"]).ExtraNonce2Size;
    }
    
    bool inline subscribe_response::valid() const {
        return valid(*this);
    }
    
    list<subscription> inline subscribe_response::subscriptions() const {
        return subscriptions(*this);
    }
    
    session_id inline subscribe_response::extra_nonce_1() const {
        return extra_nonce_1(*this);
    }
    
    uint32 inline subscribe_response::extra_nonce_2_size() const {
        return extra_nonce_2_size(*this);
    }
    
}

#endif
