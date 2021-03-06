// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_SCRIPT_PATTERN
#define GIGAMONKEY_SCRIPT_PATTERN

#include <gigamonkey/script/script.hpp>

namespace Gigamonkey::Bitcoin { 
    
    class push;
    struct optional;
    struct alternatives;
    struct repeated;
    
    // for matching and scraping values.
    struct pattern {
        bool match(bytes_view b) const {
            try {
                bytes_view rest = scan(b); 
                return rest.size() == 0;
            } catch (fail) {
                return false;
            }
        }
        
        // A pattern which matches a single op code or instruction. 
        explicit pattern(op);
        explicit pattern(instruction);
        
        // A pattern which matches a given program. 
        explicit pattern(program);
        
        // A pattern which matches an empty string.
        pattern() : Pattern{nullptr} {}
        
        // A pattern denoted as a sequence of other patterns. 
        template <typename X, typename... P>
        pattern(X, P...);
        
        struct fail {}; // Used to end out of a scan operation immediately. 
        
        virtual bytes_view scan(bytes_view p) const {
            if (Pattern == nullptr) return p;
            return Pattern->scan(p);
        }
        
        virtual ~pattern() {}
        
        struct sequence;
    protected:
        struct atom;
        struct string;
        ptr<pattern> Pattern;
        explicit pattern(ptr<pattern> p) : Pattern{p} {};
    };
    
    // A pattern that matches anything. 
    struct any final : pattern {
        any() {}
        virtual bytes_view scan(bytes_view p) const final override;
    };
    
    // A pattern that represents a single instruction. 
    struct pattern::atom final : pattern {
        instruction Instruction;
        atom(instruction i) : Instruction{i} {}
        
        virtual bytes_view scan(bytes_view p) const final override;
    };
    
    // A pattern that represents a single instruction. 
    struct pattern::string final : pattern {
        bytes Program;
        string(program p) : Program{compile(p)} {}
        
        virtual bytes_view scan(bytes_view p) const final override;
    };
    
    // A pattern that represents a push instruction 
    // and which has the ability to scrape values of
    // push instructions as a pattern is being read. 
    class push final : public pattern {
        enum type : byte {any, value, data, read};
        type Type;
        Z Value;
        bytes Data;
        bytes& Read;
        
    public:
        // match any push data.
        push() : Type{any}, Value{0}, Data{}, Read{Data} {}
        // match any push data of the given value
        push(int64 v) : Type{value}, Value{v}, Data{}, Read{Data} {}
        // match a push of the given data. 
        push(bytes_view b) : Type{data}, Value{0}, Data{b}, Read{Data} {}
        // match any push data and save the result.
        push(bytes& r) : Type{read}, Value{0}, Data{}, Read{r} {}
        
        bool match(const instruction& i) const;
        
        virtual bytes_view scan(bytes_view p) const final override;
        
        operator instruction() const;
    };
    
    class push_size final : public pattern {
        bool Reader;
        size_t Size;
        bytes Data;
        bytes& Read;
        
    public:
        // match any push data of the given value
        push_size(size_t s) : Reader{false}, Size{s}, Data{}, Read{Data} {}
        // match any push data and save the result.
        push_size(size_t s, bytes& r) : Reader(true), Size(s), Data(), Read(r) {}
        
        bool match(const instruction& i) const;
        
        virtual bytes_view scan(bytes_view p) const final override;
    };
    
    enum repeated_directive : byte {
        exactly, or_more, or_less
    };
    
    struct repeated final : public pattern {
        int64 First;
        int Second;
        repeated_directive Directive;
        
        repeated(op, uint32 = 1, repeated_directive = or_more);
        repeated(instruction, uint32 = 1, repeated_directive = or_more);
        repeated(push, uint32 = 1, repeated_directive = or_more);
        repeated(optional, uint32 = 1, repeated_directive = or_more);
        repeated(pattern, uint32 = 1, repeated_directive = or_more);
        repeated(repeated, uint32, repeated_directive = or_more);
        repeated(alternatives, uint32 = 1, repeated_directive = or_more);
        
        repeated(op, uint32, uint32);
        repeated(instruction, uint32, uint32);
        repeated(push, uint32, uint32);
        repeated(optional, uint32, uint32);
        repeated(pattern, uint32, uint32);
        repeated(repeated, uint32, uint32);
        repeated(alternatives, uint32, uint32);
        
        virtual bytes_view scan(bytes_view p) const final override;
    };
    
    struct optional final : pattern {
        
        optional(op o) : pattern{o} {}
        optional(instruction i) : pattern{i} {}
        optional(push p) : pattern{ptr<pattern>(std::make_shared<push>(p))} {}
        optional(repeated p) : pattern{ptr<pattern>(std::make_shared<repeated>(p))} {}
        optional(push_size p) : pattern{ptr<pattern>(std::make_shared<push_size>(p))} {}
        optional(pattern p);
        optional(alternatives);
        
        virtual bytes_view scan(bytes_view p) const final override;
    };
    
    inline pattern::pattern(op o) : pattern{instruction{o}} {}
    
    inline pattern::pattern(instruction i) : Pattern{ptr<pattern>(std::make_shared<atom>(i))} {}
    
    inline pattern::pattern(program p) : Pattern{ptr<pattern>(std::make_shared<string>(p))} {}
    
    struct pattern::sequence : public pattern {
        list<ptr<pattern>> Patterns;
        
        template <typename... P>
        sequence(P... p) : Patterns(make(p...)) {}
        
        virtual bytes_view scan(bytes_view p) const override;
        
    private:
        
        static ptr<pattern> construct(op p);
        static ptr<pattern> construct(instruction p);
        static ptr<pattern> construct(program p);
        static ptr<pattern> construct(push p);
        static ptr<pattern> construct(push_size p);
        static ptr<pattern> construct(alternatives p);
        static ptr<pattern> construct(optional p);
        static ptr<pattern> construct(pattern p);
        
        template <typename X> 
        static list<ptr<pattern>> make(X x);
        
        template <typename X, typename... P>
        static list<ptr<pattern>> make(X x, P... p);
    };
    
    struct alternatives final : pattern::sequence {
    public:
        template <typename... P>
        alternatives(P... p) : sequence{p...} {}
        
        virtual bytes_view scan(bytes_view) const final override;
    };
    
    // A pattern that matches a pubkey and grabs the value of that pubkey.
    inline pattern pubkey_pattern(bytes& pubkey) {
        return pattern{alternatives{push_size{33, pubkey}, push_size{65, pubkey}}};
    }
    
    struct op_return_data {
        static Bitcoin::pattern pattern() {
            static Bitcoin::pattern Pattern{optional{OP_FALSE}, OP_RETURN, repeated{push{}, 0}};
            return Pattern;
        }
        
        static Gigamonkey::script script(list<bytes> push);
        
        list<bytes> Push;
        bool Safe; // whether op_false is pushed before op_return
        bool Valid;
        
        bytes script() const {
            return script(Push);
        };
        
        op_return_data(bytes_view);
        op_return_data(list<bytes> p) : Push{p}, Safe{true}, Valid{true} {}
    };
    
    struct pay_to_pubkey {
        static Bitcoin::pattern pattern(bytes& pubkey) {
            return {pubkey_pattern(pubkey), OP_CHECKSIG};
        }
        
        static bytes script(pubkey p) {
            return compile(program{push_data(p), OP_CHECKSIG});
        }
        
        pubkey Pubkey;
        
        bool valid() const {
            return Pubkey.valid();
        }
        
        bytes script() const {
            return script(Pubkey);
        }
        
        pay_to_pubkey(bytes_view script) : Pubkey{} {
            pubkey p;
            if (!pattern(p.Value).match(script)) return;
            Pubkey = p;
        }
        
        static bytes redeem(const signature& s) {
            return compile(push_data(s));
        }
    };
    
    struct pay_to_address {
        static Bitcoin::pattern pattern(bytes& address) {
            return {OP_DUP, OP_HASH160, push_size{20, address}, OP_EQUALVERIFY, OP_CHECKSIG};
        }
        
        static bytes script(const digest160& a) {
            return compile(program{OP_DUP, OP_HASH160, bytes_view(a), OP_EQUALVERIFY, OP_CHECKSIG});
        }
        
        digest160 Address;
        
        bool valid() const {
            return Address.valid();
        }
        
        bytes script() const {
            return script(Address);
        }
        
        pay_to_address(bytes_view script) : Address{} {
            bytes addr{20};
            pattern(addr).match(script);
            std::copy(addr.begin(), addr.end(), Address.Value.begin());
        }
        
        static bytes redeem(const signature& s, const pubkey& p) {
            return compile(program{} << push_data(s) << push_data(p));
        }
    };

    std::ostream& operator<<(std::ostream& o, const instruction i);

    inline bytes_writer operator<<(bytes_writer w, const instruction i) {
        return i.write(w);
    }
    
    template <typename X, typename... P>
    pattern::pattern(X x, P... p) : Pattern(std::make_shared<sequence>(x, p...)) {}
    
    inline repeated::repeated(op x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    inline repeated::repeated(instruction x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    inline repeated::repeated(push x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    inline repeated::repeated(optional x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    inline repeated::repeated(pattern x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    inline repeated::repeated(repeated x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    inline repeated::repeated(alternatives x, uint32 first, repeated_directive d) 
        : pattern{x}, First{first}, Second{-1}, Directive{d} {}
    
    inline repeated::repeated(op x, uint32 first, uint32 second) 
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    inline repeated::repeated(instruction x, uint32 first, uint32 second)
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    inline repeated::repeated(push x, uint32 first, uint32 second)
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    inline repeated::repeated(optional x, uint32 first, uint32 second)
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    inline repeated::repeated(pattern x, uint32 first, uint32 second)
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    inline repeated::repeated(repeated x, uint32 first, uint32 second)
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    inline repeated::repeated(alternatives x, uint32 first, uint32 second)
        : pattern{x}, First{first}, Second{static_cast<int>(second)}, Directive{exactly} {}
    
    inline ptr<pattern> pattern::sequence::construct(op p) {
        return std::make_shared<atom>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(instruction p) {
        return std::make_shared<atom>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(program p) {
        return std::make_shared<string>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(push p) {
        return std::make_shared<push>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(push_size p) {
        return std::make_shared<push_size>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(alternatives p) {
        return std::make_shared<alternatives>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(optional p) {
        return std::make_shared<optional>(p);
    }
    
    inline ptr<pattern> pattern::sequence::construct(pattern p) {
        return std::make_shared<pattern>(p);
    }
    
    template <typename X> 
    inline list<ptr<pattern>> pattern::sequence::make(X x) {
        return list<ptr<pattern>>{}.prepend(construct(x));
    }
    
    template <typename X, typename... P>
    inline list<ptr<pattern>> pattern::sequence::make(X x, P... p) {
        return make(p...).prepend(construct(x));
    }
    
}

#endif
