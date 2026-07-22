/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_tasks.c
  * @brief          : LCD 128×64 点阵屏显示任务
  *
  * 页面系统：
  *   第 1 页 — 系统状态（在线数量、全局报警、零地址、地址冲突）
  *   第 2+ 页 — 每在线传感器一页，展示完整数据（按型号适配布局）
  *
  * 翻页：自动轮播，每页 5 秒。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "lcd_tasks.h"
#include "LCD_driver.h"
#include "modbus_registers.h"
#include "uart1_modbus.h"
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN 0 */

/* ==================== 背光控制 ==================== */

static void LcdBacklight_Set(uint8_t on)
{
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin,
        on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/* ==================== 传感器类型 → 缩写 ==================== */

static const char *Lcd_TypeToStr(uint8_t model)
{
    switch (model)
    {
        case SENSOR_MODEL_CO:               return "CO";
        case SENSOR_MODEL_WIND:             return "WP";
        case SENSOR_MODEL_PRESSURE:         return "PR";
        case SENSOR_MODEL_7IN1:             return "7IN";
        case SENSOR_MODEL_TH:               return "TH";
        case SENSOR_MODEL_CO2:              return "CO2";
        case SENSOR_MODEL_CHAIN_ISOLATOR:   return "CI";
        default:                            return "??";
    }
}

/* ==================== 构建在线传感器列表 ==================== */

static uint8_t Lcd_BuildOnlineList(uint8_t *addrs, uint8_t max_cnt)
{
    uint8_t cnt = 0;
    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES && cnt < max_cnt; i++)
    {
        if (ModbusReg_GetOnline(i))
            addrs[cnt++] = i;
    }
    return cnt;
}

/* ==================== 底行：页码 ==================== */

static void Lcd_DrawPageFooter(uint8_t page, uint8_t total)
{
    LCD_Printf(0, 7, 0, "<  PAGE %u/%u  >", (unsigned)page, (unsigned)total);
}

/* ==================== 第 1 页：系统状态 ==================== */

static void Lcd_DrawStatusPage(uint8_t total_pages)
{
    uint8_t i;
    uint8_t online_cnt = 0;

    for (i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        if (ModbusReg_GetOnline(i))
            online_cnt++;
    }

    LCD_Clear();

    LCD_Printf(0, 0, 0, "OnlineNum: %u", (unsigned)online_cnt);
    LCD_Printf(0, 1, 0, "GlobalAlarm: %s",
        ModbusReg_GetGlobalAlarmCoil() || ModbusReg_GetSmokeAlarm() ? "YES" : "NO");
    LCD_Printf(0, 2, 0, "Zero-Addr:%s",
        ModbusReg_GetZeroAddrPresent() ? "YES" : "NO");
    LCD_Printf(0, 3, 0, "ConflictAddr:%s",
        ModbusReg_GetAddrConflict() ? "YES" : "NO");

    Lcd_DrawPageFooter(1, total_pages);
}

/* ==================== 传感器详情页 ==================== */

/**
  * @brief  通用：单值传感器（WP / PR / CO2 / CI）
  *         第 0 行：地址 + 类型 + 报警状态
  *         第 1 行：标签 + 数值 + 单位
  */
static void Lcd_DrawSimpleSensor(uint8_t slave, uint8_t model,
    const char *label, const char *unit, uint8_t page, uint8_t total)
{
    uint16_t raw = ModbusReg_GetData(slave, 0);
    uint8_t  alarm = ModbusReg_GetAlarm(slave);

    LCD_Printf(0, 0, 0, "#%03u %s (%s)",
        (unsigned)slave, Lcd_TypeToStr(model), alarm ? "ALM" : "OK");
    LCD_Printf(0, 1, 0, "%s: %u %s", label, (unsigned)raw, unit);
    Lcd_DrawPageFooter(page, total);
}

/* ==================== CO 传感器 ==================== */

static void Lcd_DrawCOSensor(uint8_t slave, uint8_t page, uint8_t total)
{
    uint16_t raw = ModbusReg_GetData(slave, 0);
    uint8_t  alarm = ModbusReg_GetAlarm(slave);

    LCD_Printf(0, 0, 0, "#%03u %s (%s)",
        (unsigned)slave, Lcd_TypeToStr(SENSOR_MODEL_CO), alarm ? "ALM" : "OK");
    LCD_Printf(0, 1, 0, "CO: %u.%02u ppm", raw / 100, raw % 100);
    Lcd_DrawPageFooter(page, total);
}

/* ==================== 7 合 1 传感器 ==================== */

static void Lcd_Draw7In1Sensor(uint8_t slave, uint8_t page, uint8_t total)
{
    uint16_t eco2  = ModbusReg_GetData(slave, 0);
    uint16_t ech2o = ModbusReg_GetData(slave, 1);
    uint16_t tvoc  = ModbusReg_GetData(slave, 2);
    uint16_t pm25  = ModbusReg_GetData(slave, 3);
    uint16_t pm10  = ModbusReg_GetData(slave, 4);
    int16_t  s_temp = (int16_t)ModbusReg_GetData(slave, 5);
    uint16_t hum   = ModbusReg_GetData(slave, 6);
    uint8_t  alarm = ModbusReg_GetAlarm(slave);

    uint8_t temp_frac = (s_temp < 0) ? (uint8_t)((-s_temp) % 10) : (uint8_t)(s_temp % 10);

    LCD_Printf(0, 0, 0, "#%03u %s (%s)",
        (unsigned)slave, Lcd_TypeToStr(SENSOR_MODEL_7IN1), alarm ? "ALM" : "OK");

    /* 每行两列：左半 x=0，右半 x=64 */
    LCD_Printf(0,  1, 0, "eCO2:%u", (unsigned)eco2);
    LCD_Printf(64, 1, 0, "CH2O:%u", (unsigned)ech2o);

    LCD_Printf(0,  2, 0, "TVOC:%u", (unsigned)tvoc);
    LCD_Printf(64, 2, 0, "PM25:%u", (unsigned)pm25);

    LCD_Printf(0,  3, 0, "PM10:%u", (unsigned)pm10);
    LCD_Printf(64, 3, 0, "Tmp:%d.%u", s_temp / 10, temp_frac);

    LCD_Printf(0,  4, 0, "Hum: %u%%", (unsigned)(hum / 10));

    Lcd_DrawPageFooter(page, total);
}

/* ==================== 温湿度传感器 ==================== */

static void Lcd_DrawTHSensor(uint8_t slave, uint8_t page, uint8_t total)
{
    int16_t  s_temp = (int16_t)ModbusReg_GetData(slave, 0);
    uint16_t hum    = ModbusReg_GetData(slave, 1);
    uint8_t  alarm  = ModbusReg_GetAlarm(slave);

    uint8_t temp_frac = (s_temp < 0) ? (uint8_t)((-s_temp) % 10) : (uint8_t)(s_temp % 10);

    LCD_Printf(0, 0, 0, "#%03u %s (%s)",
        (unsigned)slave, Lcd_TypeToStr(SENSOR_MODEL_TH), alarm ? "ALM" : "OK");
    LCD_Printf(0, 1, 0, "Temp: %d.%u C",  s_temp / 10, temp_frac);
    LCD_Printf(0, 2, 0, "Hum:  %u%%",  (unsigned)(hum / 10));
    Lcd_DrawPageFooter(page, total);
}

/* ==================== 页面分发器 ==================== */

static void Lcd_DrawSensorPage(uint8_t slave, uint8_t page, uint8_t total)
{
    uint8_t model = ModbusReg_GetType(slave);

    LCD_Clear();

    switch (model)
    {
        case SENSOR_MODEL_CO:
            Lcd_DrawCOSensor(slave, page, total);
            break;

        case SENSOR_MODEL_WIND:
            Lcd_DrawSimpleSensor(slave, model, "Press", "Pa", page, total);
            break;

        case SENSOR_MODEL_PRESSURE:
            Lcd_DrawSimpleSensor(slave, model, "Press", "Pa", page, total);
            break;

        case SENSOR_MODEL_7IN1:
            Lcd_Draw7In1Sensor(slave, page, total);
            break;

        case SENSOR_MODEL_TH:
            Lcd_DrawTHSensor(slave, page, total);
            break;

        case SENSOR_MODEL_CO2:
            Lcd_DrawSimpleSensor(slave, model, "CO2", "ppm", page, total);
            break;

        case SENSOR_MODEL_CHAIN_ISOLATOR:
            Lcd_DrawSimpleSensor(slave, model, "Level", "", page, total);
            break;

        default:
            LCD_Printf(0, 0, 0, "#%03u ??", (unsigned)slave);
            LCD_Printf(0, 1, 0, "Unknown type");
            Lcd_DrawPageFooter(page, total);
            break;
    }
}

/* ==================== FreeRTOS 任务 ==================== */

#define MAX_ONLINE              MODBUS_MAX_SLAVES   /* 63 */
#define DISPLAY_REFRESH_MS      500                  /* 内容刷新间隔 */
#define PAGE_INTERVAL_MS        5000                 /* 翻页间隔 */

void TaskLcdDisplay(void *arg)
{
    (void)arg;

    /* ---- 硬件初始化 ---- */
    LcdBacklight_Set(1);
    LCD_Init();
    LCD_Clear();
    LCD_Printf(16, 2, 0, "Controlboard");
    LCD_Printf(8,  4, 0, "Initializing...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* ---- 页面状态 ---- */
    uint8_t  online_addrs[MAX_ONLINE];
    uint8_t  online_cnt = 0;
    uint8_t  total_pages = 1;       /* 至少 1 页（状态页） */
    uint8_t  page_idx = 0;          /* 0 = 状态页 */

    TickType_t last_draw_tick = xTaskGetTickCount();
    TickType_t last_page_tick = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&last_draw_tick, pdMS_TO_TICKS(DISPLAY_REFRESH_MS));

        /* ---- 1. 更新在线列表 ---- */
        online_cnt  = Lcd_BuildOnlineList(online_addrs, MAX_ONLINE);
        total_pages = 1 + online_cnt;

        /* ---- 2. 检查翻页 ---- */
        TickType_t now = xTaskGetTickCount();
        if (now - last_page_tick >= pdMS_TO_TICKS(PAGE_INTERVAL_MS))
        {
            page_idx++;
            if (page_idx >= total_pages)
                page_idx = 0;
            last_page_tick = now;
        }

        /* ---- 3. 绘制当前页 ---- */
        if (page_idx == 0)
        {
            Lcd_DrawStatusPage(total_pages);
        }
        else
        {
            Lcd_DrawSensorPage(online_addrs[page_idx - 1], page_idx, total_pages);
        }
    }
}

/* USER CODE END 0 */
