classdef Lockin_Amplifier
    properties (Constant)
        PI = 3.14159265358979;
        DLIA_BLOCK_SIZE = 512;         % 每次处理的样本数
        LUT_SIZE = 1024;               % LUT表大小
        LUT_MASK = 1023;               % LUT掩码 (LUT_SIZE - 1)
        LUT_SHIFT = 22;                % LUT索引右移位数 (32 - log2(LUT_SIZE))
    end
    
    properties (Access = private)
        SinLUT;                        % 正弦查找表
    end
    
    methods
        function obj = Lockin_Amplifier()
            % 构造函数，初始化LUT
            obj = obj.DLIA_Init_LUT();
        end
        
        function obj = DLIA_Init_LUT(obj)
            % 初始化正弦查找表
            obj.SinLUT = zeros(1, obj.LUT_SIZE);
            
            for i = 0:obj.LUT_SIZE-1
                angle = (2.0 * obj.PI * i) / obj.LUT_SIZE;
                obj.SinLUT(i+1) = sin(angle);  % MATLAB索引从1开始
            end
        end
        
        function [config, state] = DLIA_Init(obj, freq, fs, lpf_beta, dc_offset)
            % 初始化配置和状态
            config.Signal_Freq = freq;
            config.Ref_Freq = fs;
            config.LPF_Beta = lpf_beta;
            config.DC_Offset = dc_offset; % 这个值现在仅作为初始观察，算法内部会自动计算真实DC
            
            state.PhaseStep = uint32(freq / fs * 4294967296.0);
            state.PhaseAccumulator = uint32(0);
            state.LPF_Val_I = 0.0;
            state.LPF_Val_Q = 0.0;
        end
        
        function [state, output] = DLIA_Process(obj, config, state, ADCValue)
            % 处理ADC数据
            len = numel(ADCValue);
            
            if len ~= obj.DLIA_BLOCK_SIZE
                error('输入数据长度必须等于DLIA_BLOCK_SIZE');
            end
            
            % 加载状态
            phase_acc = state.PhaseAccumulator;
            phase_step = state.PhaseStep;
            
            % 用于 "Sum(Raw * Ref)"
            sum_I_raw = 0.0;
            sum_Q_raw = 0.0;
            
            % 用于 "协方差修正" 的辅助统计变量
            sum_adc_raw = 0.0; % Sum(Raw)
            sum_sin_ref = 0.0; % Sum(Sin)
            sum_cos_ref = 0.0; % Sum(Cos)
            
            k_convert = 3.3 * 0.000244140625;  % 3.3V / 4096
            
            % 预计算常量
            cos_offset = bitshift(obj.LUT_SIZE, -2);  % 256
            INV_256 = 1.0 / 256.0;
            
            for i = 1:len
                phase_fixed = bitshift(phase_acc, -(obj.LUT_SHIFT - 8)); % 保留8位小数
                
                % 获取0基准的索引
                idx0_base = bitand(bitshift(phase_fixed, -8), obj.LUT_MASK); 
                idx1_base = bitand(idx0_base + 1, obj.LUT_MASK);
                
                % MATLAB索引+1
                idx0 = idx0_base + 1;
                idx1 = idx1_base + 1;
                
                frac = double(bitand(phase_fixed, 255)) * INV_256;
                frac_inv = 1.0 - frac;
                
                % Sin 插值
                val_sin0 = obj.SinLUT(idx0);
                val_sin1 = obj.SinLUT(idx1);
                ref_sin = val_sin0 * frac_inv + val_sin1 * frac;
                
                % Cos 插值
                idx_cos0 = bitand(idx0_base + cos_offset, obj.LUT_MASK) + 1;
                idx_cos1 = bitand(idx1_base + cos_offset, obj.LUT_MASK) + 1;
                
                val_cos0 = obj.SinLUT(idx_cos0);
                val_cos1 = obj.SinLUT(idx_cos1);
                ref_cos = val_cos0 * frac_inv + val_cos1 * frac;
                
                phase_acc = phase_acc + phase_step; % Matlab uint32会自动处理溢出吗？
                 phase_acc_dbl = double(phase_acc) + double(phase_step);
                 phase_acc = uint32(mod(phase_acc_dbl, 4294967296));
                
                val_adc_float = double(ADCValue(i)); % 0~4095
                
                sum_I_raw = sum_I_raw + (val_adc_float * ref_sin);
                sum_Q_raw = sum_Q_raw + (val_adc_float * ref_cos);
                
                sum_adc_raw = sum_adc_raw + val_adc_float;
                sum_sin_ref = sum_sin_ref + ref_sin;
                sum_cos_ref = sum_cos_ref + ref_cos;
            end
            
            state.PhaseAccumulator = phase_acc;

            avg_adc_raw_dc = sum_adc_raw / len;
            
            final_sum_I = sum_I_raw - (avg_adc_raw_dc * sum_sin_ref);
            final_sum_Q = sum_Q_raw - (avg_adc_raw_dc * sum_cos_ref);
            
            scale_factor = (2.0 / obj.DLIA_BLOCK_SIZE) * k_convert;
            
            avg_I = final_sum_I * scale_factor;
            avg_Q = final_sum_Q * scale_factor;
            
            state.LPF_Val_I = state.LPF_Val_I + config.LPF_Beta * (avg_I - state.LPF_Val_I);
            state.LPF_Val_Q = state.LPF_Val_Q + config.LPF_Beta * (avg_Q - state.LPF_Val_Q);
            
            output.Output_I = state.LPF_Val_I;
            output.Output_Q = state.LPF_Val_Q;
            
            sum_sq = output.Output_I^2 + output.Output_Q^2;
            output.Output_Amp = sqrt(sum_sq);
            

            phase = atan2(output.Output_Q, output.Output_I);
            output.Output_PhaseRad = phase;
            output.Output_PhaseDeg = phase * 57.2957795;
        end
    end
end