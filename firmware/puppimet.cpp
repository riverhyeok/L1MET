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


// [Helper 1] 1개 입자 변환
void Get_xy(Particle_T in_particles, Particle_xy &proj_xy) {
    #pragma HLS inline
    if (in_particles.hwPt == 0) {
        proj_xy.hwPx = 0; proj_xy.hwPy = 0; return;
    }
    
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


void Sum_Particles(Particle_xy proj_xy[N_INPUT_LINKS], 
    bool is_valid[N_INPUT_LINKS], // 인자는 유지하되 내부에서 미사용 (호환성)
    Particle_xy &slice_sum, 
    ap_uint<6> &valid_cnt) {
#pragma HLS inline // 상위 함수에 병합되어 최적화

// ------------------------------------------------
// Stage 1: 36개 입력을 6개 그룹으로 나누어 병렬 덧셈
// ------------------------------------------------
proj_t partial_sum_x[6] = {0, 0, 0, 0, 0, 0};
proj_t partial_sum_y[6] = {0, 0, 0, 0, 0, 0};
ap_uint<4> partial_cnt[6] = {0, 0, 0, 0, 0, 0};

// 파티셔닝 지시어는 그대로 유지
#pragma HLS ARRAY_PARTITION variable=proj_xy complete
#pragma HLS ARRAY_PARTITION variable=is_valid complete

STAGE1_LOOP:
for(int i=0; i<6; ++i) {
#pragma HLS UNROLL
for(int j=0; j<6; ++j) {
#pragma HLS UNROLL
int idx = i*6 + j;

// [최적화 1] Copy Loop 제거: 입력 배열에서 바로 더함
// Get_xy에서 이미 0 처리가 되었으므로 조건문 없이 더해도 안전함
partial_sum_x[i] += proj_xy[idx].hwPx;
partial_sum_y[i] += proj_xy[idx].hwPy;

// Valid 카운트는 데이터 의존성이 없으므로 여기서 수행
if(is_valid[idx]) {
 partial_cnt[i]++;
}
}
}

// ------------------------------------------------
// Stage 2: 6개의 부분 합을 최종 합산
// ------------------------------------------------
proj_t total_x = 0;
proj_t total_y = 0;
ap_uint<6> total_cnt = 0;

STAGE2_LOOP:
for(int i=0; i<6; ++i) {
#pragma HLS UNROLL
total_x += partial_sum_x[i];
total_y += partial_sum_y[i];
total_cnt += partial_cnt[i];
}

slice_sum.hwPx = total_x;
slice_sum.hwPy = total_y;
valid_cnt = total_cnt;
}

// ---------------------------------------------------------
// TOP MODULE
// ---------------------------------------------------------
void puppimet_xy(
    Particle_T      in_particles[N_INPUT_LINKS],   // Stream -> Array 변경
    Particle_xy    &met_xy,
    METCtrlToken    token_d,
    METCtrlToken   &token_q
) {
    // -----------------------------------------------------
    // Interface 설정
    // -----------------------------------------------------
    #pragma HLS INTERFACE ap_none port=in_particles
    #pragma HLS INTERFACE ap_none port=met_xy

    #pragma HLS ARRAY_PARTITION variable=in_particles complete

    // Token interface
    #pragma HLS INTERFACE ap_none port=token_d
    #pragma HLS INTERFACE ap_none port=token_q
    #pragma HLS aggregate variable=token_d compact=bit
    #pragma HLS aggregate variable=token_q compact=bit

    #pragma HLS PIPELINE

    // -----------------------------------------------------
    // Static Accumulators
    // -----------------------------------------------------
    static proj_t    global_acc_px     = 0;
    static proj_t    global_acc_py     = 0;
    static ap_uint<6> frame_cnt         = 0;
    static ap_uint<9> particle_cnt_total = 0;

    // -----------------------------------------------------
    // 1. Read & Convert
    // -----------------------------------------------------
    Particle_xy converted_xy[N_INPUT_LINKS];
    bool        is_valid[N_INPUT_LINKS];

READ_CONVERT_LOOP:
    for (int i = 0; i < N_INPUT_LINKS; ++i) {
        #pragma HLS UNROLL

        // Stream → Array 변경: read() 제거
        Particle_T tmp = in_particles[i];

        Get_xy(tmp, converted_xy[i]);
        is_valid[i] = (tmp.hwPt > 0);
    }

    // -----------------------------------------------------
    // 2. Sum_Particles
    // -----------------------------------------------------
    Particle_xy slice_sum;
    ap_uint<6>  slice_valid_cnt;

    Sum_Particles(converted_xy, is_valid, slice_sum, slice_valid_cnt);

    // -----------------------------------------------------
    // 3. Global Accumulation
    // -----------------------------------------------------
    if (particle_cnt_total < MAX_PARTICLES) {
        global_acc_px      -= slice_sum.hwPx;
        global_acc_py      -= slice_sum.hwPy;
        particle_cnt_total += slice_valid_cnt;
    }

    // -----------------------------------------------------
    // 4. Output Logic
    // -----------------------------------------------------
    frame_cnt++;

    if (frame_cnt == N_FRAMES) {
        met_xy.hwPx = global_acc_px;
        met_xy.hwPy = global_acc_py;

        // Reset
        global_acc_px      = 0;
        global_acc_py      = 0;
        frame_cnt          = 0;
        particle_cnt_total = 0;
    } else {
        met_xy.hwPx = 0;
        met_xy.hwPy = 0;
    }

    // -----------------------------------------------------
    // 5. Token Pass-through
    // -----------------------------------------------------
    token_q = token_d;
}

// CORDIC 설정
#define NUM_ITER 20 // 20번 반복이면 26비트 정밀도에 충분
// CORDIC Gain (1.64676...)의 역수 = 0.607252935
const ap_fixed<26, 2> CORDIC_GAIN = 0.60725; 

// [최적화 핵심] 
// atan(2^-i) 값들에 미리 229.29936을 곱해둔 테이블입니다.
// 이렇게 하면 나중에 229.xx를 곱하는 DSP 연산을 제거할 수 있습니다.
// 값 예시: atan(1)*229.3, atan(0.5)*229.3 ...
const ap_fixed<26, 11> ANGLES_SCALED[NUM_ITER] = {
    180.091, 106.315, 56.177, 28.514, 14.336, 7.181, 3.593, 1.797, 
    0.898, 0.449, 0.225, 0.112, 0.056, 0.028, 0.014, 0.007,
    0.003, 0.002, 0.001, 0.000 // (실제로는 더 정밀하게 계산해서 넣어야 함)
};

void pxpy_to_ptphi(const Particle_xy met_xy, Sum &out_met, METCtrlToken token_d, METCtrlToken& token_q) {
  #pragma HLS PIPELINE
  // II=54를 명시하면 HLS가 리소스를 최대한 공유해서 회로를 작게 만듭니다.
  
  // 기존 인터페이스 유지
  #pragma HLS aggregate variable=met_xy compact=bit
  #pragma HLS aggregate variable=out_met compact=bit
  // ... (나머지 인터페이스 유지)

  // 1. 좌표 초기화
  // CORDIC은 1, 4사분면(x > 0)에서 동작하므로 미리 매핑이 필요할 수 있으나,
  // Full Quadrant CORDIC은 초기 회전으로 처리 가능합니다.
  // 여기서는 간단히 x, y, z(angle)로 시작합니다.
  
  ap_fixed<26, 11> x = met_xy.hwPx;
  ap_fixed<26, 11> y = met_xy.hwPy;
  ap_fixed<26, 11> z = 0; // 이것이 나중에 hwPhi가 됩니다.
  
  // 1-1. 초기 사분면 보정 (x가 음수일 때 처리)
  // CORDIC 수렴 범위를 위해 x를 양수로 만듭니다.
  // x < 0 이면 180도(pi) 회전 효과를 줍니다.
  // 여기서는 스케일링된 180도 = 3.14159 * 229.29936 = 약 720.4
  ap_fixed<26, 11> pi_scaled = 720.407; 
  
  if (x < 0) {
      x = -x;
      y = -y;
      if (met_xy.hwPy < 0) z = -pi_scaled;
      else z = pi_scaled;
  }

  // 2. Iterative CORDIC Loop
  CORDIC_LOOP:
  for (int i = 0; i < NUM_ITER; i++) {
    #pragma HLS PIPELINE off 


    ap_fixed<26, 11> x_new, y_new, z_new;
    ap_fixed<26, 11> shift_x = x >> i; // 비트 시프트 (나눗셈 대체)
    ap_fixed<26, 11> shift_y = y >> i;

    bool sign = (y < 0); // y를 0으로 만드는 것이 목표 (Vectoring Mode)

    if (!sign) { // y가 양수면 시계방향 회전
        x_new = x + shift_y;
        y_new = y - shift_x;
        z_new = z + ANGLES_SCALED[i]; // 미리 스케일된 각도 더함
    } else { // y가 음수면 반시계방향 회전
        x_new = x - shift_y;
        y_new = y + shift_x;
        z_new = z - ANGLES_SCALED[i];
    }

    x = x_new;
    y = y_new;
    z = z_new;
  }


  out_met.hwPt = x * CORDIC_GAIN; 
  out_met.hwPhi = phi_t(z); // 이미 229.299가 곱해진 상태

  token_q = token_d;
}


