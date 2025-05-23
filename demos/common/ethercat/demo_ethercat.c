
#include <ecrt.h>
#include <prt_sys.h>
#include <prt_clk.h>
#include <prt_sys_external.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <print.h>
#include "hr_delay.h"

extern int ethercat_init();
extern bool wait_for_slave_respond();
extern bool wait_for_slave_scan_complete();

#define slavePos 0, 0
#define ASDA 0x00100000, 0x000c0112
#define frequency 1000000ULL // DC 125us

static ec_master_t *master = NULL;
static ec_domain_t *domain = NULL;
static ec_domain_state_t domain_state = {};
static uint8_t *domain_pd = NULL;

#define TIMESPEC2NS(T) ((uint64_t)(T).tv_sec * 1000000000 + (T).tv_nsec)

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x6040, 0x00, 16}, /* Control Word */
    {0x607A, 0x00, 32}, /* Target Position */
    {0x60FF, 0x00, 32}, /* Target Velocity */
    {0x6071, 0x00, 16}, /* Target Torque */
    {0x6060, 0x00, 8},  /* Profile Velocity */
    {0x60B8, 0x00, 16}, /* Profile Acceleration */
    {0x607F, 0x00, 32},

    {0x603F, 0x00, 16},
    {0x6041, 0x00, 16},
    {0x6064, 0x00, 32},
    {0x6077, 0x00, 16},
    {0x6061, 0x00, 8},
    {0x60B9, 0x00, 16},
    {0x60BA, 0x00, 32},
    {0x60BC, 0x00, 32},
    {0x60FD, 0x00, 32},
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 7, slave_0_pdo_entries + 0},
    {0x1A00, 9, slave_0_pdo_entries + 7},
};

ec_sync_info_t slave_0_syncs[] = {{0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
                                  {1, EC_DIR_INPUT, 0, NULL, EC_WD_DISABLE},
                                  {2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_ENABLE},
                                  {3, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
                                  {0xff}};

/* Offsets for PDO entries */
typedef struct
{
    // RxPDO
    unsigned int control_word;                // 0x6040:控制字,subindex:0,bitlen:16
    unsigned int target_position;             // 0x607A:目标位置,subindex:0,bitlen:32
    unsigned int touch_probe;                 // 0x60B8:探针,subindex:0,bitlen:16
    unsigned int pysical_outputs;             // 0x60FE:pysical_outputs,subindex:1,bitlen:32
    unsigned int target_velocity;             // 0x60FF:target_velocity,subindex:0,bitlen:32
    unsigned int target_torque;               // 0x6071:int target_torque,subindex:0,bitlen:16
    unsigned int modes_operation;             // 0x6060:Modes of operation,subindex:0,bitlen:8
    unsigned int max_profile_velocity;        // 0x607F:max_profile_velocity,subindex:0,bitlen:32
    unsigned int positive_torque_limit_value; // 0x60E0:positive_torque_limit_value,subindex:0,bitlen:16
    unsigned int negative_torque_limit_value; // 0x60E1:negaitive_torque_limit_value,subindex:0,bitlen:16
    unsigned int torque_offset;               // 0x60B2:torque offset,subindex:0,bitlen:16

    // TxPDo
    unsigned int status_word;                  // 0x6041:status_word,subindex:0,bitlen:16
    unsigned int position_actual_value;        // 0x6064:position_actual_value,subindex:0,bitlen:32
    unsigned int touch_probe_status;           // 0x60B9,subindex:0,bitlen:16
    unsigned int touch_probe_pos1_pos_value;   // 0x60BA,subindex:0,bitlen:32
    unsigned int touch_probe_pos2_pos_value;   // 0x60BC ,subindex:0,bitlen:32
    unsigned int error_code;                   // 0x603F,subindex:0,bitlen:16
    unsigned int digital_inputs;               // 0x60FD,subindex:0,bitlen:32
    unsigned int torque_actual_value;          // 0x6077,subindex:0,bitlen:16
    unsigned int following_error_actual_value; // 0x60F4,subindex:0,bitlen:32
    unsigned int modes_of_operation_display;   // 0x6061,subindex:0,bitlen:8
    unsigned int velocity_actual_value;        // 0x606C,subindex:0,bitlen:32

    // input
    unsigned int position;
} SV630N_offset;

SV630N_offset slave_0_offset;

static ec_pdo_entry_reg_t domain0_regs[] = {
    {slavePos, ASDA, 0x6040, 0, &slave_0_offset.control_word},
    {slavePos, ASDA, 0x607A, 0, &slave_0_offset.target_position},
    {slavePos, ASDA, 0x60FF, 0, &slave_0_offset.target_velocity},
    {slavePos, ASDA, 0x6071, 0, &slave_0_offset.target_torque},
    {slavePos, ASDA, 0x6060, 0, &slave_0_offset.modes_operation},
    {slavePos, ASDA, 0x60B8, 0, &slave_0_offset.touch_probe},
    {slavePos, ASDA, 0x607F, 0, &slave_0_offset.max_profile_velocity},

    {slavePos, ASDA, 0x603F, 0, &slave_0_offset.error_code},
    {slavePos, ASDA, 0x6041, 0, &slave_0_offset.status_word},
    {slavePos, ASDA, 0x6064, 0, &slave_0_offset.position_actual_value},
    {slavePos, ASDA, 0x6077, 0, &slave_0_offset.torque_actual_value},
    {slavePos, ASDA, 0x6061, 0, &slave_0_offset.modes_of_operation_display},
    {slavePos, ASDA, 0x60B9, 0, &slave_0_offset.touch_probe_status},
    {slavePos, ASDA, 0x60BA, 0, &slave_0_offset.touch_probe_pos1_pos_value},
    {slavePos, ASDA, 0x60BC, 0, &slave_0_offset.touch_probe_pos2_pos_value},
    {slavePos, ASDA, 0x60FD, 0, &slave_0_offset.digital_inputs},
    {}};

void check_domain_state(void)
{
    ec_domain_state_t ds;
    ecrt_domain_state(domain, &ds);
    // 从站读取 wkc = 2, 从站读取写入 wkc = 3
    if (ds.working_counter != domain_state.working_counter)
    {
        printf("发生了不完整的数据帧传输，当前工作计数器为%u\n", ds.working_counter);
    }
    if (ds.wc_state != domain_state.wc_state)
    {
        printf("工作计数器状态改变为%u\n", ds.wc_state);
        domain_state = ds;
    }
}

// 发送消息的频率设置
// 计时的时候需要使用到
struct period_info
{
    U64 next_period;
    U32 period_cycles;
};

static inline void inc_period(struct period_info *pinfo)
{
    pinfo->next_period += pinfo->period_cycles;
}

static void periodic_task_init(struct period_info *pinfo)
{
    /* for simplicity, hardcoding a 10ms period */
    pinfo->period_cycles = frequency * (U64)OsSysGetClock() / 1000000000;
    pinfo->next_period = PRT_ClkGetCycleCount64();
}

static void wait_rest_of_period(struct period_info *pinfo)
{
    inc_period(pinfo);

    S64 delay = pinfo->next_period - PRT_ClkGetCycleCount64();
    if (delay <= 0)
    {
        PRT_Printf("delay < 0\n");
        exit(1);
    }

    hrdelay((U32)delay);
}

typedef enum
{
    SERVO_STAT_SWION_DIS = 0x40,
    SERVO_STAT_RDY_SWION = 0x21,
    SERVO_STAT_SWION_ENA = 0x23,
    SERVO_STAT_OP_ENA = 0x27
} sv630nRunState;

int handleTask(SV630N_offset *targetSlave, uint8_t **domain1_pd)
{
    static int initstate = 0x4F;
    uint16_t state = 0;
    static uint32_t pos;
    state = EC_READ_U16(*domain1_pd + targetSlave->status_word);
    // error_code = EC_READ_U16(*domain1_pd + targetSlave->error_code);
    pos = EC_READ_U32(*domain1_pd + targetSlave->position_actual_value);
    // PRT_Printf( " status_word: %x \n  error_code: %x \n",state,error_code);
    switch (state & initstate)
    { // 伺服初始化状态机
    case SERVO_STAT_SWION_DIS:
        EC_WRITE_U16(*domain1_pd + targetSlave->control_word, 0x0006);
        EC_WRITE_U8(*domain1_pd + targetSlave->modes_operation, 0x0008);
        EC_WRITE_U32(*domain1_pd + targetSlave->target_position, pos);
        initstate = 0x6f;
        break;
    case SERVO_STAT_RDY_SWION:
        EC_WRITE_U16(*domain1_pd + targetSlave->control_word, 0x0007);
        EC_WRITE_U8(*domain1_pd + targetSlave->modes_operation, 0x0008);
        EC_WRITE_U32(*domain1_pd + targetSlave->target_position, pos);
        initstate = 0x6f;
        break;
    case SERVO_STAT_SWION_ENA:
        EC_WRITE_U16(*domain1_pd + targetSlave->control_word, 0x000f);
        EC_WRITE_U8(*domain1_pd + targetSlave->modes_operation, 0x0008);
        EC_WRITE_U32(*domain1_pd + targetSlave->target_position, pos);
        initstate = 0x6f;
        break;
    case SERVO_STAT_OP_ENA:
        EC_WRITE_U8(*domain1_pd + targetSlave->modes_operation, 8);
        EC_WRITE_U32(*domain1_pd + targetSlave->target_position, pos + 1000);
        break;
    default:
        break;
    }
    return 0;
}

void cyclic_task(ec_master_t *master, unsigned slave_cnt, ec_domain_t *domain1, uint8_t **domain1_pd)
{
    struct period_info pinfo;
    periodic_task_init(&pinfo);
    // U64 start, end;

    while (1)
    {
        wait_rest_of_period(&pinfo);

        // start = PRT_ClkGetCycleCount64();
        ecrt_master_application_time(master, PRT_ClkCycle2Ns(pinfo.next_period));

        ecrt_master_receive(master);
        ecrt_domain_process(domain1);

        handleTask(&slave_0_offset, domain1_pd);

        ecrt_master_sync_reference_clock(master);
        ecrt_master_sync_slave_clocks(master);

        ecrt_domain_queue(domain1);
        ecrt_master_send(master);
        // end = PRT_ClkGetCycleCount64();
        // PRT_Printf("%u cycle time: %llu ns\n", __LINE__, (end - start)*1000/24);
    }
}

int init_ethercat()
{
    // 启动主站
    ec_slave_config_t *sc;
    int ret;

    master = ecrt_request_master(0);
    if (!master)
    {
        PRT_Printf("[DEMO] request master fail\n");
        return -1;
    }

    // start configuration
    domain = ecrt_master_create_domain(master);
    if (!domain)
    {
        return -1;
    }

    if (!(sc = ecrt_master_slave_config(master, slavePos, ASDA)))
    {
        printf("[DEMO] slave config fail\n");
        return -1;
    }

    PRT_Printf("[DEMO] config slave PDOS\n");
    if (ecrt_slave_config_pdos(sc, EC_END, slave_0_syncs))
    {
        PRT_Printf("[DEMO] config PDS fail\n");
        return -1;
    }
    // 在domain中注册PDO条目
    PRT_Printf("[DEMO] reg PDOS entry\n");
    if (ecrt_domain_reg_pdo_entry_list(domain, domain0_regs))
    {
        PRT_Printf("[DEMO] PDO reg fail\n");
        return -1;
    }

    ecrt_slave_config_dc(sc, 0x300, frequency, 4400000, 0, 0);

    PRT_Printf("===============\n");
    ret = 0;
    ret += ecrt_slave_config_sdo8(sc, 0x1C12, 0, 0);
    ret += ecrt_slave_config_sdo8(sc, 0x1C13, 0, 0);

    ret += ecrt_slave_config_sdo16(sc, 0x1C12, 1, 0x1600);
    ret += ecrt_slave_config_sdo8(sc, 0x1C12, 0, 1);

    ret += ecrt_slave_config_sdo16(sc, 0x1C13, 1, 0x1A00);
    ret += ecrt_slave_config_sdo8(sc, 0x1C13, 0, 1);

    ret += ecrt_slave_config_sdo8(sc, 0x6060, 0, 8);
    PRT_Printf("======ret:%d=========\n", ret);

    // 激活master
    if (ecrt_master_activate(master))
    {
        PRT_Printf("activate master failed\n");
        return -1;
    }
    PRT_Printf("activate master success\n");

    if (!(domain_pd = ecrt_domain_data(domain)))
    {
        PRT_Printf("get domain data fail\n");
        ecrt_release_master(master);
        return -1;
    }
    PRT_Printf("ecrt_domain_data success\n");
    cyclic_task(master, 1, domain, &(domain_pd));

    return 0;
}

void test_ethercat_main()
{
    int ret;
    PRT_Printf("[DEMO] test_ethercat_main enter\n");
    ret = init_ethercat();
    if (ret)
    {
        PRT_Printf("Failed to init EtherCAT master.\n");
        if (master)
        {
            ecrt_release_master(master);
        }
        return;
    }
    // printf("master init success\n");
    // simple_cyclic_task();
    // ecrt_release_master(master);
    printf("Finish.\n");
}

void ethercat_demo()
{
    int ret;

    PRT_Printf("ethercat demo!\n");

    ret = TIMER_Init();
    if (ret != 0)
    {
        PRT_Printf("set hr_timer failed.\n");
        return;
    }
    // ret = rpmsg_service_init();
    // if (ret) {
    //     return ret;
    // }
    ethercat_init();

    if (!wait_for_slave_respond())
    {
        PRT_Printf("[TEST] no slave responding!\n");
        return;
    }

    if (!wait_for_slave_scan_complete())
    {
        PRT_Printf("[TEST] slave scan not compete yet!\n");
        return;
    }

    PRT_Printf("[TEST] detect slave\n");
    test_ethercat_main();
}

void cycle_task_jitter()
{
    TIMER_Init();

    U64 freq = 1000000ULL;
    struct period_info pinfo;
    pinfo.period_cycles = freq * (U64)OsSysGetClock() / 1000000000;
    pinfo.next_period = PRT_ClkGetCycleCount64();
    U64 cycle_per_ms = OsSysGetClock() / 1000;

    U64 total = 0, max = 0;
    U64 times = 100000;

    U64 start, end;
    U64 escape, jitter;
    for (int i = 0; i < times; i++)
    {
        start = PRT_ClkGetCycleCount64();
        wait_rest_of_period(&pinfo);
        end = PRT_ClkGetCycleCount64();

        escape = end - start;
        if (escape > cycle_per_ms)
        {
            jitter = escape - cycle_per_ms;
        }
        else
        {
            jitter = cycle_per_ms - escape;
        }

        total += jitter;
        if (jitter > max)
        {
            max = jitter;
        }
    }

    PRT_Printf("1ms cycle task jitter avg %llu ns, max %llu ns.\n", (total / times) * 1000 / 24, max * 1000 / 24);
}