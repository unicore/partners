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

    [[eosio::action]] void setstatus(eosio::name username, eosio::name status);

    static eosio::name get_random_headman();
    static uint64_t get_head_count();
    static void upcounts(eosio::name username, uint64_t level);


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


     struct [[eosio::table]] userscount2
     {
        uint64_t level;
        uint64_t count;
         
        uint64_t primary_key() const {return level;}
        
        
        EOSLIB_SERIALIZE(userscount2, (level)(count))
     }; 

    typedef eosio::multi_index<"userscount2"_n, userscount2> userscount2_index;


    eosio::checksum256 hashit(std::string str) const
      {
          return eosio::sha256(const_cast<char*>(str.c_str()), str.size());
      }
        

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

        
    typedef eosio::multi_index<"partners"_n, partners,
      eosio::indexed_by<"byreferer"_n, eosio::const_mem_fun<partners, uint64_t, &partners::byreferer>>,
      eosio::indexed_by<"byid"_n, eosio::const_mem_fun<partners, uint64_t, &partners::byid>>,
      eosio::indexed_by<"bynickhash"_n, eosio::const_mem_fun<partners, eosio::checksum256, &partners::bynickhash>>
    > partners_index;


    struct [[eosio::table]] partners2 {
        eosio::name username;
        eosio::name referer;
        std::string nickname;
        eosio::checksum256 nickhash;
        
        uint64_t id;
        uint64_t cashback;
        
        eosio::name status;
        
        std::string meta;
        
        uint64_t primary_key() const{return username.value;}
        uint64_t byreferer() const{return referer.value;}
        uint64_t byid() const {return id;}
        uint64_t bystatus() const {return status.value;}

        eosio::checksum256 bynickhash() const { return nickhash; }
        
        EOSLIB_SERIALIZE(partners2, (username)(referer)(nickname)(nickhash)(id)(cashback)(status)(meta))


    };
      
    typedef eosio::multi_index<"partners2"_n, partners2,
      eosio::indexed_by<"byreferer"_n, eosio::const_mem_fun<partners2, uint64_t, &partners2::byreferer>>,
      eosio::indexed_by<"byid"_n, eosio::const_mem_fun<partners2, uint64_t, &partners2::byid>>,
      eosio::indexed_by<"bynickhash"_n, eosio::const_mem_fun<partners2, eosio::checksum256, &partners2::bynickhash>>,
      eosio::indexed_by<"bystatus"_n, eosio::const_mem_fun<partners2, uint64_t, &partners2::bystatus>>
    > partners2_index;



    struct [[eosio::table]] headmans {
        uint64_t id;
        eosio::name username;
        std::string meta;

        uint64_t primary_key() const{return id;}
        uint64_t byusername() const {return username.value;}

        EOSLIB_SERIALIZE(headmans, (id)(username)(meta))
    };


    typedef eosio::multi_index<"headmans"_n, headmans,
      eosio::indexed_by<"byusername"_n, eosio::const_mem_fun<headmans, uint64_t, &headmans::byusername>>
    > headmans_index;


};
