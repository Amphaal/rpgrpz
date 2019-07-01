#pragma once

#include <stdint.h>
#include <time.h>
#include <stdexcept>
#include <mutex>

#include <QDebug>

#include <sys/time.h>

typedef uint64_t snowflake_uid;

class SnowFlake {

    public:
        static SnowFlake* get();
        snowflake_uid nextId();

    private:
        static const uint64_t start_stmp_ = 1480166465631;
        static const uint64_t sequence_bit_ = 12;
        static const uint64_t machine_bit_ = 5;
        static const uint64_t datacenter_bit_ = 5;

        static const uint64_t max_datacenter_num_ = -1 ^ (uint64_t(-1) << datacenter_bit_);
        static const uint64_t max_machine_num_ = -1 ^ (uint64_t(-1) << machine_bit_);
        static const uint64_t max_sequence_num_ = -1 ^ (uint64_t(-1) << sequence_bit_);

        static const uint64_t machine_left = sequence_bit_;
        static const uint64_t datacenter_left = sequence_bit_ + machine_bit_;
        static const uint64_t timestmp_left = sequence_bit_ + machine_bit_ + datacenter_bit_;

        uint64_t datacenterId;
        uint64_t machineId;
        uint64_t sequence;
        uint64_t lastStmp;

        std::mutex mutex_;

        uint64_t getNextMill();
        uint64_t getNewstmp();

        static inline SnowFlake* _self = nullptr;
        SnowFlake(int datacenter_Id, int machine_Id);
}; 