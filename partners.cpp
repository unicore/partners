#include "partners.hpp"
#include <eosio/transaction.hpp>

using namespace eosio;
/**
\defgroup public_consts CONSTS
\brief Константы контракта
*/

/**
\defgroup public_actions ACTIONS
\brief Методы действий контракта
*/

/**
\defgroup public_tables TABLES
\brief Структуры таблиц контракта
*/

   /**
   * @brief      Метод обновления профиля
   * @ingroup public_actions
   * @auth username 
   * @details Операция обновления профиля позволяет изменить мета-данные аккаунта. 
   * @param[in]  username  имя аккаунта пользователя для обновления
   * @param[in]  meta  строковый JSON-объект мета-данных профиля пользователя
   */
  [[eosio::action]] void part::profupdate(eosio::name username, std::string meta){
    require_auth(username);
    partners2_index refs(_me, _me.value);

    auto ref = refs.find(username.value);
    
    refs.modify(ref, username, [&](auto &r){
        r.meta = meta;
    });
  }

/**
   * @brief      Метод установки статуса
   * @ingroup public_actions
   * @auth _me 
   * @details Метод устанавливает специальный статус лидера (headman) для партнёра. 
   * Обладая статусом headman, все свободные регистрации новых партнёров (без указания referer) устанавливаются под лидеров в случайном порядке. 
   * @param[in]  username  имя аккаунта пользователя для обновления статуса
   * @param[in]  status  имя статуса партнёра (сейчас доступен только headman)
   */
[[eosio::action]] void part::setstatus(eosio::name username, eosio::name status) {
    require_auth(_me);

    partners2_index partners(_me, _me.value);
    auto partner = partners.find(username.value);
    eosio::check(partner != partners.end(), "Partner is not found");


    headmans_index headmans(_me, _me.value);

    if (partner -> status == "headman"_n && status == ""_n){
        auto byusername_idx = headmans.template get_index<"byusername"_n>();
        auto exist_headman = byusername_idx.find(username.value);   

        byusername_idx.erase(exist_headman);
        
        partners.modify(partner, _me, [&](auto &p){
            p.status = status;
        });

    } else {
        eosio::check(status == "headman"_n, "Only headman status is available now");

        partners.modify(partner, _me, [&](auto &p){
            p.status = status;
        });

        uint64_t id = headmans.available_primary_key();
        if (id == 0)
            id = 1;

        headmans.emplace(_me, [&](auto &f){
            f.id = id;
            f.username = username;
        });
    
    }
    
};


[[eosio::action]] void part::delheadman(uint64_t id) {
    require_auth(_me);

    headmans_index headmans(_me, _me.value);
    auto h = headmans.find(id);
    headmans.erase(h);

};



    eosio::name part::get_random_headman(){
        uint64_t from = 0;
        uint64_t to = 0;
        uint64_t r = 0;

        headmans_index headmans(_me, _me.value);

        auto headman_begin = headmans.begin();

        if (headman_begin != headmans.end())
            from = headman_begin -> id;
        
        if (from != 0) {
            auto headman_end = headmans.end();
            headman_end--;
            to = headman_end -> id;   
        }
        
        if (from != to) {
            checksum256 result;
            auto mixedBlock = tapos_block_prefix() * tapos_block_num();

            const char *mixedChar = reinterpret_cast<const char *>(&mixedBlock);
            result = eosio::sha256((char *)mixedChar, sizeof(mixedChar));

            const char *p64 = reinterpret_cast<const char *>(&result);
            
            r = (abs((int64_t)p64[0]) % to) + from;    
            print(" ", r);    

        } else {
            r = from;
        }
        
        
        eosio::name headman;

        if (r == 0) {

            headman = _me;

        } else {

           headman = headmans.find(r) -> username; 

        };
        
        return headman;
        
    };


/**
 * @brief      Регистрация пользователя в реферальном дереве связей
 * @ingroup public_actions
 * @auth username | _me | _registrator
 * @details Устанавливает пользователя в глобальное реферальное дерево связей. Изменить положение в дереве связей могут аккаунты _me или _registrator.
 * 
 *
 * @param[in]  username  Имя аккаунта для регистрации
 * @param[in]  referer  Имя аккаунта партнёра, осуществившего приглашение
 * @param[in]  meta  строковый JSON-объект мета-данных профиля пользователя
 */

[[eosio::action]] void part::reg(eosio::name username, eosio::name referer, std::string meta) {

    eosio::check(has_auth(username) || has_auth(_me) || has_auth(_registrator), "missing required authority");

    eosio::check( is_account( username ), "User account does not exist");
    
    eosio::name payer =  has_auth(username) ? username : (has_auth(_registrator) ? _registrator : _me) ;
   
    partners2_index refs(_me, _me.value);
    auto ref = refs.find(username.value);

    eosio::check(username != referer, "You cant set the referer yourself");
    


    if (referer.value != 0) {
        eosio::check( is_account( referer ), "Referer account does not exist");        
    } else {

        if (username != _me)
            referer = part::get_random_headman();
            
    }
    
    //TODO eosio::check account registration;
    
    if (ref == refs.end()) {
        auto byid = refs.template get_index<"byid"_n>();
        uint64_t id = 0;

        auto byid_txn = byid.crbegin();
        
        if (byid_txn != byid.rend()){
            id = byid_txn -> id + 1;
        };
        
        refs.emplace(payer, [&](auto &r){
            r.id = id;
            r.username = username;
            r.referer = referer;
            r.meta = meta;
        });

    } else {
        eosio::check(!has_auth(username), "only registrator can change referer "); //only registrator can change referer

        refs.modify(ref, payer, [&](auto &r){
            r.referer = referer;
        });
    }

};


/**
 * @brief      Регистрация пользователя в вторичном реферальном дереве связей
 * @ingroup public_actions
 * @auth username | _me | _registrator
 * @details Устанавливает пользователя в локальное реферальное дерево связей. Изменить положение в дереве связей могут аккаунты _me или _registrator.
 * 
 *
 * @param[in]  username  Имя аккаунта для регистрации
 * @param[in]  referer  Имя аккаунта партнёра, осуществившего приглашение
 * @param[in]  meta  строковый JSON-объект мета-данных профиля пользователя
 */

[[eosio::action]] void part::reg2(eosio::name host, eosio::name username, eosio::name referer, std::string meta) {

    eosio::check(has_auth(username) || has_auth(_me) || has_auth(_registrator), "missing required authority");

    eosio::check( is_account( username ), "User account does not exist");
    
    eosio::name payer =  has_auth(username) ? username : (has_auth(_registrator) ? _registrator : _me) ;
   
    partners2_index refs(_me, host.value);
    auto ref = refs.find(username.value);

    eosio::check(username != referer, "You cant set the referer yourself");
    


    if (referer.value != 0) {
        eosio::check( is_account( referer ), "Referer account does not exist");        
    } else {

        if (username != _me)
            referer = part::get_random_headman();
            
    }
    if (referer == _me)
        referer = host;

    //TODO eosio::check account registration;
    
    if (ref == refs.end()) {
        auto byid = refs.template get_index<"byid"_n>();
        uint64_t id = 0;

        auto byid_txn = byid.crbegin();
        
        if (byid_txn != byid.rend()){
            id = byid_txn -> id + 1;
        };
        
        refs.emplace(payer, [&](auto &r){
            r.id = id;
            r.username = username;
            r.referer = referer;
            r.meta = meta;
        });

    } else {
        eosio::check(!has_auth(username), "only registrator can change referer "); //only registrator can change referer

        refs.modify(ref, payer, [&](auto &r){
            r.referer = referer;
        });
    }

};

/**
 * @brief      Метод удаления аккаунта из дерева связей
 * @ingroup public_actions
 * @auth _me
 * @details Метод удаляет аккаунт из дерева связей. Используется только администратором контракта в исключительных случаях. 
 * @param[in]  username  Имя пользователя для удаления
 * 
 */
[[eosio::action]] void part::del(eosio::name username){
    require_auth(_me);
    partners2_index refs(_me, _me.value);
    auto u = refs.find(username.value);
    refs.erase(u);
}

void part::add_promo_budget(eosio::name username, eosio::name code, eosio::asset quantity){
    pbudgets_index pbudgets(_me, _me.value);

    auto byusername_idx = pbudgets.template get_index<"byusername"_n>();
    auto exist = byusername_idx.find(username.value);   

    if (exist == byusername_idx.end()){
        pbudgets.emplace(_me, [&](auto &pb){
            pb.id = pbudgets.available_primary_key();
            pb.contract = code;
            pb.username = username;
            pb.budget = quantity;
        });
    } else {
        byusername_idx.modify(exist, _me, [&](auto &pb){
            pb.budget += quantity;
        });
    }

}


 [[eosio::action]] void part::request( eosio::name to, eosio::name from) {
     require_auth( to );


     // auto prequest = prequests.find(to.value);

     // eosio::check(prequest == prequests.end(), "Tokens already requested from this account");

     pbudgets_index pbudgets(_me, _me.value);

     auto byusername_idx = pbudgets.template get_index<"byusername"_n>();
     auto exist = byusername_idx.find(from.value);   

     
     eosio::check(exist != byusername_idx.end(), "Promoter is not found");
     
     eosio::asset quantity = asset(0, exist -> budget.symbol);
     
     if (exist -> contract == "faketoken"_n) {
        quantity.amount = 10000000;
     } else {
        quantity.amount = 10000;
     }

     eosio::check(exist -> budget.amount >= quantity.amount, "Not enough budget of sponsor");


     prequests_index prequests(_me, _me.value);

     auto prequests_index = prequests.template get_index<"byconuser"_n>();
     auto prequests_ids = combine_ids(exist->contract.value, to.value);

     auto prequest = prequests_index.find(prequests_ids);

     if (prequest == prequests_index.end()) {

         byusername_idx.modify(exist, to, [&](auto &b){
            b.budget -= quantity;
         });

        action(
            permission_level{ _me, "active"_n },
            exist->contract, "transfer"_n,
            std::make_tuple( _me, to, quantity, std::string("gift from sponsor")) 
        ).send();


         prequests.emplace(_me, [&](auto &r){
            r.id = prequests.available_primary_key();
            r.contract = exist -> contract;
            r.username = to;
            r.promoter = from;
            r.amount = quantity;
         });
     }
  }

extern "C" {
   
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == part::_me.value) {
          if (action == "reg"_n.value) {
            execute_action(name(receiver), name(code), &part::reg);
          } if (action == "reg2"_n.value){
            execute_action(name(receiver), name(code), &part::reg2);
          } else if (action == "del"_n.value) {
            execute_action(name(receiver), name(code), &part::del);
          }  else if (action == "profupdate"_n.value){
            execute_action(name(receiver), name(code), &part::profupdate);
          }  else if (action == "setstatus"_n.value){
            execute_action(name(receiver), name(code), &part::setstatus);
          }  else if (action == "delheadman"_n.value){
            execute_action(name(receiver), name(code), &part::delheadman);
          }  else if (action == "request"_n.value){
            execute_action(name(receiver), name(code), &part::request);
          }  

        } else {
          if (action == "transfer"_n.value){
            
            struct transfer{
                eosio::name from;
                eosio::name to;
                eosio::asset quantity;
                std::string memo;
            };

            auto op = eosio::unpack_action_data<transfer>();

            if (op.to == part::_me){
              part::add_promo_budget(op.from, name(code), op.quantity);
            }
          }
        }
  };
};
