clc; clear; close all;

% === 1. 核心参数修正 ===
Freq_Sig = 100000;      % 信号频率
Freq_Samp = 1000000;   % 采样率
Beta = 0.005;         % 极慢的滤波器系数
Block_Size = 512;     % 数据块大小

% 计算数据块更新率 (Hz)
Block_Rate = Freq_Samp / Block_Size; 
% 计算 IIR 时间常数 (秒)
Time_Constant = 1 / (Block_Rate * Beta); 

% === 根据时间常数自动设定仿真时长 ===
% 为了看清稳态，仿真时间至少要是时间常数的 4 倍
Sim_Duration = Time_Constant * 5; 
fprintf('IIR 时间常数: %.2f 秒\n', Time_Constant);
fprintf('自动设定仿真时长: %.2f 秒\n', Sim_Duration);

% 生成时间轴
dt = 1/Freq_Samp;
t = 0:dt:Sim_Duration-dt;
N_Samples = length(t);

% === 模拟恶劣环境 ===
% 1.0V 信号 + 0.5V 宽带噪声 + 1.65V 直流 + 100Hz 干扰
noise = 0.5 * randn(1, N_Samples);
dc_drift = 1.65;
interference = 0.2 * sin(2*pi*100*t); % 加一个干扰看看能不能滤掉

Input_Analog = 1.0 * sin(2*pi*Freq_Sig*t + pi/4) ... % 目标信号 (45度)
             + noise ...                             % 随机噪声
             + dc_drift ...                          % 直流偏置
             + interference;                         % 干扰

% === 3. 算法变量 ===
CurrentPhase = 0;
PhaseStep = 2*pi*Freq_Sig/Freq_Samp;
LPF_I = 0; 
LPF_Q = 0; % 初始值从0开始爬升

DC_Est = 1.65; % 假设DC初始猜对了
DC_Alpha = 0.0002;

% 记录结果
Rec_Amp = [];
Rec_Phase = [];
Rec_Time = [];

% === 4. 模拟循环 ===
Sum_I = 0; Sum_Q = 0;
cnt = 0;

for n = 1:N_Samples
    % --- A. DC 阻断 ---
    raw = Input_Analog(n);
    DC_Est = DC_Est + DC_Alpha * (raw - DC_Est);
    Input_AC = raw - DC_Est;
    
    % --- B. DDS 参考 ---
    ref_sin = sin(CurrentPhase);
    ref_cos = cos(CurrentPhase);
    CurrentPhase = CurrentPhase + PhaseStep;
    if CurrentPhase >= 2*pi, CurrentPhase = CurrentPhase - 2*pi; end
    
    % --- C. 混频 ---
    Sum_I = Sum_I + Input_AC * ref_sin;
    Sum_Q = Sum_Q + Input_AC * ref_cos;
    cnt = cnt + 1;
    
    % --- D. 块处理 (每512点一次) ---
    if cnt == Block_Size
        avg_I = (Sum_I / Block_Size) * 2;
        avg_Q = (Sum_Q / Block_Size) * 2;
        
        % IIR 核心公式
        LPF_I = LPF_I + Beta * (avg_I - LPF_I);
        LPF_Q = LPF_Q + Beta * (avg_Q - LPF_Q);
        
        % 计算
        Amp = sqrt(LPF_I^2 + LPF_Q^2);
        Phase = atan2(LPF_Q, LPF_I) * 180/pi;
        
        % 记录
        Rec_Amp = [Rec_Amp, Amp];
        Rec_Phase = [Rec_Phase, Phase];
        Rec_Time = [Rec_Time, n*dt];
        
        cnt = 0; Sum_I = 0; Sum_Q = 0;
    end
end

% === 收敛时间分析 ===
% 计算幅值收敛时间
target_amp = 1.0; % 目标幅值
tolerance_levels = [0.90, 0.95, 0.99]; % 收敛百分比：90%, 95%, 99%
amp_convergence_times = zeros(1, length(tolerance_levels));

for i = 1:length(tolerance_levels)
    tolerance = tolerance_levels(i);
    target_value = target_amp * tolerance;

    % 找到第一个达到或超过目标值的点
    idx = find(Rec_Amp >= target_value, 1);
    if ~isempty(idx)
        amp_convergence_times(i) = Rec_Time(idx);
    else
        amp_convergence_times(i) = NaN;
    end
end

% 计算相位收敛时间
target_phase = 45.0; % 目标相位
phase_tolerance = 1.0; % 相位容差（度）
phase_convergence_time = NaN;

% 找到相位稳定在目标值±容差范围内的点
phase_within_tolerance = abs(Rec_Phase - target_phase) <= phase_tolerance;
if any(phase_within_tolerance)
    % 找到第一个进入容差范围并保持的点
    for j = 1:length(Rec_Phase)
        if all(abs(Rec_Phase(j:end) - target_phase) <= phase_tolerance)
            phase_convergence_time = Rec_Time(j);
            break;
        end
    end
end

% 输出收敛时间结果
fprintf('\n=== 收敛时间分析结果 ===\n');
fprintf('仿真参数:\n');
fprintf('  信号频率: %.0f Hz\n', Freq_Sig);
fprintf('  采样率: %.0f Hz\n', Freq_Samp);
fprintf('  滤波器系数β: %.4f\n', Beta);
fprintf('  数据块大小: %d 样本\n', Block_Size);
fprintf('  块更新率: %.1f Hz\n', Block_Rate);
fprintf('  理论IIR时间常数: %.4f 秒\n', Time_Constant);
fprintf('  仿真时长: %.3f 秒 (%.1f倍时间常数)\n', Sim_Duration, Sim_Duration/Time_Constant);

fprintf('\n幅值收敛时间:\n');
for i = 1:length(tolerance_levels)
    if ~isnan(amp_convergence_times(i))
        fprintf('  达到%.0f%%稳态值: %.4f 秒 (%.1f倍时间常数)\n', ...
            tolerance_levels(i)*100, amp_convergence_times(i), amp_convergence_times(i)/Time_Constant);
    else
        fprintf('  达到%.0f%%稳态值: 未收敛 (仿真时间可能不足)\n', tolerance_levels(i)*100);
    end
end

if ~isnan(phase_convergence_time)
    fprintf('\n相位收敛时间:\n');
    fprintf('  稳定在%.1f°±%.1f°: %.4f 秒 (%.1f倍时间常数)\n', ...
        target_phase, phase_tolerance, phase_convergence_time, phase_convergence_time/Time_Constant);
else
    fprintf('\n相位收敛时间: 未收敛到目标范围内\n');
end

% 计算最终稳态误差
final_amp_error = abs(Rec_Amp(end) - target_amp) / target_amp * 100;
final_phase_error = abs(Rec_Phase(end) - target_phase);
fprintf('\n最终稳态性能:\n');
fprintf('  最终幅值: %.4f V (目标: %.4f V)\n', Rec_Amp(end), target_amp);
fprintf('  幅值误差: %.4f V (%.2f%%)\n', Rec_Amp(end) - target_amp, final_amp_error);
fprintf('  最终相位: %.2f° (目标: %.2f°)\n', Rec_Phase(end), target_phase);
fprintf('  相位误差: %.2f°\n', final_phase_error);

% 收敛速度分析
if length(Rec_Amp) > 1
    % 计算收敛速度（达到90%稳态值所需的时间）
    if ~isnan(amp_convergence_times(1))
        convergence_speed = 1 / amp_convergence_times(1); % 1/秒
        fprintf('  收敛速度: %.3f 1/秒\n', convergence_speed);
    end
end

% === 绘图验证 ===
figure('Color', 'w');

% 幅值
subplot(2,1,1);
plot(Rec_Time, Rec_Amp, 'LineWidth', 1.5);
hold on;
yline(1.0, 'r--', 'LineWidth', 1.5);
% 添加收敛时间标记线
for i = 1:length(amp_convergence_times)
    if ~isnan(amp_convergence_times(i))
        xline(amp_convergence_times(i), 'g--', ...
            sprintf('%.0f%%: %.3fs', tolerance_levels(i)*100, amp_convergence_times(i)), ...
            'LineWidth', 1, 'LabelVerticalAlignment', 'bottom');
    end
end
hold off;
title(sprintf('幅值收敛 (目标 1.0V) - 最终: %.4f V', Rec_Amp(end)));
xlabel('Time (s)'); ylabel('Voltage (V)');
grid on; ylim([0, 1.2]);

% 相位
subplot(2,1,2);
plot(Rec_Time, Rec_Phase, 'LineWidth', 1.5);
hold on;
yline(45, 'r--', 'LineWidth', 1.5);
yline(45 + phase_tolerance, 'g:', 'LineWidth', 1);
yline(45 - phase_tolerance, 'g:', 'LineWidth', 1);
if ~isnan(phase_convergence_time)
    xline(phase_convergence_time, 'm--', ...
        sprintf('稳定: %.3fs', phase_convergence_time), ...
        'LineWidth', 1, 'LabelVerticalAlignment', 'bottom');
end
hold off;
title(sprintf('相位收敛 (目标 45.0°) - 最终: %.2f°', Rec_Phase(end)));
xlabel('Time (s)'); ylabel('Degree (°)');
grid on; ylim([40, 50]);