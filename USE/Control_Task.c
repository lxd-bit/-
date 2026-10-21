#include "cmsis_os.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encode.h"
#include "PID.h"
#include "robot_state.h"
#include "Control_Task.h"
#include "task.h"
#include <math.h>

static QueueHandle_t MPU6050_Mailbox;
static TaskHandle_t ControlTaskHandle;

BaseType_t MPU6050_Mailbox_Init(void)
{
    MPU6050_Mailbox = xQueueCreate(1, sizeof(MPU6050_Data_t));
    return MPU6050_Mailbox != NULL ? pdPASS : pdFAIL;
}

BaseType_t MPU6050_Mailbox_Peek(MPU6050_Data_t *data, TickType_t timeout)
{
    if (MPU6050_Mailbox == NULL || data == NULL)
    {
        return pdFAIL;
    }

    return xQueuePeek(MPU6050_Mailbox, data, timeout);
}

void MPU6050_Task(void *argument)
{
    MPU6050_Data_t data;
    TickType_t lastWakeTime = xTaskGetTickCount();

    (void)argument;
    for (;;)
    {
        MPU6050_GetData(&data.AccX, &data.AccY, &data.AccZ,
                        &data.GyroX, &data.GyroY, &data.GyroZ);
        if (xQueueOverwrite(MPU6050_Mailbox, &data) != pdPASS)
        {
            Error_Handler();
        }
        xTaskNotifyGive(ControlTaskHandle);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(5));
    }
}

void Control_Task(void *argument)
{
    /* ========== 初始化（只跑一次） ========== */


    PID_Init(&AnglePID);
    PID_Init(&SpeedPID);
    PID_Init(&TurnPID);

    uint8_t speed_tick = 0;
    MPU6050_Data_t sensorData;
    float angleAcc;
    float angleGyro;
    ControlTaskHandle = xTaskGetCurrentTaskHandle();

    for(;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (MPU6050_Mailbox_Peek(&sensorData, 0) != pdPASS)
        {
            Error_Handler();
        }

        /* ---- 2. 姿态解算（江科大互补滤波） ---- */
        int16_t gyroY = sensorData.GyroY - 31;   // 零偏，需要实测后修改
        float Alpha = 0.01;
        angleAcc  = -atan2(sensorData.AccX, sensorData.AccZ) / 3.14159 * 180;
        angleGyro = Angle + gyroY / 32768.0 * 2000 * 0.005;   // 注意：5ms 周期
        Angle     = Alpha * angleAcc + (1 - Alpha) * angleGyro;

        /* ---- 3. 摔倒保护 ---- */
        if (Angle > 50 || Angle < -50)
        {
            Run_Flag = 0;
        }

        /* ---- 4. 直立环（5ms）---- */
        if (Run_Flag)
        {
            AnglePID.Actual = Angle;
            PID_Update(&AnglePID);
            AvePWM = -AnglePID.Out;

            LeftPWM  = AvePWM + DifPWM / 2;
            RightPWM = AvePWM - DifPWM / 2;

            /* 限幅 */
            if (LeftPWM  >  100) LeftPWM  =  100;
            if (LeftPWM  < -100) LeftPWM  = -100;
            if (RightPWM >  100) RightPWM =  100;
            if (RightPWM < -100) RightPWM = -100;

            Motor_SetPWM(1, LeftPWM);
            Motor_SetPWM(2, RightPWM);
        }
        else
        {
            Motor_SetPWM(1, 0);
            Motor_SetPWM(2, 0);
        }

        /* ---- 5. 速度环 + 转向环（50ms 分频）---- */
        speed_tick++;
        if (speed_tick >= 10)
        {
            speed_tick = 0;

            LeftSpeed  = Encoder_Get(1) / 44.0 / 0.05 / 9.27666;
            RightSpeed = Encoder_Get(2) / 44.0 / 0.05 / 9.27666;

            AveSpeed = (LeftSpeed + RightSpeed) / 2;
            DifSpeed = LeftSpeed - RightSpeed;

            if (Run_Flag)
            {
                SpeedPID.Actual = AveSpeed;
                PID_Update(&SpeedPID);
                AnglePID.Target = SpeedPID.Out;   // 速度环输出作为直立环目标

                TurnPID.Actual = DifSpeed;
                PID_Update(&TurnPID);
                DifPWM = TurnPID.Out;
            }
        }

    }
}
