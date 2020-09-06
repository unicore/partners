#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/contract.hpp>
#include <eosio/crypto.hpp>
#include "consts.hpp"

// #define isDebug = TRUE
class [[eosio::contract]] part : public eosio::contract {

public:
    part( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}

    [[eosio::action]] void reg(eosio::name username, eosio::name referer, std::string meta);
    [[eosio::action]] void del(eosio::name username);
    [[eosio::action]] void profupdate(eosio::name username, std::string meta);


    void apply(uint64_t receiver, uint64_t code, uint64_t action);
    
    
    
        
    
    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };


     struct [[eosio::table]] userscount
     {
         eosio::name cname = "registered"_n;
         uint64_t count;
         
         uint64_t primary_key() const {return cname.value;}

         EOSLIB_SERIALIZE(userscount, (cname)(count))
     }; 

    typedef eosio::multi_index<"userscount"_n, userscount> userscount_index;


    struct [[eosio::table]] partners {
        eosio::name username;
        eosio::name referer;
        std::string nickname;
        eosio::checksum256 nickhash;
        
        uint64_t id;
        uint64_t cashback = 0;
        std::string meta;
        
        uint64_t primary_key() const{return username.value;}
        uint64_t byreferer() const{return referer.value;}
        uint64_t byid() const {return id;}

        eosio::checksum256 bynickhash() const { return nickhash; }
        
        EOSLIB_SERIALIZE(partners, (username)(referer)(nickname)(nickhash)(id)(cashback)(meta))


    };
      
      eosio::checksum256 hashit(std::string str) const
        {
            return eosio::sha256(const_cast<char*>(str.c_str()), str.size());
        }
        
        
    typedef eosio::multi_index<"partners"_n, partners,
      eosio::indexed_by<"byreferer"_n, eosio::const_mem_fun<partners, uint64_t, &partners::byreferer>>,
      eosio::indexed_by<"byid"_n, eosio::const_mem_fun<partners, uint64_t, &partners::byid>>,
      eosio::indexed_by<"bynickhash"_n, eosio::const_mem_fun<partners, eosio::checksum256, &partners::bynickhash>>
    > partners_index;



};
