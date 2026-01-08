#include "puppimet.h"
#include <hls_math.h>
#include <hls_stream.h>

poly2_t cos2_par0[NPOINT] = {-1.00007,-0.924181,-0.707596,-0.382902,-0.000618262,0.382137,0.707056,0.923708,1.00007,0.924181,0.707594,0.383285,0.000188727,-0.382139,-0.706719,-0.923708};
poly2_t cos2_par1[NPOINT] = {9.164680268990924e-06, 0.0017064607695524156, 0.0031441321076514446, 0.004079929656016374, 0.004437063290882583, 0.004095969231842202, 0.0031107221424451436, 0.001689531075808071, -9.161756842493832e-06, -0.001706456406229286, -0.003143961938049376, -0.004103015998697129, -0.004411145151490469, -0.0040958165155326525, -0.0031310072316764474, -0.001689531075808071};
poly2_t cos2_par2[NPOINT] = {9.319674765430664e-06, 7.871694899063284e-06, 5.222989318251642e-06, 2.0256106486379287e-06, -1.9299417402361656e-06, -5.35167113952279e-06, -7.740062096537953e-06, -9.348822844786505e-06, -9.319674765430664e-06, -7.871694899063284e-06, -5.225331064666252e-06, -1.780776301343235e-06, 1.6556927733433181e-06, 5.3495197789955455e-06, 7.954684107366423e-06, 9.348822844786505e-06};

poly2_t sin2_par0[NPOINT] = {0.000524872,-0.382229,-0.706791,-0.923959,-1.00008,-0.924156,-0.707264,-0.383199,-0.000525527,0.382228,0.706792,0.923752,1.00013,0.924155,0.707535,0.3832};
poly2_t sin2_par1[NPOINT] = {-0.004431478237276202, -0.00409041472149773, -0.0031267268116859314, -0.00167440343451641, 9.741773386162849e-06, 0.0017049641497188307, 0.00312406082125351, 0.0040978672774037465, 0.004431478237276202, 0.00409041472149773, 0.0031266351819002015, 0.0016868781753450394, -1.249302315254411e-05, -0.001704846339994321, -0.003140405829698437, -0.0040978672774037465};
poly2_t sin2_par2[NPOINT] = {1.870674613498914e-06, 5.292404012785538e-06, 7.909829192302831e-06, 9.188746390688592e-06, 9.313525301268721e-06, 7.887020962996302e-06, 5.435897856093815e-06, 1.8358587462761668e-06, -1.870668901922293e-06, -5.292404012785538e-06, -7.908420336736317e-06, -9.320836119343602e-06, -9.284396260501616e-06, -7.88869635880513e-06, -5.262894200243701e-06, -1.835864457852788e-06};

phi_t phi2_edges[NPOINT+1] = {-720, -630, -540, -450, -360, -270, -180, -90, 0, 90, 180, 270, 360, 450, 540, 630, 720};



void Get_xy(Particle_T in_particles, Particle_xy &proj_xy) {
    // This function calculates x, y projection values using 2nd Polynomial interpolation
    #pragma HLS inline

    
    int phibin = 0;
    if      (in_particles.hwPhi < phi2_edges[1]) phibin = 0;
    else if (in_particles.hwPhi < phi2_edges[2]) phibin = 1;
    else if (in_particles.hwPhi < phi2_edges[3]) phibin = 2;
    else if (in_particles.hwPhi < phi2_edges[4]) phibin = 3;
    else if (in_particles.hwPhi < phi2_edges[5]) phibin = 4;
    else if (in_particles.hwPhi < phi2_edges[6]) phibin = 5;
    else if (in_particles.hwPhi < phi2_edges[7]) phibin = 6;
    else if (in_particles.hwPhi < phi2_edges[8]) phibin = 7;
    else if (in_particles.hwPhi < phi2_edges[9]) phibin = 8;
    else if (in_particles.hwPhi < phi2_edges[10]) phibin = 9;
    else if (in_particles.hwPhi < phi2_edges[11]) phibin = 10;
    else if (in_particles.hwPhi < phi2_edges[12]) phibin = 11;
    else if (in_particles.hwPhi < phi2_edges[13]) phibin = 12;
    else if (in_particles.hwPhi < phi2_edges[14]) phibin = 13;
    else if (in_particles.hwPhi < phi2_edges[15]) phibin = 14;
    else if (in_particles.hwPhi < phi2_edges[16]) phibin = 15;
    
    poly_t cos_var = cos2_par0[phibin] + cos2_par1[phibin] * (in_particles.hwPhi - phi2_edges[phibin]) + cos2_par2[phibin] * (in_particles.hwPhi - phi2_edges[phibin]) * (in_particles.hwPhi - phi2_edges[phibin]);
    poly_t sin_var = sin2_par0[phibin] + sin2_par1[phibin] * (in_particles.hwPhi - phi2_edges[phibin]) + sin2_par2[phibin] * (in_particles.hwPhi - phi2_edges[phibin]) * (in_particles.hwPhi - phi2_edges[phibin]);
    proj_xy.hwPx = in_particles.hwPt * cos_var;
    proj_xy.hwPy = in_particles.hwPt * sin_var;
}

void Sum_Particles(Particle_xy proj_xy[N_INPUT_LINKS], Particle_xy &met_xy) {
    // Sum of maximum 36 vectors in parallel
  
    #pragma HLS inline
  
    proj_t proj_x[N_INPUT_LINKS];
    proj_t proj_y[N_INPUT_LINKS];
    proj_t met_x = 0;
    proj_t met_y = 0;
  
    met_xy.hwPx = 0;
    met_xy.hwPy = 0;
  
    #pragma HLS ARRAY_PARTITION variable=proj_xy complete
    #pragma HLS ARRAY_PARTITION variable=proj_x complete
    #pragma HLS ARRAY_PARTITION variable=proj_y complete
  
    for(int i=0; i<N_INPUT_LINKS; ++i) {
      #pragma HLS unroll
      proj_x[i] = proj_xy[i].hwPx;
      proj_y[i] = proj_xy[i].hwPy;
    }
  
    CALC_LOOP:
    for(int i=0; i<N_INPUT_LINKS; ++i) {
      #pragma HLS unroll
      // Note: If Proj value has more float bits than metx&y, it makes lots of latency
      met_xy.hwPx -= proj_x[i];
      met_xy.hwPy -= proj_y[i];
    }
  }
  

void puppimet_xy(Particle_T in_particles[N_INPUT_LINKS], Particle_xy  &met_xy, METCtrlToken token_d, METCtrlToken &token_q, METCtrlToken token_i) {
    
    #pragma HLS PIPELINE

    Particle_xy converted_xy[N_INPUT_LINKS];

    #pragma HLS ARRAY_PARTITION variable=in_particles complete
    #pragma HLS ARRAY_PARTITION variable=converted_xy complete

    #pragma HLS aggregate variable=token_d compact=bit
    #pragma HLS aggregate variable=token_q compact=bit
    #pragma HLS aggregate variable=in_particles compact=bit
    #pragma HLS aggregate variable=met_xy compact=bit
    
    #pragma HLS INTERFACE ap_none port=in_particles
    #pragma HLS INTERFACE ap_none port=met_xy
    #pragma HLS INTERFACE ap_none port=token_d
    #pragma HLS INTERFACE ap_none port=token_q
    #pragma HLS INTERFACE ap_none port=token_i

    static proj_t    global_acc_px     = 0;
    static proj_t    global_acc_py     = 0;
    static ap_uint<6> frame_cnt        = 0;

    READ_CONVERT_LOOP:
    for (int i = 0; i < N_INPUT_LINKS; ++i) {
        #pragma HLS UNROLL
        Get_xy(in_particles[i], converted_xy[i]);
    }

    Particle_xy slice_sum;

    Sum_Particles(converted_xy, slice_sum);
    global_acc_px += slice_sum.hwPx;
    global_acc_py += slice_sum.hwPy;
    frame_cnt++;
    met_xy.hwPx = global_acc_px;
    met_xy.hwPy = global_acc_py;

    if (frame_cnt == N_FRAMES) {
        // Reset
        global_acc_px      = 0;
        global_acc_py      = 0;
        frame_cnt          = 0;

        token_i.start_of_orbit = token_d.start_of_orbit;
        token_i.dataValid = true;
        token_i.frameValid = true;
    } else {
        token_i.dataValid = false;
        token_i.frameValid = false;
    }

    token_q = token_i;
}

static const ap_int<12> VAL_90DEG = 360;
static const ap_int<12> VAL_180DEG = 720;
static const int LUT_SIZE = 1024; // Python 코드의 ADDR_BITS=10 일치

void pxpy_to_ptphi(
    const Particle_xy met_xy,
    Sum &out_met,
    METCtrlToken token_d,
    METCtrlToken& token_q
) {
    #pragma HLS PIPELINE II=1
    
    // Interface & Aggregation pragmas
    #pragma HLS aggregate variable=met_xy compact=bit
    #pragma HLS aggregate variable=out_met compact=bit
    #pragma HLS aggregate variable=token_d compact=bit
    #pragma HLS aggregate variable=token_q compact=bit

    #pragma HLS interface ap_none port=met_xy
    #pragma HLS interface ap_none port=out_met
    #pragma HLS interface ap_none port=token_d
    #pragma HLS interface ap_none port=token_q

    // LUT를 BRAM이나 ROM으로 지정 (Resource 관리)
    #pragma HLS BIND_STORAGE variable=LUT_ATAN2_SCALED type=rom_1p impl=auto
    #pragma HLS BIND_STORAGE variable=LUT_HYPOT_SCALE type=rom_1p impl=auto

    // ---------------------------------------------------------
    // 1. 절대값 변환 (Unsigned Fixed Point로 변환하여 처리)
    // ---------------------------------------------------------
    // proj_t는 ap_fixed<22, 12>입니다.
    // 절대값을 취하기 위해 부호를 제거합니다.
    proj_t px = met_xy.hwPx;
    proj_t py = met_xy.hwPy;

    // 부호 비트 저장
    bool sign_x = (px < 0);
    bool sign_y = (py < 0);

    // 절대값 취득 (비트 조작 없이 수식적으로 처리, HLS가 최적화함)
    // 계산 정밀도 확보를 위해 ufixed로 캐스팅하여 연산 준비
    ap_ufixed<22, 12> abs_x = (sign_x) ? (ap_ufixed<22, 12>)(-px) : (ap_ufixed<22, 12>)px;
    ap_ufixed<22, 12> abs_y = (sign_y) ? (ap_ufixed<22, 12>)(-py) : (ap_ufixed<22, 12>)py;

    // ---------------------------------------------------------
    // 2. 8분면 매핑 (Octant Mapping) -> 0~45도 영역으로 변환
    // ---------------------------------------------------------
    ap_ufixed<22, 12> max_val, min_val;
    bool xy_swapped = false;

    if (abs_y > abs_x) {
        max_val = abs_y;
        min_val = abs_x;
        xy_swapped = true; // 45~90도 영역임
    } else {
        max_val = abs_x;
        min_val = abs_y;
        xy_swapped = false; // 0~45도 영역임
    }

    // ---------------------------------------------------------
    // 3. LUT 인덱스 계산 (나눗셈 1회 수행)
    // ---------------------------------------------------------
    // Index = (min / max) * 1024
    // 정밀도를 위해 min을 먼저 Shift하고 나눕니다.
    // LUT_SIZE가 1024(10비트)이므로 << 10
    
    ap_uint<10> lut_idx = 0;
    
    // max_val이 0이면(입력이 0,0) 인덱스 0, 결과도 0
    if (max_val > 0) {
        // ap_ufixed 나눗셈. 분자가 더 커야 소수점 아래 정보가 유지됨.
        // 분자: 22bit + 10bit shift = 32bit.
        // 결과가 10bit 정수 범위에 들어오도록 설정.
        // HLS 나눗셈기 최적화를 위해 명시적 캐스팅 권장
        ap_ufixed<32, 22> num = min_val; 
        num = num << 10; // * 1024
        
        ap_ufixed<10, 10> ratio = num / max_val;
        
        // 인덱스 클램핑 (혹시 모를 오버플로우 방지)
        if (ratio >= LUT_SIZE) lut_idx = LUT_SIZE - 1;
        else lut_idx = ratio;
    }

    // ---------------------------------------------------------
    // 4. Look-Up Table 조회
    // ---------------------------------------------------------
    // LUT 데이터 타입은 생성한 헤더파일과 일치해야 합니다.
    // 예: LUT_HYPOT_SCALE은 ap_ufixed<18, 2> 가정
    // 예: LUT_ATAN2_SCALED는 ap_uint<10> 가정
    
    auto scale_factor = LUT_HYPOT_SCALE[lut_idx];
    auto phi_base     = LUT_ATAN2_SCALED[lut_idx];

    // ---------------------------------------------------------
    // 5. Pt 계산 (Hypotenuse)
    // ---------------------------------------------------------
    // Pt = Max * sqrt(1 + (min/max)^2)
    // 결과 타입 pt_t (14, 12)에 맞춰 포화(Saturation) 처리됨
    // 곱셈 결과가 pt_t보다 클 수 있으므로 중간 변수 사용
    ap_ufixed<30, 14> pt_calc = max_val * scale_factor;
    out_met.hwPt = (pt_t)pt_calc;
    out_met.hwSumPt = 0; // SumPt는 여기서 계산 안하면 0 또는 패스스루

    // ---------------------------------------------------------
    // 6. Phi 계산 (복원 과정)
    // ---------------------------------------------------------
    ap_int<12> phi_temp; // 계산용 넉넉한 비트 (최대 720)

    // A. Swap 복원 (0~90도 생성)
    if (xy_swapped) {
        // 45~90도 영역이었으므로, 90도에서 뺌
        phi_temp = VAL_90DEG - phi_base;
    } else {
        phi_temp = phi_base;
    }

    // B. X축 대칭 복원 (0~180도 생성)
    if (sign_x) {
        // 2, 3사분면 (X < 0) -> 180도에서 뺌
        phi_temp = VAL_180DEG - phi_temp;
    }

    // C. Y축 대칭 복원 (-180 ~ +180도 생성)
    // Y가 음수면 전체 각도에 마이너스 (3, 4사분면)
    if (sign_y) {
        phi_temp = -phi_temp;
    }

    // 최종 할당 (phi_t는 ap_int<11>이므로 캐스팅)
    out_met.hwPhi = (phi_t)phi_temp;

    // ---------------------------------------------------------
    // Token 전달
    // ---------------------------------------------------------
    out_met.hwSumPt = 0; // 필요시 연결
    token_q = token_d;
}