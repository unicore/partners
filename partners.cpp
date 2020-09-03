#include "partners.hpp"

using namespace eosio;

    /**
   * @brief      Метод обновления профиля
   * Операция обновления профиля позволяет изменить мета-данные аккаунта. 
   * @param[in]  op    The operation
   */
  [[eosio::action]] void part::profupdate(eosio::name username, std::string meta){
    require_auth(username);
    partners_index refs(_me, _me.value);

    auto ref = refs.find(username.value);
    
    refs.modify(ref, username, [&](auto &r){
        r.meta = meta;
    });
  }


/**
 * @brief      Регистрация пользователя в системе. 
 * Характеризуется созданием профиля с ссылкой на приглашающий аккаунт. Приглашающий аккаунт используется в качестве связи для вычисления партнерских структур различного профиля.
 * 
 * TODO ввести порядковые номера  
 *
 * @param[in]  op    The operation
 */

[[eosio::action]] void part::reg(eosio::name username, eosio::name referer, std::string meta) {
    
    eosio::check(has_auth(username) || has_auth(_me), "missing required authority");

    eosio::check( is_account( username ), "User account does not exist");
    
     
    partners_index refs(_me, _me.value);
    auto ref = refs.find(username.value);

    eosio::check(username != referer, "You cant set the referer yourself");
    
    if (!has_auth(_me)){
        eosio::check(referer.value != 0, "Registration without referer is not possible");
        eosio::check( is_account( referer ), "Referer account does not exist");
        auto pref = refs.find(referer.value);
        eosio::check(pref != refs.end(), "Referer is not registered in the core");    
    } 

    
    userscount_index usercounts(_me, _me.value);
    auto usercount = usercounts.find("registered"_n.value);
    uint64_t count = 1;


    //TODO eosio::check account registration;
    
    if (ref == refs.end()){
        if (usercount == usercounts.end()){
            usercounts.emplace(_me, [&](auto &u){
                u.cname = "registered"_n;
                u.count = count;
            });
        } else {
            count =  usercount -> count + 1;
            usercounts.modify(usercount, _me, [&](auto &u){
                u.count = count;
            });
        };

        refs.emplace(_me, [&](auto &r){
            r.id = count;
            r.username = username;
            r.referer = referer;
            r.meta = meta;
        });
    } else {
        require_auth(_me); //only registrator can change referer

        refs.modify(ref, _me, [&](auto &r){
            r.referer = referer;
        });
    }
};


[[eosio::action]] void part::del(eosio::name username){
    require_auth(_me);
    partners_index refs(_me, _me.value);
    auto u = refs.find(username.value);
    refs.erase(u);
    userscount_index usercounts(_me, _me.value);
    auto usercount = usercounts.find("registered"_n.value);
    usercounts.modify(usercount, _me, [&](auto &u){
        u.count = u.count - 1;
    });

}




extern "C" {
   
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == _me.value) {
          if (action == "reg"_n.value){
            execute_action(name(receiver), name(code), &part::reg);
          }  else if (action == "del"_n.value){
            execute_action(name(receiver), name(code), &part::del);
          }  else if (action == "profupdate"_n.value){
            execute_action(name(receiver), name(code), &part::profupdate);
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

            if (op.to == _me){
              //Do the stuff
            }
          }
        }
  };
};
