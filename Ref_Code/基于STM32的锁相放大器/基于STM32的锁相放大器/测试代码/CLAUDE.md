# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an **STM32F407 microcontroller project** implementing a **dual-channel ADC signal processing system** with real-time FFT analysis. The system captures synchronized samples from two ADC channels, performs 1024-point FFT on both signals, and calculates frequency, amplitude, and phase difference between them. Results are displayed on an OLED screen.

**Key Specifications:**
- Target: STM32F407VETx (Cortex-M4 with FPU)
- Sampling: 80kHz (Fs = 80000)
- Buffer: 1024 samples per channel
- ADC Mode: Dual simultaneous (ADC_DUALMODE_REGSIMULT)
- DSP: ARM CMSIS-DSP library for FFT
- Display: OLED screen via custom driver

## Development Environment

### Primary IDE: Keil MDK-ARM (µVision)
- Project file: `MDK-ARM/STM32F407_Hal_5_DualADC.uvprojx`
- Toolchain: ARM-ADS (ARM Compiler 5)
- Device Pack: Keil.STM32F4xx_DFP.2.13.0

**Keil Commands:**
- Build: F7 or Project → Build Target
- Flash: F8 or Flash → Download
- Debug: Ctrl+F5 or Debug → Start/Stop Debug Session
- Open project: Double-click `.uvprojx` file

### Secondary IDE: Visual Studio Code
- Configuration: `MDK-ARM/.vscode/`
- IntelliSense: Configured via `c_cpp_properties.json`
- Includes all necessary paths and defines for STM32 development

### Configuration Tool: STM32CubeMX
- Project file: `STM32F407_Hal_5_DualADC.ioc`
- Regenerate code after peripheral configuration changes

## Build Configuration

**Key Preprocessor Defines:**
- `USE_HAL_DRIVER`: Enable STM32 HAL library
- `STM32F407xx`: Target device family
- `ARM_MATH_CM4`: ARM CMSIS-DSP for Cortex-M4
- `__CC_ARM`: ARM Compiler specific

**Memory Configuration:**
- IRAM: 0x20000000-0x2001BFFF (112KB)
- IRAM2: 0x2001C000-0x2001FFFF (16KB)
- IROM: 0x8000000-0x807FFFF (512KB)
- Clock: 25MHz external → 168MHz system clock via PLL

## Architecture

### Signal Processing Flow
1. **ADC Capture**: Timer-triggered dual ADC simultaneous sampling
2. **DMA Transfer**: 1024 samples per channel to memory buffers
3. **Preprocessing**: DC offset removal, Hanning window application
4. **FFT Analysis**: 1024-point real FFT using `arm_rfft_fast_f32`
5. **Feature Extraction**: Frequency, amplitude, phase calculation
6. **Display**: OLED output of measurements

### Key Components

**ADC/DMA System (`Core/Src/adc.c`, `Core/Src/dma.c`):**
- Dual ADC mode: ADC1 (master) + ADC2 (slave)
- DMA in circular mode for continuous sampling
- Timer TIM2 triggers conversions at 80kHz

**Signal Processing (`MDK-ARM/Systrem/fft.c/.h`):**
- Uses ARM CMSIS-DSP library (`arm_math.h`)
- Real FFT with `arm_rfft_fast_f32`
- Functions: `FFT1()`, `FFT2()`, `Get_Freq1()`, `Get_Freq2()`, `Get_PhaseDiff()`, `Get_Amplitude1()`, `Get_Amplitude2()`

**Display System (`MDK-ARM/Hardware/OLED.c/.h`):**
- Custom OLED driver for visualization
- Displays frequency, amplitude, and phase measurements

**Main Application (`Core/Src/main.c`):**
- Initializes all peripherals
- Starts ADC+DMA acquisition
- Processes data when buffer is full
- Updates OLED display

### Data Buffers
- `AdcValue[1024]`: Raw interleaved ADC data (ADC1, ADC2 alternating)
- `Adc1Value[1024]`, `Adc2Value[1024]`: Separated channel data
- `input1[N]`, `input2[N]`: Preprocessed float buffers for FFT
- `output1[N]`, `output2[N]`: FFT complex output buffers
- `amplitude1[N/2]`, `amplitude2[N/2]`: Magnitude spectra
- `phase1[N/2]`, `phase2[N/2]`: Phase spectra

## Development Workflow

### 1. Code Editing
- Use VSCode for editing with IntelliSense support
- Keil project file maintains build configuration
- Header files in `Core/Inc/`, source in `Core/Src/`
- Custom code in `MDK-ARM/Systrem/` and `MDK-ARM/Hardware/`

### 2. Building and Flashing
1. Open `.uvprojx` in Keil MDK-ARM
2. Build project (F7)
3. Connect ST-Link/V2 debugger
4. Flash to device (F8)
5. Start debug session if needed (Ctrl+F5)

### 3. Configuration Changes
1. Open `.ioc` in STM32CubeMX
2. Modify peripheral configurations
3. Generate code (overwrites `Core/Src/` and `Core/Inc/` files)
4. Rebuild in Keil

### 4. Debugging
- Use Keil debugger with SVD file: `CMSIS\SVD\STM32F40x.svd`
- Monitor variables: `Adc1Value`, `Adc2Value`, FFT results
- Check `data_ready` flag for buffer completion

## Key Files and Locations

**Project Configuration:**
- `MDK-ARM/STM32F407_Hal_5_DualADC.uvprojx` - Keil project
- `STM32F407_Hal_5_DualADC.ioc` - CubeMX configuration
- `MDK-ARM/.vscode/c_cpp_properties.json` - VSCode IntelliSense

**Application Code:**
- `Core/Src/main.c` - Main application loop
- `Core/Src/adc.c` - ADC configuration and DMA setup
- `Core/Src/tim.c` - Timer configuration for ADC triggering

**Signal Processing:**
- `MDK-ARM/Systrem/fft.c/.h` - FFT implementation and analysis
- `MDK-ARM/Systrem/Delay.c/.h` - Delay utilities

**Hardware Drivers:**
- `MDK-ARM/Hardware/OLED.c/.h` - OLED display driver
- `Drivers/` - STM32 HAL and CMSIS libraries
- `Middlewares/ST/ARM/DSP/` - ARM CMSIS-DSP library

## Important Notes

1. **Dual ADC Mode**: Uses ADC_DUALMODE_REGSIMULT for synchronized sampling
2. **FFT Parameters**: 1024-point FFT with Hanning window, 80kHz sampling rate
3. **Memory Alignment**: FFT buffers use `__attribute__((aligned(4)))` for ARM CMSIS-DSP requirements
4. **Phase Calculation**: Returns phase difference in degrees between -180° and 180°
5. **Amplitude Scaling**: Normalized by N/4 to account for windowing and FFT scaling

## Common Development Tasks

**Adding New Signal Processing:**
1. Add functions to `fft.c/.h`
2. Call from main loop after `FFT1()` and `FFT2()`
3. Update OLED display if needed

**Modifying ADC Configuration:**
1. Edit `.ioc` in STM32CubeMX
2. Regenerate code
3. Update `adc.c` if manual configuration needed

**Changing Sampling Parameters:**
- Update `Fs` in `fft.c` (line 10)
- Adjust timer period in `tim.c` (currently 41,24 for 80kHz)
- Ensure buffer sizes match (N = 1024)

**Adding Peripherals:**
1. Configure in STM32CubeMX `.ioc` file
2. Regenerate code
3. Add initialization calls in `main.c`
4. Implement driver code in appropriate location