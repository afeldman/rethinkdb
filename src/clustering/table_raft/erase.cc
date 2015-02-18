// Copyright 2010-2015 RethinkDB, all rights reserved.
#include "clustering/table_raft/erase.hpp"

namespace table_raft {

erase_t::erase_t(
        const server_id_t &sid,
        store_view_t *s,
        UNUSED branch_history_manager_t *bhm,
        const region_t &r,
        UNUSED perfmon_collection_t *perfmons,
        const contract_t &contract,
        const std::function<void(contract_ack_t)> &ack_cb) :
    server_id(sid), store(s), region(r)
{
    guarantee(s->get_region() == region);
    guarantee(contract.replicas.count(server_id) == 0);
    ack_cb(contract_ack_t(contract_ack_t::state_t::nothing));
    coro_t::spawn_sometime(std::bind(&erase_t::run, this, drainer.lock()));
}

void erase_t::update_contract(
        const contract_t &contract,
        const std::function<void(contract_ack_t)> &ack_cb) {
    guarantee(contract.replicas.count(server_id) == 0);
    ack_cb(contract_ack_t(contract_ack_t::state_t::nothing));
}

void erase_t::run(auto_drainer_t::lock_t keepalive) {
    try {
        store->reset_data(
            binary_blob_t(version_t::zero()),
            region,
            write_durability_t::HARD,
            keepalive.get_drain_signal());
    } catch (const interrupted_exc_t &) {
        /* do nothing */
    }
}

} /* namespace table_raft */

