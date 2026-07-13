/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_polling.c
  * @brief          : Modbus 轮询任务 — 顺序扫描地址 0-128
  *
  * 任务流：
  *   TaskModbusPoll (每 50ms)
  *     └── 构造 FC 0x02 请求 (slave_addr=current, 130 bits)
  *           └── ModbusMaster_EnqueueRequest() → [xMasterSendQueue]
  *                 └── 发送任务发帧 → Controlboard 回复 → 接收任务解析
  *
  * 地址回绕时 (current_addr > 128)：
  *   └── HostReg_StepCycle() — 更新零地址标志和重复地址标志
  *
  * 参考 Controlboard/Core/Src/modbus_polling.c 架构。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "modbus_polling.h"
#include "uart1_modbus_master.h"
#include "hostboard_registers.h"
#include "dwin.h"
#include "detail_view_data.h"
/* USER CODE BEGIN 0 */
#include "main.h"       /* LED3_Pin / LED3_GPIO_Port */

/* LED3 闪烁间隔：每轮询多少个地址翻转一次（可调）*/
#define LED3_BLINK_INTERVAL  16U

/* ==================== 详情采集状态机 ==================== */

typedef enum {
    DETAIL_IDLE       = 0,  /* selectAddr == 0，不执行 */
    DETAIL_NEED_COIL  = 1,  /* 需要读选中控制器的全量线圈 */
    DETAIL_NEXT_TYPE  = 2,  /* 读下一个在线传感器的型号 */
    DETAIL_NEXT_DATA  = 3,  /* 读下一个在线传感器的数据 */
} DetailState_t;

static uint8_t  s_detail_poll_interval;  /* 插队间隔计数器 */
static uint8_t  s_detail_state;          /* 采集阶段 */
static uint8_t  s_detail_next;           /* 下一个要采集的传感器索引 (1-63) */
static uint8_t  s_detail_last_addr;      /* 上次采集的控制器地址，用于检测切换 */

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ==================== 详情采集静态函数 ==================== */

/**
  * @brief  详情采集状态机一步，由轮询任务每 3 次调用一次
  * @param  selectAddr  当前屏幕选中的控制器地址（0 = 未选中）
  */
static void DetailCollect_Step(uint8_t selectAddr)
{
    /* 检测选中控制器切换 → 重置 */
    if (selectAddr != s_detail_last_addr)
    {
        DetailView_Reset();
        s_detail_state = DETAIL_NEED_COIL;
        s_detail_next = 1;
        s_detail_last_addr = selectAddr;
    }

    switch (s_detail_state)
    {
        case DETAIL_IDLE:
            if (selectAddr != 0)
            {
                s_detail_state = DETAIL_NEED_COIL;
                s_detail_next = 1;
                DetailView_Reset();
                s_detail_last_addr = selectAddr;
            }
            break;

        case DETAIL_NEED_COIL:
            /* 离线控制器 → 跳过采集（等下次唤醒） */
            if (!HostReg_IsOnline(selectAddr))
                break;
            if (!DetailView_IsCoilReady(selectAddr))
            {
                ModbusMasterRequest_t req = {
                    .slave_addr = selectAddr,
                    .func_code  = MODBUS_FUNC_READ_DISCRETE_INPUTS,
                    .reg_addr   = 0,
                    .reg_value  = 130
                };
                ModbusMaster_EnqueueRequest(&req);
            }
            else
            {
                s_detail_state = DETAIL_NEXT_TYPE;
            }
            break;

        case DETAIL_NEXT_TYPE:
        {
            while (s_detail_next <= 63)
            {
                if (HostReg_GetCoilBit(selectAddr, (uint16_t)(s_detail_next - 1)))
                {
                    if (DetailView_GetType(s_detail_next) == 0)
                    {
                        ModbusMasterRequest_t req = {
                            .slave_addr = selectAddr,
                            .func_code  = MODBUS_FUNC_READ_HOLDING_REGISTERS,
                            .reg_addr   = s_detail_next,
                            .reg_value  = 1
                        };
                        ModbusMaster_EnqueueRequest(&req);
                        s_detail_state = DETAIL_NEXT_DATA;
                        return;
                    }
                }
                s_detail_next++;
            }
            s_detail_next = 1;
            /* 全轮采集完成 → 清缓存重新开始，实现持续刷新 */
            DetailView_Reset();
            s_detail_last_addr = selectAddr;
            s_detail_state = DETAIL_NEED_COIL;
            break;
        }

        case DETAIL_NEXT_DATA:
        {
            uint8_t n = s_detail_next;
            if (DetailView_GetType(n) != 0)
            {
                ModbusMasterRequest_t req = {
                    .slave_addr = selectAddr,
                    .func_code  = MODBUS_FUNC_READ_HOLDING_REGISTERS,
                    .reg_addr   = 64 + (uint16_t)(n - 1) * 7,
                    .reg_value  = 7
                };
                ModbusMaster_EnqueueRequest(&req);
                s_detail_next++;
                s_detail_state = DETAIL_NEXT_TYPE;
            }
            else
            {
                /* 类型未就绪（可能响应丢失）→ 跳过，下轮重试 */
                s_detail_next++;
                s_detail_state = DETAIL_NEXT_TYPE;
            }
            break;
        }
    }
}

/* ==================== 轮询任务（8+1 穿插 + 详情插队） ==================== */

void TaskModbusPoll(void *arg)
{
    (void)arg;

    /* 启动延迟：等待 Controlboard 完全启动并读好 DIP 地址，避免误检测零地址 */
    vTaskDelay(pdMS_TO_TICKS(500));

    uint16_t current_addr = 0;       /* 正常轮询地址 0-128 */
    uint8_t  poll_count    = 0;       /* 正常轮询计数（0-7） */
    uint16_t online_idx    = 1;       /* 在线控制器扫描起始 */
    uint8_t  do_online     = 0;       /* 本次是否穿插在线 */
    
    static uint8_t  s_led3_count    = 0;

    for (;;)
    {
        /* ---- 轮询间隔 50ms ---- */
        vTaskDelay(pdMS_TO_TICKS(50));

        /* ---- 详情采集插队：每 3 次轮询执行 1 次 ---- */
        uint8_t selectAddr = g_hostDwinStatus.selectAddr;
        if (selectAddr != 0 && ++s_detail_poll_interval >= 3)
        {
            s_detail_poll_interval = 0;
            DetailCollect_Step(selectAddr);
            continue;   /* 跳过本次常规轮询 */
        }

        /* ---- 构造 FC 0x02 请求（各分支设置 reg_addr/reg_value） ---- */
        ModbusMasterRequest_t req;
        req.func_code  = MODBUS_FUNC_READ_DISCRETE_INPUTS;

        /* 每 8 次正常轮询穿插 1 次在线控制器 */
        if (poll_count >= 8)
        {
            poll_count = 0;
            do_online = 1;
        }

        if (do_online)
        {
            /* ---- 在线穿插：读 67 bits（63-129）获取完整传感器状态 ---- */
            req.reg_addr   = 63;
            req.reg_value  = 67;

            uint8_t found = 0;
            for (uint16_t i = 0; i < MAX_CTRLBD_ADDR; i++)
            {
                uint16_t addr = online_idx + i;
                if (addr > MAX_CTRLBD_ADDR)
                    addr -= MAX_CTRLBD_ADDR;
                if (HostReg_IsOnline((uint8_t)addr))
                {
                    req.slave_addr = (uint8_t)addr;
                    online_idx = (uint16_t)(addr + 1);
                    if (online_idx > MAX_CTRLBD_ADDR)
                        online_idx = 1;
                    found = 1;
                    break;
                }
            }

            if (found)
            {
                ModbusMaster_EnqueueRequest(&req);
                do_online = 0;
                continue;   /* 跳过正常轮询，下次继续正常地址 */
            }
            else
            {
                /* 无在线控制器 → 跳过穿插，执行正常轮询 */
                do_online = 0;
            }
        }

        /* ---- 正常地址轮询：读 4 bits（126-129） ---- */
        req.reg_addr   = 126;
        req.reg_value  = 4;
        req.slave_addr = (uint8_t)current_addr;
        ModbusMaster_EnqueueRequest(&req);

        poll_count++;
        current_addr++;

        /* LED3 心跳：每轮询 LED3_BLINK_INTERVAL 个地址翻转一次 */
        if (++s_led3_count >= LED3_BLINK_INTERVAL)
        {
            s_led3_count = 0;
            HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
        }

        if (current_addr > MAX_CTRLBD_ADDR)
        {
            current_addr = 0;
            HostReg_StepCycle();
            poll_count = 0;     /* 新一轮开始 */
        }
    }
}

/* USER CODE END 0 */
