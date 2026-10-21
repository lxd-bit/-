#include "cmsis_os.h"
#include "Key.h"
#include "LED.h"
#include "OLED.h"
#include "PID.h"
#include "robot_state.h"
#include "Control_Task.h"

void Key_Task(void *argument)
{
    for(;;)
    {
        Key_Tick();     // 消抖（内部 Count >= 20）
        osDelay(1);     // 1ms 一次，配合 Count>=20 刚好 20ms 消抖
    }
}


void UI_Task(void *argument)
{
    /* ========== 初始化（只跑一次） ========== */
    OLED_Init();
    OLED_Clear();
    OLED_Update();

    uint8_t oled_tick = 0;
    uint8_t key_val;
    MPU6050_Data_t sensorData;

    /* ========== 20ms 周期循环 ========== */
    for(;;)
    {
        /* ---- 1. 按键扫描 ---- */
        key_val = Key_GetNum();

        if (key_val == 1)
        {
            Run_Flag = !Run_Flag;
            if (Run_Flag) {
                PID_Init(&AnglePID);
                PID_Init(&SpeedPID);
                PID_Init(&TurnPID);
            }
        }

        /* ---- 2. LED 状态指示 ---- */
        if (Run_Flag)
        {
            LED_ON();   // 运行中：常亮
        }
        else
        {
            if (oled_tick % 5 == 0) LED_Turn();   // 停止：慢闪
        }

        /* ---- 3. OLED 刷新（每 5 次 = 100ms 一次） ---- */
        oled_tick++;
        if (oled_tick >= 5)
        {
            oled_tick = 0;

			OLED_Clear();
			OLED_Printf(0, 0, OLED_6X8, "  Angle");
			OLED_Printf(0, 8, OLED_6X8, "P:%05.2f", AnglePID.Kp);
			OLED_Printf(0, 16, OLED_6X8, "I:%05.2f", AnglePID.Ki);
			OLED_Printf(0, 24, OLED_6X8, "D:%05.2f", AnglePID.Kd);
			OLED_Printf(0, 32, OLED_6X8, "T:%+05.1f", AnglePID.Target);
			OLED_Printf(0, 40, OLED_6X8, "A:%+05.1f", Angle);
			OLED_Printf(0, 48, OLED_6X8, "O:%+05.0f", AnglePID.Out);
            if (MPU6050_Mailbox_Peek(&sensorData, 0) == pdPASS)
            {
                OLED_Printf(0, 56, OLED_6X8, "GY:%+05d", sensorData.GyroY);
            }
			OLED_Printf(56, 56, OLED_6X8, "Offset:%02.0f", AnglePID.OutOffset);
			OLED_Printf(50, 0, OLED_6X8, "Speed");
			OLED_Printf(50, 8, OLED_6X8, "%05.2f", SpeedPID.Kp);
			OLED_Printf(50, 16, OLED_6X8, "%05.2f", SpeedPID.Ki);
			OLED_Printf(50, 24, OLED_6X8, "%05.2f", SpeedPID.Kd);
			OLED_Printf(50, 32, OLED_6X8, "%+05.1f", SpeedPID.Target);
			OLED_Printf(50, 40, OLED_6X8, "%+05.1f", AveSpeed);
			OLED_Printf(50, 48, OLED_6X8, "%+05.0f", SpeedPID.Out);
			OLED_Printf(88, 0, OLED_6X8, "Turn");
			OLED_Printf(88, 8, OLED_6X8, "%05.2f", TurnPID.Kp);
			OLED_Printf(88, 16, OLED_6X8, "%05.2f", TurnPID.Ki);
			OLED_Printf(88, 24, OLED_6X8, "%05.2f", TurnPID.Kd);
			OLED_Printf(88, 32, OLED_6X8, "%+05.1f", TurnPID.Target);
			OLED_Printf(88, 40, OLED_6X8, "%+05.1f", DifSpeed);
			OLED_Printf(88, 48, OLED_6X8, "%+05.0f", TurnPID.Out);
			OLED_Update();
        }

        osDelay(20);   // 20ms 周期
    }
}
