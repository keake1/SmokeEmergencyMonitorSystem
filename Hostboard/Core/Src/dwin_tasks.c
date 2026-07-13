/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : dwin_tasks.c
  * @brief          : USART3 迪文屏发送/接收 FreeRTOS 任务实现
  *
  * 发送任务 (TaskDwinTx)：
  *   1. 阻塞等待 xDwinTxQueue
  *   2. HAL_UART_Transmit_IT 中断发送
  *   3. 等 xDwinTxCompleteSem（50ms 超时）
  *
  * 接收任务 (TaskDwinRx)：
  *   1. 阻塞等待 xDwinRxQueue（ISR 状态机填充）
  *   2. 将来：解析帧内容
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "dwin_tasks.h"
#include "dwin.h"
#include "usart.h"
#include "hostboard_registers.h"
#include <string.h>

/* USER CODE BEGIN 0 */
#include <stdio.h>

/* ==================== 调试：迪文屏接收帧日志环缓冲 ==================== */

#define DWIN_DBG_RX_LOG_MAX    32U   /* 最多保留 32 帧 */
#define DWIN_DBG_RX_LOG_LEN    64U   /* 每帧最大记录字节 */

typedef struct {
    uint32_t tick;                     /* 接收时的 FreeRTOS tick */
    uint16_t len;                      /* 帧实际长度 */
    uint8_t  data[DWIN_DBG_RX_LOG_LEN]; /* 帧数据（超长截断）*/
} dwin_dbg_rx_entry_t;

static dwin_dbg_rx_entry_t s_dbg_rx_log[DWIN_DBG_RX_LOG_MAX];
static uint8_t s_dbg_rx_idx = 0;      /* 环形写入索引 */

/**
 * @brief 记录一帧接收数据到调试环缓冲
 */
static void Dbg_LogRxFrame(const uint8_t *buf, uint16_t len)
{
    dwin_dbg_rx_entry_t *entry = &s_dbg_rx_log[s_dbg_rx_idx];
    uint16_t copy_len = (len < DWIN_DBG_RX_LOG_LEN) ? len : DWIN_DBG_RX_LOG_LEN;

    entry->tick = xTaskGetTickCount();
    entry->len  = len;
    memcpy(entry->data, buf, copy_len);

    s_dbg_rx_idx = (s_dbg_rx_idx + 1U) % DWIN_DBG_RX_LOG_MAX;
}

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ==================== 报警监测环缓冲与快照 ==================== */

/**
 * @brief  报警事件（由 0→1 跳变产生）
 */
typedef struct {
    uint8_t ctrlAddr;   /* 控制器地址 1-128 */
    uint8_t sensorIdx;  /* 传感器索引 0-63: 0=烟雾报警, 1-63=传感器编号 */
} alarm_event_t;

#define ALARM_EVENT_BUF_SIZE    8U

static alarm_event_t s_alarm_buf[ALARM_EVENT_BUF_SIZE];  /* 事件环缓冲 */
static uint8_t       s_alarm_wr;          /* 环缓冲写索引 */
static uint8_t       s_alarm_rd;          /* 环缓冲读索引 */
static uint8_t       s_pending_flag;      /* 1=有挂起的 RTC 请求待处理 */
static uint8_t       s_pending_ctrl;      /* 挂起事件的控制器地址 */
static uint8_t       s_pending_sensor;    /* 挂起事件的传感器索引 */
static uint64_t      s_prev_alarm[MAX_CTRLBD_ADDR + 1];  /* 上一周期报警位快照 */

/* ==================== 图标计算辅助函数 ==================== */

static uint16_t HostCalcCtrlIcon(uint8_t addr)
{
    if (!HostReg_IsOnline(addr))
        return DWIN_ICON_OFFLINE;

    if (HostReg_GetCoilBit(addr, COIL_OFFSET_ZERO_ADDR) ||
        HostReg_GetCoilBit(addr, COIL_OFFSET_ADDR_CONF))
        return DWIN_ICON_TROUBLE;

    if (HostReg_GetCoilBit(addr, COIL_GLOBAL_ALARM) ||
        HostReg_GetCoilBit(addr, COIL_SMOKE_ALARM))
        return DWIN_ICON_ALARM;

    return DWIN_ICON_NORMAL;
}

static uint16_t HostCalcSysIcon(void)
{
    if (HostReg_GetAddrConflict())
        return 3;

    if (HostReg_GetZeroAddrPresent())
        return 2;

    for (uint16_t addr = 1; addr <= MAX_CTRLBD_ADDR; addr++)
    {
        if (HostReg_IsOnline((uint8_t)addr) &&
            HostReg_GetCoilBit((uint8_t)addr, COIL_GLOBAL_ALARM))
        {
            return 1;
        }
    }
    return 0;
}

static uint16_t DetailSensorIcon(uint8_t selectAddr, uint8_t sensor_idx)
{
    /* 详情页图标值：0=离线, 1=报警, 2=正常（不与主界面图标常量共用）*/
    if (!HostReg_GetCoilBit(selectAddr, sensor_idx - 1))
        return 0;  /* offline */

    if (HostReg_GetCoilBit(selectAddr, COIL_OFFSET_ALARM + sensor_idx - 1))
        return 1;  /* alarm */

    return 2;  /* normal */
}

/* ==================== 迪文屏初始化任务（一次性） ==================== */

void TaskDwinInit(void *arg)
{
    (void)arg;
    
    vTaskDelay(pdMS_TO_TICKS(1000));  /* 等待系统稳定 */

    for (uint16_t slot = 0U; slot < DWIN_TIP_SLOT_COUNT; slot++)
    {
        uint32_t flash_addr = DWIN_TIP_FLASH_ADDR(slot);
        DWIN_NorFlashRead(flash_addr, DWIN_TIP_READ_BUF_ADDR, DWIN_TIP_WORDS_PER_SLOT);
        vTaskDelay(pdMS_TO_TICKS(50U));

        DWIN_ReadVar(DWIN_TIP_READ_BUF_ADDR, (uint8_t)DWIN_TIP_WORDS_PER_SLOT);
        vTaskDelay(pdMS_TO_TICKS(50U));
    }

    /* 通知等待的任务：连续 Give 两次，放行两个 Take */
    xSemaphoreGive(xDwinInitDoneSem);
    xSemaphoreGive(xDwinInitDoneSem);

    vTaskDelete(NULL);
}

/* ==================== USART3 发送任务 ==================== */

void TaskDwinTx(void *arg)
{
    (void)arg;
    DwinFrame_t frame;

    for (;;)
    {
        /* ---- 1. 等待发送队列 ---- */
        if (xQueueReceive(xDwinTxQueue, &frame, portMAX_DELAY) != pdPASS)
            continue;

        /* ---- 2. 中断发送 ---- */
        HAL_UART_Transmit_IT(&huart3, frame.data, frame.length);

        /* ---- 3. 等待发送完成（50ms 超时） ---- */
        xSemaphoreTake(xDwinTxCompleteSem, pdMS_TO_TICKS(50));
    }
}

/* ==================== USART3 接收任务 ==================== */

/* 全局 DWIN 解析状态 */
DWIN_HostStatus_t g_hostDwinStatus;

void TaskDwinRx(void *arg)
{
    (void)arg;
    DwinFrame_t frame;
    uint8_t cnt_tip = 0;    /* 开机恢复备注进度 */

    DwinRxReset();
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    __HAL_UART_CLEAR_FEFLAG(&huart3);
    __HAL_UART_CLEAR_NEFLAG(&huart3);
    __HAL_UART_CLEAR_PEFLAG(&huart3);
    SET_BIT(huart3.Instance->CR1, USART_CR1_RXNEIE);

    for (;;)
    {
        /* ---- 1. 等待完整帧（ISR 状态机组装后入队） ---- */
        if (xQueueReceive(xDwinRxQueue, &frame, portMAX_DELAY) != pdPASS)
            continue;

        /* 调试：记录收到的原始帧 */
        Dbg_LogRxFrame(frame.data, frame.length);

        uint8_t *buf = frame.data;
        uint16_t len = frame.length;

        /* ---- 2. 校验帧头 ---- */
        if (buf[0] != 0x5A || buf[1] != 0xA5)
            continue;

        /* 只处理 0x83 读变量应答帧 */
        if (len < 7 || buf[3] != 0x83)
            continue;

        uint16_t varAddr = ((uint16_t)buf[4] << 8) | buf[5];

        /* 备注变量区地址修正（迪文屏返回地址比实际少 1）*/
        if (varAddr >= (DWIN_TIP_VAR_ADDR - 1U) &&
            varAddr <  (DWIN_TIP_VAR_ADDR - 1U + DWIN_TIP_SLOT_COUNT * 0x10U))
        {
            varAddr += 1U;
        }

        /* ── 分支1：备注写入（屏幕下发备注内容）── */
        if (varAddr >= DWIN_TIP_VAR_ADDR &&
            varAddr <  DWIN_TIP_VAR_ADDR + DWIN_TIP_SLOT_COUNT * 0x10U)
        {
            if (len < 10U)
                continue;

            uint8_t content_len = buf[8];
            if (content_len == 0U || content_len > DWIN_TIP_CONTENT_MAX_LEN
                || len < (uint16_t)(9U + content_len))
            {
                continue;
            }

            uint16_t slot = (varAddr - DWIN_TIP_VAR_ADDR) / 0x10U;
            if (slot >= DWIN_TIP_SLOT_COUNT)
                continue;

            uint8_t store_buf[DWIN_TIP_BYTES_PER_SLOT];
            memset(store_buf, 0xFFU, sizeof(store_buf));
            store_buf[0] = content_len;
            memcpy(&store_buf[1], &buf[9], content_len);

            uint16_t crc = DWIN_CalcCRC16(store_buf, 1U + content_len);
            store_buf[1U + content_len]      = (uint8_t)(crc >> 8U);
            store_buf[1U + content_len + 1U] = (uint8_t)(crc & 0xFFU);

            uint8_t raw_bytes   = 1U + content_len + 2U;
            uint8_t align_bytes = (raw_bytes + 3U) & (uint8_t)~3U;
            if (align_bytes > DWIN_TIP_BYTES_PER_SLOT)
                align_bytes = DWIN_TIP_BYTES_PER_SLOT;

            DWIN_WriteVar(DWIN_TIP_WRITE_BUF_ADDR, store_buf, align_bytes);
            vTaskDelay(pdMS_TO_TICKS(20));

            uint32_t flash_addr = DWIN_TIP_FLASH_ADDR(slot);
            uint16_t word_len   = (uint16_t)(align_bytes / 2U);
            if (word_len & 1U) { word_len += 1U; }
            DWIN_NorFlashWrite(flash_addr, DWIN_TIP_WRITE_BUF_ADDR, word_len);
            vTaskDelay(pdMS_TO_TICKS(30));
        }
        /* ── 分支2：开机恢复（从 NorFlash 读回的备注帧写入屏幕变量）── */
        else if (varAddr == DWIN_TIP_READ_BUF_ADDR)
        {
            if (len < 9U)
                continue;

            uint8_t content_len = buf[7];
            if (content_len == 0U || content_len > DWIN_TIP_CONTENT_MAX_LEN
                || len < (uint16_t)(7U + 1U + content_len + 2U))
            {
                continue;
            }

            uint16_t crc_calc   = DWIN_CalcCRC16(&buf[7], 1U + content_len);
            uint16_t crc_stored = ((uint16_t)buf[7U + 1U + content_len] << 8U)
                                 | buf[7U + 1U + content_len + 1U];
            if (crc_calc != crc_stored)
                continue;

            uint8_t write_buf[DWIN_TIP_CONTENT_MAX_LEN];
            uint8_t write_len = content_len + 2U;
            if (write_len & 1U) { write_len += 1U; }
            memset(write_buf, 0xFFU, sizeof(write_buf));
            memcpy(write_buf, &buf[8], content_len);

            DWIN_WriteVar((uint16_t)(DWIN_TIP_VAR_ADDR + (uint16_t)cnt_tip * 0x10U),
                          write_buf, write_len);
            cnt_tip++;
        }
        /* ── 分支3：屏幕选中控制器地址 ── */
        else if (varAddr == DWIN_SELECT_CTRL_ADDR)
        {
            /* 5A A5 06 83 addrH addrL 01 dataH dataL → 共 9 字节 */
            if (len >= 9U)
            {
                g_hostDwinStatus.selectAddr = (uint8_t)(((uint16_t)buf[7] << 8U) | buf[8]);
            }
        }
        /* ── 分支4：迪文屏 RTC 时间应答帧（地址 0x0010）── */
        else if (varAddr == 0x0010U && len >= 15U)
        {
            g_hostDwinStatus.rtcYear   = buf[7];
            g_hostDwinStatus.rtcMonth  = buf[8];
            g_hostDwinStatus.rtcDay    = buf[9];
            g_hostDwinStatus.rtcHour   = buf[11];
            g_hostDwinStatus.rtcMinute = buf[12];
            g_hostDwinStatus.rtcSecond = buf[13];
            g_hostDwinStatus.rtcReady  = 1U;
        }
    }
}

/* ==================== 迪文屏图标更新任务（每 500ms） ==================== */

void TaskDwinIcons(void *arg)
{
    (void)arg;
    uint8_t buf[64];

    /* 等待迪文屏初始化完成 */
    xSemaphoreTake(xDwinInitDoneSem, portMAX_DELAY);

    for (;;)
    {
        uint8_t selectAddr = g_hostDwinStatus.selectAddr;

        if (selectAddr == 0)
        {
            /* ===== 主界面模式（现有逻辑） ===== */
            for (uint8_t frame = 0; frame < 4; frame++)
            {
                uint16_t base_addr  = DWIN_CTRL_ICON_BASE_ADDR + (uint16_t)frame * DWIN_ICONS_PER_FRAME;
                uint8_t  base_slave = frame * DWIN_ICONS_PER_FRAME + 1;

                for (uint8_t i = 0; i < DWIN_ICONS_PER_FRAME; i++)
                {
                    uint16_t icon = HostCalcCtrlIcon(base_slave + i);
                    buf[i * 2]     = (uint8_t)(icon >> 8);
                    buf[i * 2 + 1] = (uint8_t)(icon & 0xFF);
                }

                DWIN_WriteVar(base_addr, buf, DWIN_ICONS_PER_FRAME * 2);
                vTaskDelay(pdMS_TO_TICKS(20));
            }

            uint16_t sys_icon = HostCalcSysIcon();
            buf[0] = (uint8_t)(sys_icon >> 8);
            buf[1] = (uint8_t)(sys_icon & 0xFF);
            DWIN_WriteVar(DWIN_HOST_STATUS_ICON_ADDR, buf, 2);
            vTaskDelay(pdMS_TO_TICKS(20));
        }
        else
        {
            /* ===== 详情界面模式 ===== */

            buf[0] = 0;

            /* 帧 1：窗状态 0x1882 */
            uint8_t win = HostReg_GetCoilBit(selectAddr, COIL_SMOKE_ALARM)
                       || HostReg_GetCoilBit(selectAddr, COIL_GLOBAL_ALARM);
            buf[1] = win ? 1 : 0;
            DWIN_WriteVar(DWIN_DETAIL_WIN_ICON_ADDR, buf, 2);
            vTaskDelay(pdMS_TO_TICKS(20));
            /* 帧 2：全局报警 0x1883 */
            buf[1] = HostReg_GetCoilBit(selectAddr, COIL_GLOBAL_ALARM) ? 1 : 0;
            DWIN_WriteVar(DWIN_DETAIL_GLB_ALARM_ADDR, buf, 2);
            vTaskDelay(pdMS_TO_TICKS(20));

            /* 帧 3：零地址 0x1884 */
            buf[1] = HostReg_GetCoilBit(selectAddr, COIL_OFFSET_ZERO_ADDR) ? 1 : 0;
            DWIN_WriteVar(DWIN_DETAIL_ZERO_ADDR_ADDR, buf, 2);
            vTaskDelay(pdMS_TO_TICKS(20));

            /* 帧 4：重复地址 0x1885 */
            buf[1] = HostReg_GetCoilBit(selectAddr, COIL_OFFSET_ADDR_CONF) ? 1 : 0;
            DWIN_WriteVar(DWIN_DETAIL_CONF_ADDR_ADDR, buf, 2);
            vTaskDelay(pdMS_TO_TICKS(20));
            /* 帧 5-6：传感器图标 0x1890-0x18CE（63 个 word） */
            uint8_t *p = buf;
            uint8_t count = 0;
            uint16_t icon_addr = DWIN_DETAIL_SENSOR_ICON_ADDR;

            for (uint8_t i = 1; i <= 63; i++)
            {
                uint16_t icon = DetailSensorIcon(selectAddr, i);
                *p++ = (uint8_t)(icon >> 8);
                *p++ = (uint8_t)(icon & 0xFF);
                count++;

                if (count == 32 || i == 63)
                {
                    DWIN_WriteVar(icon_addr, buf, count * 2);
                    icon_addr += count;
                    count = 0;
                    p = buf;
                    vTaskDelay(pdMS_TO_TICKS(20));
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ==================== 报警监测任务（每 50ms） ==================== */

void TaskAlarmMonitor(void *arg)
{
    (void)arg;

    /* 等待迪文屏初始化完成 */
    xSemaphoreTake(xDwinInitDoneSem, portMAX_DELAY);

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(50));

        /* ---- 状态机：WAIT_RTC 分支（等待迪文屏 RTC 应答） ---- */
        if (s_pending_flag)
        {
            if (g_hostDwinStatus.rtcReady)
            {
                /* RTC 已就绪 → 写入报警记录 */
                DWIN_RTC_t rtc;
                rtc.year   = g_hostDwinStatus.rtcYear;
                rtc.month  = g_hostDwinStatus.rtcMonth;
                rtc.day    = g_hostDwinStatus.rtcDay;
                rtc.hour   = g_hostDwinStatus.rtcHour;
                rtc.minute = g_hostDwinStatus.rtcMinute;
                rtc.second = g_hostDwinStatus.rtcSecond;

                DWIN_WriteAlarmRecord(&rtc, s_pending_ctrl, s_pending_sensor);

                /* 打印机非阻塞入队 */
                printer_job_t pjob;
                pjob.ctrlAddr   = s_pending_ctrl;
                pjob.sensorIdx  = s_pending_sensor;
                pjob.rtcYear    = rtc.year;
                pjob.rtcMonth   = rtc.month;
                pjob.rtcDay     = rtc.day;
                pjob.rtcHour    = rtc.hour;
                pjob.rtcMinute  = rtc.minute;
                pjob.rtcSecond  = rtc.second;
                xQueueSend(xPrinterTxQueue, &pjob, 0);

                g_hostDwinStatus.rtcReady = 0U;
                s_pending_flag = 0;

                /* 环缓冲中还有事件？→ 立即处理下一个 */
                if (s_alarm_rd != s_alarm_wr)
                {
                    s_pending_ctrl   = s_alarm_buf[s_alarm_rd].ctrlAddr;
                    s_pending_sensor = s_alarm_buf[s_alarm_rd].sensorIdx;
                    s_alarm_rd = (s_alarm_rd + 1U) % ALARM_EVENT_BUF_SIZE;
                    s_pending_flag = 1;
                    DWIN_ReadRTC();
                }
            }
            /* rtcReady == 0 → 继续等待下个周期 */
            continue;
        }

        /* ---- 状态机：IDLE 分支（扫描所有在线控制器） ---- */
        for (uint16_t addr = 1; addr <= MAX_CTRLBD_ADDR; addr++)
        {
            if (!HostReg_IsOnline((uint8_t)addr))
                continue;

            uint64_t curr    = HostReg_GetAlarmBits64((uint8_t)addr);
            uint64_t rising  = curr & ~s_prev_alarm[(uint8_t)addr];
            s_prev_alarm[(uint8_t)addr] = curr;

            /* 遍历所有 0→1 跳变的位 → 入环缓冲 */
            while (rising)
            {
                uint8_t n   = (uint8_t)__builtin_ctzll(rising);
                rising     &= (rising - 1ULL);  /* 清除最低置位位 */

                uint8_t next = (uint8_t)(s_alarm_wr + 1U) % ALARM_EVENT_BUF_SIZE;
                if (next != s_alarm_rd)
                {
                    s_alarm_buf[s_alarm_wr].ctrlAddr  = (uint8_t)addr;
                    s_alarm_buf[s_alarm_wr].sensorIdx = n;
                    s_alarm_wr = next;
                }
                /* 环缓冲满 → 丢弃 latest，下轮扫描会重新检出 */
            }
        }

        /* ---- 环缓冲有事件 → 出队第一个，发起 RTC 读取 ---- */
        if (s_alarm_rd != s_alarm_wr)
        {
            s_pending_ctrl   = s_alarm_buf[s_alarm_rd].ctrlAddr;
            s_pending_sensor = s_alarm_buf[s_alarm_rd].sensorIdx;
            s_alarm_rd = (s_alarm_rd + 1U) % ALARM_EVENT_BUF_SIZE;
            s_pending_flag = 1;
            DWIN_ReadRTC();
        }
    }
}

/* ==================== 打印机发送函数 ==================== */

/**
 * @brief  中断发送一段数据到打印机，等待 TX 完成
 * @param  data 数据指针
 * @param  len  数据长度
 */
static void Printer_Send(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit_IT(&huart2, (uint8_t *)data, len);
    xSemaphoreTake(xPrinterTxCompleteSem, pdMS_TO_TICKS(100));
}

/**
 * @brief  打印一条报警记录
 * @param  job  打印机作业结构体指针
 */
static void Printer_PrintAlarm(const printer_job_t *job)
{
    /* ---- ESC/POS 指令 ---- */
    const uint8_t cmd_init[]  = {0x1B, 0x40};                /* 初始化 */
    const uint8_t cmd_align[] = {0x1B, 0x61, 0x01};          /* 居中 */
    const uint8_t cmd_size[]  = {0x1D, 0x21, 0x11};          /* 倍高倍宽 */
    const uint8_t feed[]      = {0x1B, 0x64, 0x02};          /* 走纸 2 行 */
    const uint8_t crlf[]      = {0x0D, 0x0A};                /* 回车换行 */

    /* ---- GBK 编码中文 ---- */
    const uint8_t cn_sl[] = {0xB4, 0xD3, 0xBB, 0xFA};       /* 从机 */
    const uint8_t cn_dt[] = {0xBC, 0xEC, 0xB2, 0xE2, 0xC6, 0xF7}; /* 检测器 */
    const uint8_t cn_tm[] = {0xCA, 0xB1, 0xBC, 0xE4};       /* 时间 */

    /* ---- ASCII 数字 ---- */
    char buf_s[4], buf_d[4], date_buf[12], time_buf[12];
    sprintf(buf_s, "%02u", (unsigned)job->ctrlAddr);
    sprintf(buf_d, "%02u", (unsigned)job->sensorIdx);
    sprintf(date_buf, "%02u-%02u-%02u",
            (unsigned)job->rtcYear, (unsigned)job->rtcMonth, (unsigned)job->rtcDay);
    sprintf(time_buf, "%02u:%02u:%02u",
            (unsigned)job->rtcHour, (unsigned)job->rtcMinute, (unsigned)job->rtcSecond);

    /* ---- 发送打印内容 ---- */
    Printer_Send(cmd_init, sizeof(cmd_init));
    Printer_Send(cmd_align, sizeof(cmd_align));
    Printer_Send(cmd_size, sizeof(cmd_size));

    Printer_Send(cn_sl, sizeof(cn_sl));
    Printer_Send((const uint8_t *)buf_s, 2);
    Printer_Send(cn_dt, sizeof(cn_dt));
    Printer_Send((const uint8_t *)buf_d, 2);
    Printer_Send(crlf, sizeof(crlf));

    Printer_Send(cn_tm, sizeof(cn_tm));
    Printer_Send(crlf, sizeof(crlf));
    Printer_Send((const uint8_t *)date_buf, strlen(date_buf));
    Printer_Send(crlf, sizeof(crlf));
    Printer_Send((const uint8_t *)time_buf, strlen(time_buf));
    Printer_Send(crlf, sizeof(crlf));

    Printer_Send(feed, sizeof(feed));
}

/* ==================== 打印机发送任务 ==================== */

void TaskPrinterTx(void *arg)
{
    (void)arg;
    printer_job_t job;

    for (;;)
    {
        /* 阻塞等待打印机作业 */
        if (xQueueReceive(xPrinterTxQueue, &job, portMAX_DELAY) != pdPASS)
            continue;

        Printer_PrintAlarm(&job);
    }
}

/* USER CODE END 0 */
