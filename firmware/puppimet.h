#ifndef PUPPIMET_H
#define PUPPIMET_H

#include "data.h"
#include <hls_stream.h>

#define NPOINT 16
#define N_INPUT_LINKS 36    // 36개 병렬 입력
#define N_FRAMES 54         // 54 프레임 누적
#define MAX_PARTICLES 128   // 최대 128개 제한

// 36개 입력을 더하는 함수 (Unroll 사용)
void Sum_Particles(Particle_xy proj_xy[N_INPUT_LINKS], 
                   bool is_valid[N_INPUT_LINKS],
                   Particle_xy &slice_sum, 
                   ap_uint<6> &valid_cnt);

// Token 구조체 그대로 사용 (Pragma로 패킹)
void puppimet_xy(hls::stream<Particle_T> in_particles[N_INPUT_LINKS], 
                 Particle_xy &met_xy, 
                 METCtrlToken token_d, 
                 METCtrlToken& token_q);

void pxpy_to_ptphi(const Particle_xy met_xy, Sum &out_met, METCtrlToken token_d, METCtrlToken& token_q);

#endif