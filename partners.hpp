#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/contract.hpp>
#include <eosio/crypto.hpp>
#include "consts.hpp"


// #define isDebug = TRUE
/**
 * @brief      Начните ознакомление здесь.
 */
class [[eosio::contract]] part : public eosio::contract {

public:
    part( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}

    [[eosio::action]] void reg(eosio::name username, eosio::name referer, std::string meta);
    [[eosio::action]] void del(eosio::name username);
    [[eosio::action]] void profupdate(eosio::name username, std::string meta);

    [[eosio::action]] void setstatus(eosio::name username, eosio::name status);

    static eosio::name get_random_headman();
    
    void apply(uint64_t receiver, uint64_t code, uint64_t action);
    
    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };


    /**
    * @ingroup public_consts 
    * @{ 
    */

    static constexpr eosio::name _me = "part"_n;              /*!< собственное имя аккаунта контракта */
    static constexpr eosio::name _registrator = "registrator"_n;  /*!< аккаунт контракта регистратора */

    /**
    * @}
    */

    eosio::checksum256 hashit(std::string str) const
      {
          return eosio::sha256(const_cast<char*>(str.c_str()), str.size());
      }
        

    //таблица устарела и не используется
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


    /**
     * @brief      Таблица хранения глобальной сети партнёров
     * @ingroup public_tables
     * @contract _me
     * @scope _me
     * @table partners
     * @details Таблица хранит реферальные связи партнёров, их профили и глобальные статусы.
     */
    struct [[eosio::table]] partners2 {
        eosio::name username; /*!< имя аккаунта пользователя */
        eosio::name referer; /*!< имя аккаунта пользователя, совершившего приглашение */
        std::string nickname; /*!< строковый никнейм пользователя */
        eosio::checksum256 nickhash; /*!< хэш, вычисленный из строкового никнейма для быстрого поиска */
        
        uint64_t id;    /*!< идентификатор численного номера регистрации */
        uint64_t cashback; /*!< параметр кэшбэка в %, используемый для возврата части собственных средств на генерирующего партнёра */
        
        eosio::name status; /*!< особый глобальный статус партнёра */
        
        std::string meta; /*!< строковый json-объект, хранящий профиль пользователя */
        
        uint64_t primary_key() const{return username.value;}        /*!< return username - primary_key */
        uint64_t byreferer() const{return referer.value;}           /*!< return referer - secondary_key 2 */
        uint64_t byid() const {return id;}                          /*!< return id - secondary_key 3 */
        eosio::checksum256 bynickhash() const { return nickhash; }  /*!< return nickhash - secondary_key 4 */
        uint64_t bystatus() const {return status.value;}            /*!< return status - secondary_key 5 */

        EOSLIB_SERIALIZE(partners2, (username)(referer)(nickname)(nickhash)(id)(cashback)(status)(meta))


    };
      
    typedef eosio::multi_index<"partners2"_n, partners2,
      eosio::indexed_by<"byreferer"_n, eosio::const_mem_fun<partners2, uint64_t, &partners2::byreferer>>,
      eosio::indexed_by<"byid"_n, eosio::const_mem_fun<partners2, uint64_t, &partners2::byid>>,
      eosio::indexed_by<"bynickhash"_n, eosio::const_mem_fun<partners2, eosio::checksum256, &partners2::bynickhash>>,
      eosio::indexed_by<"bystatus"_n, eosio::const_mem_fun<partners2, uint64_t, &partners2::bystatus>>
    > partners2_index;


    /**
     * @brief      Таблица лидеров
     * @ingroup public_tables
     * @scope _me
     * @table _me
     * @contract _me
     * @details Таблица хранит лидеров, которые получают свободные регистрации из системы в случайном порядке. 
     * Лидер получит нового партнёра, если он осуществляет регистрацию методом reg без указания реферальной связи.
     */
    struct [[eosio::table]] headmans {
        uint64_t id;              /*!< числовой идентификатор записи */
        eosio::name username;     /*!< имя акканта лидера */
        std::string meta;         /*!< мета-данные лидера */

        uint64_t primary_key() const{return id;}    /*!< return id - primary_key */
        uint64_t byusername() const {return username.value;} /*!< return username - secondary_key 2 */

        EOSLIB_SERIALIZE(headmans, (id)(username)(meta))
    };


    typedef eosio::multi_index<"headmans"_n, headmans,
      eosio::indexed_by<"byusername"_n, eosio::const_mem_fun<headmans, uint64_t, &headmans::byusername>>
    > headmans_index;


};
