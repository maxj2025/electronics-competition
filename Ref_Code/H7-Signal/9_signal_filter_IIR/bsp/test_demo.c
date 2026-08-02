#include "test_demo.h"


///* ??????512KB AXI SRAM???????? */
//__attribute__((section (".RAM_D1")))  volatile uint16_t AXISRAMBuf[10];
//__attribute__((section (".RAM_D1")))  volatile uint16_t AXISRAMCount;

///* ??????128KB SRAM1(0x30000000) + 128KB SRAM2(0x30020000) + 32KB SRAM3(0x30040000)???????? */
//__attribute__((section (".RAM_D2")))  volatile uint16_t D2SRAMBuf[10];
//__attribute__((section (".RAM_D2")))  volatile uint16_t D2SRAMount;

///* ??????64KB SRAM4(0x38000000)???????? */
//__attribute__((section (".RAM_D3")))  volatile uint16_t D3SRAMBuf[10];
//__attribute__((section (".RAM_D3")))  volatile uint16_t D3SRAMCount;

///* ??????32MB SDRAM(0xC0000000)???????? */
//__attribute__((section (".RAM_SDRAM"),zero_init)) volatile uint16_t SDRAMSRAMCount;
//__attribute__((section (".RAM_SDRAM"),zero_init)) volatile uint16_t SDRAMSRAMBuf[1024*1024*4];
//__attribute__((section (".RAM_SDRAM"),zero_init)) volatile uint16_t SDRAMSRAMBuf2[10];


//uint16_t temp_buff[10];
//void Mem_test(void)
//{

//    for (int i = 0; i < 10; i++)
//    {
//    SDRAMSRAMBuf[i] = 0x1234+i;
//		AXISRAMBuf[i] = 0x1234 + i;
//		D2SRAMBuf[i] = 0x2234 + i;
//		D3SRAMBuf[i] = 0x3234 + i;
//    }
//		
//		 for (int i = 0; i < 10; i++)
//    {
//    temp_buff[i] = SDRAMSRAMBuf[i];
//    }

//    AXISRAMCount = 0x55AA;
//		D2SRAMount = 0x55AA;
//		D3SRAMCount = 0x55AA;
//		SDRAMSRAMCount = 0x55AA;
//		for(int i=0;i<10;i++)
//		{
//			SDRAMSRAMBuf2[i] = 0x00+i;
//		}
//		
//		for (int i = 0; i < 10; i++)
//    {
//    temp_buff[i] = SDRAMSRAMBuf2[i];
//    }
//		
//		UART1_Printf("\r\nCPU : STM32H723ZGT6主频: %dMHz\r\n", SystemCoreClock / 1000000);
//		HAL_Delay(10);
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
//		HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);
//}

//void SDRAM_Speed_Test(void)
//{
//    uint32_t i, start, end;
//    uint32_t count = sizeof(SDRAMSRAMBuf) / sizeof(SDRAMSRAMBuf[0]);
//    uint32_t sum = 0;
//    float t, speed;

//    UART1_Printf("SDRAM speed test start...\r\n");
//		HAL_Delay(10);
//    UART1_Printf("Buffer size: %lu bytes\r\n", sizeof(SDRAMSRAMBuf));
//		HAL_Delay(10);
//	
//    start = uwTick;
//    for (i = 0; i < count; i++)
//    {
//        SDRAMSRAMBuf[i] = (uint16_t)i;
//    }
//    end = uwTick;
//		
//		SCB_CleanInvalidateDCache();
//    if (end == start) end = start + 1;
//    t = (float)(end - start) / 1000.0f;
//    speed = (float)sizeof(SDRAMSRAMBuf) / 1024.0f / 1024.0f / t;
//    UART1_Printf("Write speed: %.2f MB/s\r\n", speed);
//		HAL_Delay(10);
//		
//    start = uwTick;
//    for (i = 0; i < count; i++)
//    {
//        sum += SDRAMSRAMBuf[i];
//    }
//    end = uwTick;
//		
//		SCB_CleanInvalidateDCache();
//    if (end == start) end = start + 1;
//    t = (float)(end - start) / 1000.0f;
//    speed = (float)sizeof(SDRAMSRAMBuf) / 1024.0f / 1024.0f / t;
//    UART1_Printf("Read speed : %.2f MB/s, sum=0x%08lX\r\n", speed, sum);
//		HAL_Delay(10);
//}


////#define TEST_ADDR  0x001000      /* 测试地址，建议从0开始或选一个空闲扇区 */
////#define TEST_LEN    32

////uint8_t tx_buf[TEST_LEN] = {
////    0x11, 0x22, 0x33, 0x44,
////    0x55, 0x66, 0x77, 0x88,
////    0xA1, 0xA2, 0xA3, 0xA4,
////    0xB1, 0xB2, 0xB3, 0xB4,
////    0xC1, 0xC2, 0xC3, 0xC4,
////    0xD1, 0xD2, 0xD3, 0xD4,
////    0xE1, 0xE2, 0xE3, 0xE4,
////    0xF1, 0xF2, 0xF3, 0xF4
////};

////uint8_t rx_buf[TEST_LEN];

/////* 打印缓冲区 */
////static void norflash_dump_buf(const char *name, uint8_t *buf, uint32_t len)
////{
////    uint32_t i;
////    UART1_Printf("%s:\r\n", name);

////    for (i = 0; i < len; i++)
////    {
////        UART1_Printf("%02X ", buf[i]);

////        if ((i + 1) % 16 == 0)
////        {
////            UART1_Printf("\r\n");
////        }
////    }

////    if (len % 16 != 0)
////    {
////        UART1_Printf("\r\n");
////    }
////}

/////* 最小测试函数 */
////void norflash_test(void)
////{
////    uint16_t id;
////    uint32_t i;
////    uint8_t ok = 1;

////    UART1_Printf("\r\n================ NORFLASH TEST BEGIN ================\r\n");

////    /* 1. 初始化 */
////    norflash_init();

////    /* 2. 读ID */
////    id = norflash_read_id();
////    UART1_Printf("NORFLASH ID = 0x%04X\r\n", id);

////    /* 3. 擦除测试扇区
////       TEST_ADDR=0x000000，对应扇区号=0
////       如果你不想动第0扇区，可改 TEST_ADDR 为 0x1000 / 0x2000 / 0x10000 等 */
////    UART1_Printf("Erase sector...\r\n");
////    norflash_erase_sector(TEST_ADDR / 4096);

////    /* 4. 先读一次，确认擦除后是否为0xFF */
////    memset(rx_buf, 0, sizeof(rx_buf));
////    norflash_read(rx_buf, TEST_ADDR, TEST_LEN);
////    norflash_dump_buf("After Erase Read", rx_buf, TEST_LEN);

////    for (i = 0; i < TEST_LEN; i++)
////    {
////        if (rx_buf[i] != 0xFF)
////        {
////            UART1_Printf("Erase check failed at %lu, data=0x%02X\r\n", i, rx_buf[i]);
////            ok = 0;
////            break;
////        }
////    }

////    if (ok == 0)
////    {
////        UART1_Printf("NORFLASH erase test failed!\r\n");
////        UART1_Printf("================ NORFLASH TEST END ==================\r\n");
////        return;
////    }

////    /* 5. 写入测试数据 */
////    UART1_Printf("Write data...\r\n");
////    norflash_write(tx_buf, TEST_ADDR, TEST_LEN);

////    /* 6. 读回校验 */
////    memset(rx_buf, 0, sizeof(rx_buf));
////    norflash_read(rx_buf, TEST_ADDR, TEST_LEN);

////    norflash_dump_buf("Write Data", tx_buf, TEST_LEN);
////    norflash_dump_buf("Read  Data", rx_buf, TEST_LEN);

////    for (i = 0; i < TEST_LEN; i++)
////    {
////        if (rx_buf[i] != tx_buf[i])
////        {
////            UART1_Printf("Verify failed at %lu, W=0x%02X R=0x%02X\r\n", i, tx_buf[i], rx_buf[i]);
////            ok = 0;
////            break;
////        }
////    }

////    if (ok)
////    {
////        UART1_Printf("NORFLASH read/write test success!\r\n");
////    }
////    else
////    {
////        UART1_Printf("NORFLASH read/write test failed!\r\n");
////    }

////    UART1_Printf("================ NORFLASH TEST END ==================\r\n");
////}


//#define SPEED_TEST_ADDR      0x010000
//#define SPEED_TEST_SIZE      (4 * 1024)
//#define SPEED_TEST_SECTOR    4096
//#define READ_REPEAT_COUNT    10

//static uint8_t speed_tx_buf[SPEED_TEST_SIZE];
//static uint8_t speed_rx_buf[SPEED_TEST_SIZE];

///* 填充测试数据 */
//static void norflash_speed_fill_pattern(uint8_t *buf, uint32_t len)
//{
//    uint32_t i;
//    for (i = 0; i < len; i++)
//    {
//        buf[i] = (uint8_t)(i & 0xFF);
//    }
//}

///* 校验数据 */
//static uint8_t norflash_speed_verify(uint8_t *tx, uint8_t *rx, uint32_t len)
//{
//    uint32_t i;
//    for (i = 0; i < len; i++)
//    {
//        if (tx[i] != rx[i])
//        {
//            UART1_Printf("Verify failed at %lu, W=0x%02X R=0x%02X\r\n",
//                         i, tx[i], rx[i]);
//            return 1;
//        }
//    }
//    return 0;
//}

//void norflash_speed_test(void)
//{
//    uint32_t start_tick, end_tick;
//    uint32_t erase_time_ms;
//    uint32_t write_time_ms;
//    uint32_t read_time_ms;
//    uint32_t sector_count;
//    uint32_t i;
//    float erase_speed;
//    float write_speed;
//    float read_speed;

//    UART1_Printf("\r\n================ NORFLASH SPEED TEST BEGIN ================\r\n");
//    UART1_Printf("Test Addr : 0x%08lX\r\n", SPEED_TEST_ADDR);
//    UART1_Printf("Test Size : %lu bytes (%lu KB)\r\n", SPEED_TEST_SIZE, SPEED_TEST_SIZE / 1024);

//    norflash_init();

//    norflash_speed_fill_pattern(speed_tx_buf, SPEED_TEST_SIZE);
//    memset(speed_rx_buf, 0, SPEED_TEST_SIZE);

//    /* 擦除 */
//    sector_count = SPEED_TEST_SIZE / SPEED_TEST_SECTOR;
//    if (SPEED_TEST_SIZE % SPEED_TEST_SECTOR)
//    {
//        sector_count++;
//    }

//    UART1_Printf("Erase %lu sectors...\r\n", sector_count);

//    start_tick = HAL_GetTick();
//    for (i = 0; i < sector_count; i++)
//    {
//        norflash_erase_sector((SPEED_TEST_ADDR / SPEED_TEST_SECTOR) + i);
//    }
//    end_tick = HAL_GetTick();

//    erase_time_ms = end_tick - start_tick;
//    if (erase_time_ms == 0) erase_time_ms = 1;
//    erase_speed = ((float)SPEED_TEST_SIZE / 1024.0f) / ((float)erase_time_ms / 1000.0f);

//    UART1_Printf("Erase Time : %lu ms\r\n", erase_time_ms);
//    UART1_Printf("Erase Speed: %.2f KB/s\r\n", erase_speed);

//    /* 写 */
//    UART1_Printf("Write test...\r\n");

//    start_tick = HAL_GetTick();
//    norflash_write(speed_tx_buf, SPEED_TEST_ADDR, SPEED_TEST_SIZE);
//    end_tick = HAL_GetTick();

//    write_time_ms = end_tick - start_tick;
//    if (write_time_ms == 0) write_time_ms = 1;
//    write_speed = ((float)SPEED_TEST_SIZE / 1024.0f) / ((float)write_time_ms / 1000.0f);

//    UART1_Printf("Write Time : %lu ms\r\n", write_time_ms);
//    UART1_Printf("Write Speed: %.2f KB/s\r\n", write_speed);

//    /* 读，多次重复更准确 */
//    UART1_Printf("Read test...\r\n");

//    memset(speed_rx_buf, 0, SPEED_TEST_SIZE);

//    start_tick = HAL_GetTick();
//    for (i = 0; i < READ_REPEAT_COUNT; i++)
//    {
//        norflash_read(speed_rx_buf, SPEED_TEST_ADDR, SPEED_TEST_SIZE);
//    }
//    end_tick = HAL_GetTick();

//    read_time_ms = end_tick - start_tick;
//    if (read_time_ms == 0) read_time_ms = 1;
//    read_speed = ((float)(SPEED_TEST_SIZE * READ_REPEAT_COUNT) / 1024.0f) / ((float)read_time_ms / 1000.0f);

//    UART1_Printf("Read Time  : %lu ms\r\n", read_time_ms);
//    UART1_Printf("Read Speed : %.2f KB/s\r\n", read_speed);

//    /* 校验 */
//    UART1_Printf("Verify data...\r\n");
//    norflash_read(speed_rx_buf, SPEED_TEST_ADDR, SPEED_TEST_SIZE);

//    if (norflash_speed_verify(speed_tx_buf, speed_rx_buf, SPEED_TEST_SIZE) == 0)
//    {
//        UART1_Printf("Verify OK!\r\n");
//    }
//    else
//    {
//        UART1_Printf("Verify FAILED!\r\n");
//    }

//    UART1_Printf("================ NORFLASH SPEED TEST END ==================\r\n");
//}


////void SDRAM_BasicTest(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;
////    volatile uint16_t r[16];
////    int i;

////    for (i = 0; i < 16; i++)
////        p[i] = 0x1230 + i;

////    for (i = 0; i < 16; i++)
////        r[i] = p[i];
////}

////void test_same_addr(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;
////    volatile uint16_t a, b, c;

////    p[0] = 0x1111; a = p[0];
////    
////    p[0] = 0x3333; c = p[0];
////		p[0] = 0x2222; b = p[0];
////}

////void test_addr_alias(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;

////    p[0] = 0xAAAA;
////    p[1] = 0x5555;

////    volatile uint16_t r0 = p[0];
////    volatile uint16_t r1 = p[1];
////}

////void SDRAM_AliasTest(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;

////    p[0] = 0x1000;
////    p[1] = 0x1001;
////    p[2] = 0x1002;
////    p[3] = 0x1003;

////    volatile uint16_t r0 = p[0];
////    volatile uint16_t r1 = p[1];
////    volatile uint16_t r2 = p[2];
////    volatile uint16_t r3 = p[3];
////}

////void SDRAM_AddressBusTest(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;

////    p[0]  = 0xA000;
////    p[1]  = 0xA001;
////    p[2]  = 0xA002;
////    p[4]  = 0xA004;
////    p[8]  = 0xA008;
////    p[16] = 0xA010;
////    p[32] = 0xA020;
////}

////volatile uint16_t g_pat[4];

////void SDRAM_AllZeroOneTest(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;

////    p[0] = 0x0000; g_pat[0] = p[0];
////    p[0] = 0xFFFF; g_pat[1] = p[0];
////    p[0] = 0xAAAA; g_pat[2] = p[0];
////    p[0] = 0x5555; g_pat[3] = p[0];
////}

////uint32_t err = 0;
////void SDRAM_FullTest(void)
////{
////    volatile uint16_t *p = (volatile uint16_t *)0xC0000000;
////    uint32_t i;
////    

////    for (i = 0; i < 4096; i++)
////        p[i] = (uint16_t)(0x5000 + i);

////    for (i = 0; i < 4096; i++)
////    {
////        if (p[i] != (uint16_t)(0x5000 + i))
////            err++;
////    }

//// 
////}
//#define SDRAM_BASE_ADDR       0xC0000000UL
//#define SDRAM_SIZE_BYTES      (32UL * 1024UL * 1024UL)
//#define SDRAM_HALFWORD_COUNT  (SDRAM_SIZE_BYTES / 2UL)

//static uint32_t sdram_fail_addr;
//static uint16_t sdram_fail_expect;
//static uint16_t sdram_fail_actual;

//static void SDRAM_PrintFail(const char *name, uint32_t index, uint16_t expect, uint16_t actual)
//{
//    uint32_t addr = SDRAM_BASE_ADDR + index * 2UL;

//    UART1_Printf("[%s] FAILED\r\n", name);
//    UART1_Printf("Index : %lu\r\n", index);
//    UART1_Printf("Addr  : 0x%08lX\r\n", addr);
//    UART1_Printf("Expect: 0x%04X\r\n", expect);
//    UART1_Printf("Actual: 0x%04X\r\n", actual);

//    sdram_fail_addr = addr;
//    sdram_fail_expect = expect;
//    sdram_fail_actual = actual;
//}

//static uint32_t SDRAM_DataBusTest(void)
//{
//    volatile uint16_t *p = (volatile uint16_t *)SDRAM_BASE_ADDR;
//    uint16_t pattern;
//    uint16_t actual;

//    UART1_Printf("SDRAM data bus test...\r\n");

//    for (pattern = 1U; pattern != 0U; pattern <<= 1)
//    {
//        p[0] = pattern;
//        actual = p[0];

//        if (actual != pattern)
//        {
//            SDRAM_PrintFail("DataBus", 0, pattern, actual);
//            return 1;
//        }
//    }

//    p[0] = 0xAAAA;
//    actual = p[0];
//    if (actual != 0xAAAA)
//    {
//        SDRAM_PrintFail("DataBus", 0, 0xAAAA, actual);
//        return 2;
//    }

//    p[0] = 0x5555;
//    actual = p[0];
//    if (actual != 0x5555)
//    {
//        SDRAM_PrintFail("DataBus", 0, 0x5555, actual);
//        return 3;
//    }

//    UART1_Printf("SDRAM data bus OK\r\n");
//    return 0;
//}

//static uint32_t SDRAM_AddressBusTest(void)
//{
//    volatile uint16_t *p = (volatile uint16_t *)SDRAM_BASE_ADDR;
//    uint32_t offset;
//    uint32_t test_offset_count = SDRAM_HALFWORD_COUNT;
//    uint16_t actual;

//    UART1_Printf("SDRAM address bus test...\r\n");

//    p[0] = 0x0000;

//    for (offset = 1UL; offset < test_offset_count; offset <<= 1)
//    {
//        p[offset] = (uint16_t)(offset ^ 0xA5A5U);
//    }

//    for (offset = 1UL; offset < test_offset_count; offset <<= 1)
//    {
//        uint16_t expect = (uint16_t)(offset ^ 0xA5A5U);
//        actual = p[offset];

//        if (actual != expect)
//        {
//            SDRAM_PrintFail("AddressBus", offset, expect, actual);
//            return offset;
//        }
//    }

//    actual = p[0];
//    if (actual != 0x0000)
//    {
//        SDRAM_PrintFail("AddressBus", 0, 0x0000, actual);
//        return 0xFFFFFFFFUL;
//    }

//    UART1_Printf("SDRAM address bus OK\r\n");
//    return 0;
//}

//static uint32_t SDRAM_FillVerify16(uint16_t pattern)
//{
//    volatile uint16_t *p = (volatile uint16_t *)SDRAM_BASE_ADDR;
//    uint32_t i;
//    uint16_t actual;

//    UART1_Printf("Fill 0x%04X...\r\n", pattern);

//    for (i = 0; i < SDRAM_HALFWORD_COUNT; i++)
//    {
//        p[i] = pattern;
//    }

//    UART1_Printf("Verify 0x%04X...\r\n", pattern);

//    for (i = 0; i < SDRAM_HALFWORD_COUNT; i++)
//    {
//        actual = p[i];

//        if (actual != pattern)
//        {
//            SDRAM_PrintFail("FillVerify", i, pattern, actual);
//            return i;
//        }
//    }

//    UART1_Printf("Fill 0x%04X OK\r\n", pattern);
//    return 0;
//}

//static uint32_t SDRAM_WalkingPatternTest(void)
//{
//    volatile uint16_t *p = (volatile uint16_t *)SDRAM_BASE_ADDR;
//    uint32_t i;
//    uint16_t expect;
//    uint16_t actual;

//    UART1_Printf("SDRAM walking pattern write...\r\n");

//    for (i = 0; i < SDRAM_HALFWORD_COUNT; i++)
//    {
//        p[i] = (uint16_t)(i ^ 0xA5A5U);
//    }

//    UART1_Printf("SDRAM walking pattern verify...\r\n");

//    for (i = 0; i < SDRAM_HALFWORD_COUNT; i++)
//    {
//        expect = (uint16_t)(i ^ 0xA5A5U);
//        actual = p[i];

//        if (actual != expect)
//        {
//            SDRAM_PrintFail("WalkingPattern", i, expect, actual);
//            return i;
//        }
//    }

//    UART1_Printf("SDRAM walking pattern OK\r\n");
//    return 0;
//}

//static uint32_t SDRAM_InverseWalkingPatternTest(void)
//{
//    volatile uint16_t *p = (volatile uint16_t *)SDRAM_BASE_ADDR;
//    uint32_t i;
//    uint16_t expect;
//    uint16_t actual;

//    UART1_Printf("SDRAM inverse pattern write...\r\n");

//    for (i = 0; i < SDRAM_HALFWORD_COUNT; i++)
//    {
//        p[i] = (uint16_t)(~(i ^ 0xA5A5U));
//    }

//    UART1_Printf("SDRAM inverse pattern verify...\r\n");

//    for (i = 0; i < SDRAM_HALFWORD_COUNT; i++)
//    {
//        expect = (uint16_t)(~(i ^ 0xA5A5U));
//        actual = p[i];

//        if (actual != expect)
//        {
//            SDRAM_PrintFail("InversePattern", i, expect, actual);
//            return i;
//        }
//    }

//    UART1_Printf("SDRAM inverse pattern OK\r\n");
//    return 0;
//}

//static uint32_t SDRAM_BoundaryTest(void)
//{
//    volatile uint16_t *p = (volatile uint16_t *)SDRAM_BASE_ADDR;
//    uint32_t last = SDRAM_HALFWORD_COUNT - 1UL;

//    UART1_Printf("SDRAM boundary test...\r\n");

//    p[0] = 0x1111;
//    p[1] = 0x2222;
//    p[last - 1] = 0x3333;
//    p[last] = 0x4444;

//    if (p[0] != 0x1111)
//    {
//        SDRAM_PrintFail("Boundary", 0, 0x1111, p[0]);
//        return 1;
//    }

//    if (p[1] != 0x2222)
//    {
//        SDRAM_PrintFail("Boundary", 1, 0x2222, p[1]);
//        return 2;
//    }

//    if (p[last - 1] != 0x3333)
//    {
//        SDRAM_PrintFail("Boundary", last - 1, 0x3333, p[last - 1]);
//        return 3;
//    }

//    if (p[last] != 0x4444)
//    {
//        SDRAM_PrintFail("Boundary", last, 0x4444, p[last]);
//        return 4;
//    }

//    UART1_Printf("SDRAM boundary OK\r\n");
//    return 0;
//}

//uint32_t SDRAM_FullTest(void)
//{
//    uint32_t err;

//    UART1_Printf("\r\n================ SDRAM FULL TEST BEGIN ================\r\n");
//    UART1_Printf("Base : 0x%08lX\r\n", SDRAM_BASE_ADDR);
//    UART1_Printf("Size : %lu bytes\r\n", SDRAM_SIZE_BYTES);
//    UART1_Printf("Words: %lu halfwords\r\n", SDRAM_HALFWORD_COUNT);

//    err = SDRAM_DataBusTest();
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: DataBus err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_AddressBusTest();
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: AddressBus err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_BoundaryTest();
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: Boundary err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_FillVerify16(0x0000);
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: Fill 0x0000 err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_FillVerify16(0xFFFF);
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: Fill 0xFFFF err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_FillVerify16(0xAAAA);
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: Fill 0xAAAA err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_FillVerify16(0x5555);
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: Fill 0x5555 err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_WalkingPatternTest();
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: WalkingPattern err=0x%08lX\r\n", err);
//        return err;
//    }

//    err = SDRAM_InverseWalkingPatternTest();
//    if (err != 0)
//    {
//        UART1_Printf("SDRAM FULL TEST FAILED: InversePattern err=0x%08lX\r\n", err);
//        return err;
//    }

//    UART1_Printf("================ SDRAM FULL TEST PASS =================\r\n");

//    return 0;
//}
//HAL_Delay(1000);
//uint32_t sdram_err = SDRAM_FullTest();

//if (sdram_err == 0)
//{
//    UART1_Printf("SDRAM OK\r\n");
//}
//else
//{
//    UART1_Printf("SDRAM ERROR = 0x%08lX\r\n", sdram_err);
//}

